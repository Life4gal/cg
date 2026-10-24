#include <cg/flow/duel.hpp>

#include <algorithm>
#include <array>
#include <ranges>

#include <cg/utility/functional.hpp>
#include <cg/execution/chain.hpp>

#include <libassert/assert.hpp>

namespace cg::flow
{
	auto Duel::open_timing(const std::span<const execution::EventWindow> windows) noexcept -> bool
	{
		if (!chain_.empty())
		{
			// 连锁中不能开新时点
			return false;
		}

		// 新时点从回合玩家的优先权开始
		priority_.reset();

		windows_.assign(windows.begin(), windows.end());

		const auto candidates = interpreter_.collect_triggered(windows_, turn_.turn_player());
		if (candidates.empty())
		{
			return false;
		}

		auto added = false;

		// SEGOC 四组:回合必发 → 对方必发 → 回合选发 → 对方选发
		for (std::size_t group = 0; group < 4; ++group)
		{
			const auto optional = group >= 2;
			const auto is_turn_player = (group % 2) == 0;
			const auto player = is_turn_player ? turn_.turn_player() : domain::opponent_of(turn_.turn_player());

			std::vector<std::size_t> group_indices{};
			std::vector<execution::TriggerChoice> choices{};

			for (std::size_t index = 0; index < candidates.size(); ++index)
			{
				const auto& entry = candidates[index];

				if (entry.trigger.get().optional != optional)
				{
					continue;
				}
				if ((entry.controller == turn_.turn_player()) != is_turn_player)
				{
					continue;
				}
				// 使用次数/互斥已满的效果不作为候选
				if (!can_use(entry.effect.get(), entry.instance.get()))
				{
					continue;
				}
				// 召唤宣言窗口的第一环必须是召唤无效效果
				if (summon_.has_value() && !entry.effect.get().properties.contains(dsl::EffectProperty::SUMMON_NEGATE))
				{
					continue;
				}

				group_indices.push_back(index);
				choices.push_back(
					execution::TriggerChoice
					{
							.controller = entry.controller,
							.source = entry.instance.get().id(),
							.effect = entry.effect.get().id,
							.window = entry.window,
							.optional = optional,
					}
				);
			}

			if (choices.empty())
			{
				continue;
			}

			const auto selected = selector_.get().choose_triggers(
				execution::TriggerDecision
				{
						.player = player,
						.optional = optional,
						.candidates = std::span<const execution::TriggerChoice>{choices},
				}
			);

			// 外部选择校验:索引合法、不重复;必发组必须全选
			// 非法选择视为驱动错误,引擎不自动纠正
			std::vector selected_flags(choices.size(), false);
			for (const auto choice: selected)
			{
				if (choice >= choices.size() || selected_flags[choice])
				{
					return false;
				}
				selected_flags[choice] = true;
			}
			if (!optional && selected.size() != choices.size())
			{
				return false;
			}

			for (const auto choice: selected)
			{
				const auto& entry = candidates[group_indices[choice]];
				const auto& instance = entry.instance.get();

				// 前面组可能已占用次数
				if (!can_use(entry.effect.get(), instance))
				{
					continue;
				}
				if (summon_.has_value() && !entry.effect.get().properties.contains(dsl::EffectProperty::SUMMON_NEGATE))
				{
					continue;
				}

				auto link = interpreter_.make_link(entry);
				if (!link.has_value())
				{
					continue;
				}

				const auto mutex_inserted = mark_used(entry.effect.get(), instance);
				link->set_mutex_inserted(mutex_inserted);
				chain_.push_back(std::move(*link));

				added = true;
			}
		}

		if (added)
		{
			// 最后发动者把优先权交给对方
			priority_.transfer_from(chain_.back().controller(), turn_.turn_player());

			// 已经进入连锁,本时点关闭
			windows_.clear();
		}

		return added;
	}

	auto Duel::process_pending_events() noexcept -> void
	{
		// 连锁中产生的事件由结算结束时统一处理
		if (!chain_.empty())
		{
			return;
		}

		const auto events = interpreter_.take_events();
		if (events.empty())
		{
			return;
		}

		// 同一批事件视为同一时点
		execution::EventWindowList windows{};
		windows.reserve(events.size());

		for (const auto& [code, card]: events)
		{
			windows.push_back(
				{
						.source = dsl::TriggerSource{code},
						.card = card,
						.player = std::nullopt,
						.amount = std::nullopt,
				}
			);
		}

		open_timing(windows);
	}

	auto Duel::resolve_chain() noexcept -> void
	{
		if (chain_.empty())
		{
			return;
		}

		// 连锁开始结算后不能再插入发动,时点关闭
		windows_.clear();

		std::ignore = interpreter_.resolve(chain_);

		// 无效回滚:发动无效(USE以外)/ 效果无效(APPLY)
		for (const auto& link: chain_)
		{
			const auto* effect = link.effect();

			if (effect == nullptr)
			{
				continue;
			}

			const auto source = link.source();
			const auto* instance = game_.find_instance(source);
			if (instance == nullptr)
			{
				continue;
			}

			if (link.activation_negated())
			{
				if (effect->usage.has_value() && effect->usage->verb != dsl::RestrictionVerb::USE)
				{
					unmark_used(*effect, *instance, link.mutex_inserted());
				}
			}
			else if (link.effect_negated())
			{
				if (effect->usage.has_value() && effect->usage->verb == dsl::RestrictionVerb::APPLY)
				{
					unmark_used(*effect, *instance, link.mutex_inserted());
				}
			}
		}

		// 整条连锁结算完毕:不能留场的卡送墓(通常魔法/陷阱、被无效发动的卡)
		for (const auto& link: chain_)
		{
			const auto source = link.source();
			auto* instance = game_.find_instance_mutable(source);
			if (instance == nullptr)
			{
				continue;
			}

			if (const auto zone = game_.zone_of(source);
				zone != domain::Zone::SPELL_TRAP && zone != domain::Zone::FIELD_SPELL)
			{
				continue;
			}

			if (link.activation_negated() || !can_stay_on_field(*instance))
			{
				// 发动无效的魔陷送墓;但不视为「从场上送去墓地」
				game_.move_to_graveyard(*instance, {domain::Reason::RULE});
			}
		}

		// 召唤宣言链:仅 Chain Link 1 可以无效召唤;由 flow 消费 SUMMON_NEGATE
		if (summon_.has_value() && !chain_.empty())
		{
			const auto& first = chain_.front();
			const auto* effect = first.effect();
			summon_negated_ = effect != nullptr
			                  && effect->properties.contains(dsl::EffectProperty::SUMMON_NEGATE)
			                  && !first.activation_negated()
			                  && !first.effect_negated();
		}

		chain_.clear();
		priority_.reset();

		// 结算中产生的事件:合并为下一个时点
		process_pending_events();
	}

	auto Duel::contains_duplicated_effect(const state::CardInstanceId source, const dsl::Effect* effect) const noexcept -> bool
	{
		return std::ranges::any_of(
			chain_,
			[&](const execution::ChainLink& link) noexcept -> bool
			{
				return link.source() == source && link.effect() == effect;
			}
		);
	}

	auto Duel::activation_speed(const state::CardInstance& card, const dsl::ActivationKind& activation) const noexcept -> domain::EffectSpeed
	{
		const auto type = [&] noexcept -> domain::CardType
		{
			const auto types = interpreter_.state_resolver().card_types(card);

			// 优先怪兽
			if (types.contains(domain::CardType::MONSTER))
			{
				return domain::CardType::MONSTER;
			}

			if (types.contains(domain::CardType::TRAP))
			{
				return domain::CardType::TRAP;
			}

			return domain::CardType::SPELL;
		}();

		return dsl::speed_of(type, interpreter_.state_resolver().card_subtypes(card), activation);
	}

	auto Duel::can_chain(const domain::EffectSpeed speed) const noexcept -> bool
	{
		if (chain_.empty())
		{
			return true;
		}

		// 响应方须速度 2 以上
		if (!domain::is_fast_effect(speed))
		{
			return false;
		}

		// 且不低于上一环
		if (const auto& last_link = chain_.back();
			std::to_underlying(speed) < std::to_underlying(last_link.speed()))
		{
			return false;
		}

		return true;
	}

	auto Duel::can_stay_on_field(const state::CardInstance& card) const noexcept -> bool
	{
		if (interpreter_.state_resolver().contains_card_type(card, domain::CardType::MONSTER))
		{
			return true;
		}

		return interpreter_.state_resolver().contains_card_subtypes(
			card,
			{
					domain::CardSubtype::CONTINUOUS,
					domain::CardSubtype::FIELD,
					domain::CardSubtype::EQUIP,
			}
		);
	}

	auto Duel::can_activate_in_damage_step(const state::CardInstance& card, const dsl::Effect* effect) const noexcept -> bool
	{
		if (!battle_.is_in_damage_step())
		{
			return true;
		}

		// Counter Trap 在伤害步骤内不受限制
		if (interpreter_.state_resolver().contains_card_type(card, domain::CardType::TRAP) &&
		    interpreter_.state_resolver().contains_card_subtype(card, domain::CardSubtype::COUNTER))
		{
			return true;
		}

		if (effect == nullptr || effect->properties.empty())
		{
			return false;
		}

		const auto& properties = effect->properties;
		const auto damage_step = battle_.damage_step();

		// 无效「发动」的快速效果:任意伤害步骤窗口
		if (properties.contains(dsl::EffectProperty::ACTIVATION_NEGATE))
		{
			return true;
		}

		// 明确「伤害计算时」:仅伤害计算窗口
		if (properties.contains(dsl::EffectProperty::DAMAGE_CALCULATION))
		{
			return damage_step == domain::DamageStep::PERFORM_CALCULATION;
		}

		// 直接改变 ATK/DEF:仅伤害步骤开始 / 伤害计算前
		if (properties.contains(dsl::EffectProperty::ATK_DEF_CHANGE))
		{
			return damage_step == domain::DamageStep::START ||
			       damage_step == domain::DamageStep::BEFORE_CALCULATION;
		}

		// 卡片文本明确允许伤害步骤:具体窗口由脚本负责
		return properties.contains(dsl::EffectProperty::DAMAGE_STEP);
	}

	auto Duel::find_summon_procedure(const state::CardInstance& card, const domain::SummonMethod method) const noexcept -> const dsl::SummonProcedure*
	{
		const auto* script = interpreter_.find_script(card.origin_code());
		if (script == nullptr)
		{
			return nullptr;
		}

		for (const auto& summon: script->summons)
		{
			if (method == domain::SummonMethod::SYNCHRO && std::holds_alternative<dsl::SynchroProcedure>(summon))
			{
				return &summon;
			}
			if (method == domain::SummonMethod::XYZ && std::holds_alternative<dsl::XyzProcedure>(summon))
			{
				return &summon;
			}
			if (method == domain::SummonMethod::FUSION && std::holds_alternative<dsl::FusionProcedure>(summon))
			{
				return &summon;
			}
			if (method == domain::SummonMethod::LINK && std::holds_alternative<dsl::LinkProcedure>(summon))
			{
				return &summon;
			}
			if (method == domain::SummonMethod::RITUAL && std::holds_alternative<dsl::RitualProcedure>(summon))
			{
				return &summon;
			}
			if (method == domain::SummonMethod::SPECIAL && std::holds_alternative<dsl::SelfSpecialProcedure>(summon))
			{
				return &summon;
			}
		}

		return nullptr;
	}

	auto Duel::all_summon_materials_valid(const state::CardInstance& card, const std::span<const state::CardInstanceId> materials) const noexcept -> bool
	{
		return all_summon_materials_valid(
			card,
			materials,
			utility::EnumMask{domain::Zone::MONSTER, domain::Zone::EXTRA_MONSTER}
		);
	}

	auto Duel::all_summon_materials_valid(
		const state::CardInstance& card,
		const std::span<const state::CardInstanceId> materials,
		const utility::EnumMask<domain::Zone> allowed_zones
	) const noexcept -> bool
	{
		const auto controller = card.controller();

		for (const auto material: materials)
		{
			if (material == card.id() || std::ranges::count(materials, material) != 1)
			{
				return false;
			}

			const auto* instance = game_.find_instance(material);
			if (instance == nullptr || instance->controller() != controller)
			{
				return false;
			}

			if (const auto zone = game_.zone_of(material);
				!allowed_zones.contains(zone))
			{
				return false;
			}

			// 手牌素材没有场上位置;场上素材必须是表侧表示
			if (const auto zone = game_.zone_of(material);
				zone == domain::Zone::MONSTER || zone == domain::Zone::EXTRA_MONSTER)
			{
				const auto field_info = domain::field_info_of(instance->placement());
				if (!field_info.has_value())
				{
					return false;
				}

				if (field_info->position != domain::Position::FACE_UP_ATTACK && field_info->position != domain::Position::FACE_UP_DEFENSE)
				{
					return false;
				}
			}
		}

		return true;
	}

	auto Duel::match_summon_materials(
		const state::CardInstance& card,
		const dsl::Expression& predicate,
		const std::span<const state::CardInstanceId> materials
	) noexcept -> std::optional<execution::Group>
	{
		execution::Group candidates{};
		candidates.append_range(materials);

		execution::Context context{card.controller(), card.id()};
		return interpreter_.filter(predicate, candidates, context);
	}

	auto Duel::validate_fusion_materials(
		const state::CardInstance& summon_card,
		const dsl::FusionProcedure& procedure,
		const std::span<const state::CardInstanceId> materials
	) noexcept -> bool
	{
		if (procedure.materials.empty() ||
		    !all_summon_materials_valid(
			    summon_card,
			    materials,
			    utility::EnumMask{domain::Zone::HAND, domain::Zone::MONSTER, domain::Zone::EXTRA_MONSTER}
		    ))
		{
			return false;
		}

		const auto required = std::ranges::fold_left(
			procedure.materials | std::views::transform(&dsl::MaterialSpec::count),
			std::size_t{0},
			std::plus{}
		);

		if (required != materials.size())
		{
			return false;
		}

		std::vector used(materials.size(), false);

		const auto material_matches = [&](const dsl::MaterialSpec& spec, const state::CardInstanceId material) noexcept -> bool
		{
			const auto* instance = game_.find_instance(material);
			if (instance == nullptr)
			{
				return false;
			}

			return std::visit(
				utility::Overloaded
				{
						[&](const domain::CardCode code) noexcept -> bool
						{
							return interpreter_.state_resolver().has_code(*instance, code);
						},
						[&](const dsl::Expression& expression) noexcept -> bool
						{
							const auto candidates = match_summon_materials(summon_card, expression, {&material, 1});
							return candidates.has_value() && !candidates->empty();
						}
				},
				spec.selector
			);
		};

		// 按素材声明顺序回溯分配,避免贪婪匹配把通用素材占给后续的特定素材
		const auto assign = [&](
			this auto&& self,
			std::size_t spec_index,
			const std::size_t start,
			const std::size_t remaining
		) noexcept -> bool
		{
			if (spec_index == procedure.materials.size())
			{
				return std::ranges::all_of(used, std::identity{});
			}

			if (remaining == 0)
			{
				spec_index += 1;
				if (spec_index == procedure.materials.size())
				{
					return std::ranges::all_of(used, std::identity{});
				}

				return self(spec_index, 0, static_cast<std::size_t>(procedure.materials[spec_index].count));
			}

			for (std::size_t index = start; index < materials.size(); ++index)
			{
				if (used[index] || !material_matches(procedure.materials[spec_index], materials[index]))
				{
					continue;
				}

				used[index] = true;
				if (self(spec_index, index + 1, remaining - 1))
				{
					return true;
				}
				used[index] = false;
			}

			return false;
		};

		return assign(0, 0, static_cast<std::size_t>(procedure.materials.front().count));
	}

	auto Duel::validate_synchro_materials(
		const state::CardInstance& card,
		const dsl::SynchroProcedure& procedure,
		const std::span<const state::CardInstanceId> materials
	) noexcept -> bool
	{
		if (materials.size() < 2 || !all_summon_materials_valid(card, materials))
		{
			return false;
		}

		const auto tuners = match_summon_materials(card, procedure.tuner, materials);
		const auto non_tuners = match_summon_materials(card, procedure.non_tuner, materials);
		if (!tuners.has_value() || tuners->empty() || !non_tuners.has_value() || non_tuners->empty())
		{
			return false;
		}

		// tuner / non-tuner 必须是不同素材
		auto has_distinct = false;
		for (const auto tuner: *tuners)
		{
			for (const auto non_tuner: *non_tuners)
			{
				if (tuner != non_tuner)
				{
					has_distinct = true;
					break;
				}
			}
			if (has_distinct)
			{
				break;
			}
		}
		if (!has_distinct)
		{
			return false;
		}

		// 每个素材至少命中 tuner 或 non_tuner 之一
		for (const auto material: materials)
		{
			if (!std::ranges::contains(*tuners, material) && !std::ranges::contains(*non_tuners, material))
			{
				return false;
			}
		}

		const auto target_level = interpreter_.state_resolver().level(card);
		std::underlying_type_t<domain::LevelRank> total_level{};
		for (const auto material: materials)
		{
			const auto* instance = game_.find_instance(material);
			if (instance == nullptr)
			{
				return false;
			}
			total_level += interpreter_.state_resolver().level(*instance);
		}

		return total_level == target_level;
	}

	auto Duel::validate_xyz_materials(
		const state::CardInstance& summon_card,
		const dsl::XyzProcedure& procedure,
		const std::span<const state::CardInstanceId> materials
	) noexcept -> bool
	{
		if (materials.size() < 2 || !all_summon_materials_valid(summon_card, materials))
		{
			return false;
		}

		if (procedure.count != 0 && materials.size() != static_cast<std::size_t>(procedure.count))
		{
			return false;
		}

		const auto matched = match_summon_materials(summon_card, procedure.material, materials);
		if (!matched.has_value() || matched->size() != materials.size())
		{
			return false;
		}

		const auto target_level = interpreter_.state_resolver().level(summon_card);
		std::optional<std::underlying_type_t<domain::LevelRank>> material_level{};
		for (const auto material: materials)
		{
			const auto* instance = game_.find_instance(material);
			if (instance == nullptr)
			{
				return false;
			}

			const auto level = interpreter_.state_resolver().level(*instance);
			if (!material_level.has_value())
			{
				material_level = level;
			}
			else if (*material_level != level)
			{
				return false;
			}
		}

		return material_level.has_value() && *material_level == target_level;
	}

	auto Duel::validate_link_materials(
		const state::CardInstance& summon_card,
		const dsl::LinkProcedure& procedure,
		const std::span<const state::CardInstanceId> materials
	) noexcept -> bool
	{
		if (materials.empty() || !all_summon_materials_valid(summon_card, materials))
		{
			return false;
		}

		if (procedure.count != 0 && materials.size() != static_cast<std::size_t>(procedure.count))
		{
			return false;
		}

		// 连接怪兽的连接评级等于其连接标记数量，也等于所用连接素材数量。
		if (summon_card.origin_link_marker().size() != materials.size())
		{
			return false;
		}

		const auto matched = match_summon_materials(summon_card, procedure.material, materials);
		return matched.has_value() && matched->size() == materials.size();
	}

	auto Duel::validate_ritual_materials(
		const state::CardInstance& summon_card,
		const dsl::RitualProcedure& procedure,
		const std::span<const state::CardInstanceId> materials
	) noexcept -> bool
	{
		if (
			materials.empty() ||
			!all_summon_materials_valid(
				summon_card,
				materials,
				utility::EnumMask{domain::Zone::HAND, domain::Zone::MONSTER, domain::Zone::EXTRA_MONSTER}
			)
		)
		{
			return false;
		}


		if (const auto matched = match_summon_materials(summon_card, procedure.material, materials);
			!matched.has_value() || matched->size() != materials.size())
		{
			return false;
		}

		execution::Context context{summon_card.controller(), summon_card.id()};
		const auto target = interpreter_.evaluate(procedure.level, context);
		if (!target.has_value())
		{
			return false;
		}

		const auto* target_level = std::get_if<execution::Integer>(&*target);
		if (target_level == nullptr)
		{
			return false;
		}

		std::uint64_t total_level{};
		for (const auto material: materials)
		{
			const auto* instance = game_.find_instance(material);
			if (instance == nullptr)
			{
				return false;
			}

			total_level += interpreter_.state_resolver().level(*instance);
		}

		return std::cmp_equal(total_level, static_cast<execution::Integer::value_type>(*target_level));
	}

	auto Duel::summonable_instance_of(const state::CardInstanceId card, const domain::AnyMonsterPlacement placement) noexcept -> state::CardInstance*
	{
		// 连锁中只能发动效果,不能进行召唤等行动
		if (!chain_.empty())
		{
			return nullptr;
		}

		if (const auto phase = turn_.phase();
			phase != domain::Phase::MAIN_1 && phase != domain::Phase::MAIN_2)
		{
			return nullptr;
		}

		auto* instance = game_.find_instance_mutable(card);
		if (instance == nullptr)
		{
			return nullptr;
		}

		const auto controller = instance->controller();
		if (controller != turn_.turn_player())
		{
			return nullptr;
		}

		if (priority_.holder_player(turn_.turn_player()) != controller)
		{
			return nullptr;
		}

		if (const auto monster = interpreter_.state_resolver().contains_card_type(*instance, domain::CardType::MONSTER);
			!monster)
		{
			return nullptr;
		}

		if (!game_.is_location_empty(controller, domain::location_of(placement)))
		{
			return nullptr;
		}

		return instance;
	}

	auto Duel::begin_summon(SummonState summon, const domain::EventCode declared) noexcept -> bool
	{
		if (summon_.has_value())
		{
			return false;
		}

		// 清掉上一次可能残留的标记
		summon_negated_ = false;

		const auto card = summon.card.get().id();
		summon_ = std::move(summon);

		const execution::EventWindow window
		{
				.source = dsl::TriggerSource{declared},
				.card = card,
				.player = std::nullopt,
				.amount = std::nullopt,
		};
		open_timing({&window, 1});

		return true;
	}

	auto Duel::complete_summon(SummonState& summon) noexcept -> bool
	{
		auto& instance = summon.card.get();

		if (summon.method == domain::SummonMethod::NORMAL || summon.method == domain::SummonMethod::TRIBUTE)
		{
			game_.place(instance, domain::placement_of(summon.placement));
			trigger_event(dsl::TriggerSource{domain::EventCode::NORMAL_SUMMON_SUCCESS}, instance.id());
			return true;
		}

		const auto from = instance.placement();

		if (summon.method == domain::SummonMethod::FLIP)
		{
			game_.place(instance, domain::placement_of(summon.placement));
			game_.set_summon_record(
				instance,
				{
						.player = summon.controller,
						.method = domain::SummonMethod::FLIP,
						.from = from,
						.materials = {},
				}
			);

			priority_.reset();
			const std::array windows
			{
					execution::EventWindow
					{
							.source = dsl::TriggerSource{domain::EventCode::FLIP_SUMMON_SUCCESS},
							.card = instance.id(),
							.player = std::nullopt,
							.amount = std::nullopt,
					},
					execution::EventWindow
					{
							.source = dsl::TriggerSource{domain::EventCode::FLIP},
							.card = instance.id(),
							.player = std::nullopt,
							.amount = std::nullopt,
					},
			};
			open_timing(windows);
			return true;
		}
		game_.place(instance, domain::placement_of(summon.placement));

		auto materials = summon.materials | std::views::transform(&state::CardInstance::id) | std::ranges::to<std::vector<state::CardInstanceId>>();
		game_.set_summon_record(
			instance,
			{
					.player = summon.controller,
					.method = summon.method,
					.from = from,
					.materials = std::move(materials),
			}
		);

		if (summon.method == domain::SummonMethod::XYZ)
		{
			for (const auto& material: summon.materials | std::views::transform(&std::reference_wrapper<state::CardInstance>::get))
			{
				game_.add_overlay(instance, material);
			}
		}

		priority_.reset();
		trigger_event(dsl::TriggerSource{domain::EventCode::SPECIAL_SUMMON_SUCCESS}, instance.id());

		return true;
	}

	auto Duel::negate_pending_summon(SummonState& summon) noexcept -> void
	{
		auto& instance = summon.card.get();
		game_.move_to_graveyard(instance, {domain::Reason::RULE});

		for (auto& material: summon.materials | std::views::transform(&std::reference_wrapper<state::CardInstance>::get))
		{
			// 同调/祭品素材已在宣言前送墓;仅超量素材仍处于NOT_EXIST
			if (game_.zone_of(material.id()) == domain::Zone::NOT_EXIST)
			{
				game_.move_to_graveyard(material, {domain::Reason::RULE});
			}
		}

		const auto event = summon.method == domain::SummonMethod::NORMAL || summon.method == domain::SummonMethod::TRIBUTE
			                   ? domain::EventCode::NORMAL_SUMMON_NEGATED
			                   : (summon.method == domain::SummonMethod::FLIP
				                      ? domain::EventCode::FLIP_SUMMON_NEGATED
				                      : domain::EventCode::SPECIAL_SUMMON_NEGATED);
		const execution::EventWindow window
		{
				.source = dsl::TriggerSource{event},
				.card = instance.id(),
				.player = std::nullopt,
				.amount = std::nullopt,
		};
		open_timing({&window, 1});
	}

	auto Duel::resolve_summon() noexcept -> void
	{
		if (!summon_.has_value())
		{
			return;
		}

		auto summon = std::move(*summon_);
		summon_.reset();

		if (const auto negated = std::exchange(summon_negated_, false);
			negated)
		{
			negate_pending_summon(summon);
			return;
		}

		std::ignore = complete_summon(summon);
	}

	auto Duel::perform_normal_summon(const state::CardInstanceId card, const domain::MonsterPlacement placement) noexcept -> bool
	{
		// 如果已通招或已有召唤尚未结算则失败
		if (summon_.has_value() || state_.normal_summoned)
		{
			return false;
		}

		auto* instance = summonable_instance_of(card, {placement});
		if (instance == nullptr)
		{
			return false;
		}

		if (const auto zone = game_.zone_of(card);
			zone != domain::Zone::HAND)
		{
			return false;
		}

		// 等级 5 以上需要祭品(使用 tribute_summon / tribute_set)
		if (const auto level = interpreter_.state_resolver().level(*instance);
			level >= 5)
		{
			return false;
		}

		// 已通常召唤(即使召唤被无效也计入次数)
		state_.normal_summoned = true;

		// 盖放不是召唤,直接成功
		if (placement.position == domain::Position::FACE_DOWN_DEFENSE)
		{
			game_.place(*instance, {placement});
			set_turns_.insert_or_assign(card, turn_.turn_id());
			trigger_event(dsl::TriggerSource{domain::EventCode::MONSTER_SET}, card);
			return true;
		}

		return begin_summon(
			SummonState
			{
					.controller = instance->controller(),
					.placement = domain::AnyMonsterPlacement{placement},
					.method = domain::SummonMethod::NORMAL,
					.card = std::ref(*instance),
					.materials = {},
			},
			domain::EventCode::NORMAL_SUMMON_DECLARED
		);
	}

	auto Duel::perform_tribute_summon(const state::CardInstanceId card, const domain::MonsterPlacement placement, const std::span<const state::CardInstanceId> tributes) noexcept -> bool
	{
		// 如果已通招或已有召唤尚未结算则失败
		if (summon_.has_value() || state_.normal_summoned)
		{
			return false;
		}

		auto* instance = summonable_instance_of(card, {placement});
		if (instance == nullptr)
		{
			return false;
		}

		if (const auto zone = game_.zone_of(card);
			zone != domain::Zone::HAND)
		{
			return false;
		}

		const auto level = interpreter_.state_resolver().level(*instance);
		// 只有等级 5+ 才需要/允许上级召唤
		if (level < 5)
		{
			return false;
		}

		// 等级 5~6 需要 1 个祭品,等级 7+ 需要 2 个
		if (const auto required = level >= 7 ? std::size_t{2} : std::size_t{1};
			tributes.size() != required)
		{
			return false;
		}

		const auto controller = instance->controller();
		for (const auto [index, tribute]: tributes | std::views::enumerate)
		{
			// 祭品不能是自己且不能重复
			if (tribute == card || std::ranges::count(tributes, tribute) != 1)
			{
				return false;
			}

			const auto* tribute_instance = game_.find_instance(tribute);

			if (tribute_instance == nullptr)
			{
				return false;
			}

			// 必须是自己控制的怪兽
			if (const auto tribute_controller = tribute_instance->controller();
				tribute_controller != controller)
			{
				return false;
			}

			// 必须是场上的怪兽
			if (const auto zone = game_.zone_of(tribute);
				zone != domain::Zone::MONSTER && zone != domain::Zone::EXTRA_MONSTER)
			{
				return false;
			}
		}

		// 解放祭品(即使召唤被无效也不返还)
		std::vector<std::reference_wrapper<state::CardInstance>> summon_materials{};
		summon_materials.reserve(tributes.size());
		for (const auto tribute: tributes)
		{
			auto* tribute_instance = game_.find_instance_mutable(tribute);
			LIBASSERT_DEBUG_ASSERT(tribute_instance != nullptr);
			game_.move_to_graveyard(*tribute_instance, {domain::Reason::RELEASE});

			summon_materials.push_back(std::ref(*tribute_instance));
		}

		// 已通常召唤(即使召唤被无效也计入次数)
		state_.normal_summoned = true;

		// 盖放不是召唤,直接成功
		if (placement.position == domain::Position::FACE_DOWN_DEFENSE)
		{
			game_.place(*instance, {placement});
			set_turns_.insert_or_assign(card, turn_.turn_id());
			trigger_event(dsl::TriggerSource{domain::EventCode::MONSTER_SET}, card);
			return true;
		}

		return begin_summon(
			SummonState
			{
					.controller = controller,
					.placement = domain::AnyMonsterPlacement{placement},
					.method = domain::SummonMethod::TRIBUTE,
					.card = std::ref(*instance),
					.materials = std::move(summon_materials),
			},
			domain::EventCode::NORMAL_SUMMON_DECLARED
		);
	}

	auto Duel::usage_owner(const dsl::UsageKind kind, const state::CardInstance& card) const noexcept -> UsageKey::owner_type
	{
		if (kind == dsl::UsageKind::TURN_PER_CARD)
		{
			return card.id().raw();
		}

		return std::to_underlying(interpreter_.state_resolver().effective_code(card));
	}

	auto Duel::make_usage_key(const dsl::Effect& effect, const state::CardInstance& card) const noexcept -> UsageKey
	{
		LIBASSERT_DEBUG_ASSERT(effect.usage.has_value());

		return
		{
				.owner = usage_owner(effect.usage->kind, card), // NOLINT(bugprone-unchecked-optional-access)
				.kind = effect.usage->kind, // NOLINT(bugprone-unchecked-optional-access)
				.counter = effect.id,
		};
	}

	auto Duel::make_mutex_key(const dsl::Effect& effect, const state::CardInstance& card) const noexcept -> MutexKey
	{
		LIBASSERT_DEBUG_ASSERT(effect.mutex.has_value());

		return
		{
				.owner = interpreter_.state_resolver().effective_code(card),
				.group = *effect.mutex,
		};
	}

	auto Duel::can_use(const dsl::Effect& effect, const state::CardInstance& card) const noexcept -> bool
	{
		if (effect.usage.has_value())
		{
			const auto key = make_usage_key(effect, card);
			const auto it = usage_.find(key);

			if (const auto used = it == usage_.end() ? dsl::UsageRestriction::size_type{0} : it->second;
				used >= effect.usage->count)
			{
				return false;
			}
		}

		if (effect.mutex.has_value())
		{
			const auto key = make_mutex_key(effect, card);

			if (const auto it = mutex_choices_.find(key);
				it != mutex_choices_.end() && it->second != effect.id)
			{
				return false;
			}
		}

		return true;
	}

	auto Duel::mark_used(const dsl::Effect& effect, const state::CardInstance& card) noexcept -> bool
	{
		if (effect.usage.has_value())
		{
			const auto key = make_usage_key(effect, card);

			usage_[key] += 1;
		}

		auto mutex_inserted = false;
		if (effect.mutex.has_value())
		{
			const auto key = make_mutex_key(effect, card);

			mutex_inserted = mutex_choices_.try_emplace(key, effect.id).second;
		}

		return mutex_inserted;
	}

	auto Duel::unmark_used(const dsl::Effect& effect, const state::CardInstance& card, const bool mutex_inserted) noexcept -> void
	{
		if (effect.usage.has_value())
		{
			const auto key = make_usage_key(effect, card);

			if (const auto it = usage_.find(key);
				it != usage_.end() && it->second > 0)
			{
				it->second -= 1;
			}
		}

		if (effect.mutex.has_value() && mutex_inserted)
		{
			const auto key = make_mutex_key(effect, card);

			if (const auto it = mutex_choices_.find(key);
				it != mutex_choices_.end() && it->second == effect.id)
			{
				mutex_choices_.erase(it);
			}
		}
	}

	auto Duel::is_on_field(const state::CardInstanceId card) const noexcept -> bool
	{
		const auto zone = game_.zone_of(card);
		return zone == domain::Zone::MONSTER || zone == domain::Zone::EXTRA_MONSTER;
	}

	auto Duel::advance_battle() noexcept -> void
	{
		if (!attack_.has_value())
		{
			return;
		}

		auto& attack = *attack_;

		if (attack.replay_required)
		{
			const auto* attacker = game_.find_instance(attack.attacker);
			const auto field_info = attacker == nullptr ? std::optional<domain::FieldInfo>{} : domain::field_info_of(attacker->placement());
			if (!field_info.has_value() || field_info->position != domain::Position::FACE_UP_ATTACK)
			{
				clear_attack();
			}

			return;
		}

		if (battle_.battle_step() == domain::BattleStep::BATTLE)
		{
			if (!is_on_field(attack.attacker))
			{
				clear_attack();
				return;
			}

			const auto* attacker_instance = game_.find_instance(attack.attacker);
			const auto attacker_field_info = attacker_instance == nullptr
				                                 ? std::optional<domain::FieldInfo>{}
				                                 : domain::field_info_of(attacker_instance->placement());
			if (!attacker_field_info.has_value() || attacker_field_info->position != domain::Position::FACE_UP_ATTACK)
			{
				clear_attack();
				return;
			}

			battle_.advance_battle_step(); // BATTLE → DAMAGE
			open_battle_window(dsl::TriggerSource{dsl::PeriodRef{domain::DamageStep::START}});
			return;
		}

		if (battle_.battle_step() != domain::BattleStep::DAMAGE)
		{
			clear_attack();
			return;
		}

		switch (battle_.damage_step())
		{
			case domain::DamageStep::START:
			{
				if (!is_on_field(attack.attacker))
				{
					clear_attack();
					return;
				}

				if (attack.target.has_value() && !is_on_field(*attack.target))
				{
					attack.target.reset();
					attack.replay_required = true;
					attack.target_flipped = false;
					attack.target_position = domain::Position::FACE_UP_ATTACK;
					priority_.reset();
					windows_.clear();
					return;
				}

				if (!prepare_attack_target(attack))
				{
					clear_attack();
					return;
				}
				battle_.advance_damage_step(); // START → BEFORE
				open_battle_window(dsl::TriggerSource{dsl::PeriodRef{domain::DamageStep::BEFORE_CALCULATION}});
				break;
			}
			case domain::DamageStep::BEFORE_CALCULATION:
			{
				battle_.advance_damage_step(); // BEFORE → PERFORM
				open_battle_window(dsl::TriggerSource{dsl::PeriodRef{domain::DamageStep::PERFORM_CALCULATION}});
				break;
			}
			case domain::DamageStep::PERFORM_CALCULATION:
			{
				if (!calculate_battle(attack))
				{
					clear_attack();
					return;
				}
				battle_.advance_damage_step(); // PERFORM → AFTER
				open_after_calculation_window(attack);
				break;
			}
			case domain::DamageStep::AFTER_CALCULATION:
			{
				battle_.advance_damage_step(); // AFTER → END
				finish_damage_step(attack);
				break;
			}
			case domain::DamageStep::END:
			{
				clear_attack();
				break;
			}
		}
	}

	auto Duel::open_battle_window(const dsl::TriggerSource source) noexcept -> void
	{
		const execution::EventWindow window{.source = source, .card = std::nullopt, .player = std::nullopt, .amount = std::nullopt};

		open_timing({&window, 1});
	}

	auto Duel::clear_attack() noexcept -> void
	{
		attack_.reset();
		battle_.reset();
		// START → BATTLE:下一次攻击仍从战斗步骤开始
		battle_.advance_battle_step();
		priority_.reset();
		windows_.clear();
	}

	auto Duel::prepare_attack_target(AttackState& attack) noexcept -> bool
	{
		if (!attack.target.has_value())
		{
			return true;
		}

		if (!is_on_field(*attack.target))
		{
			return false;
		}

		auto* target_instance = game_.find_instance_mutable(*attack.target);
		if (target_instance == nullptr)
		{
			return false;
		}

		const auto placement = target_instance->placement();
		const auto field_info = domain::field_info_of(placement);
		if (!field_info.has_value())
		{
			return false;
		}

		attack.target_position = field_info->position;
		if (attack.target_position != domain::Position::FACE_DOWN_DEFENSE)
		{
			return true;
		}

		attack.target_position = domain::Position::FACE_UP_DEFENSE;
		attack.target_flipped = true;

		if (std::holds_alternative<domain::ExtraMonsterPlacement>(placement))
		{
			game_.place(*target_instance, domain::ExtraMonsterPlacement{.column = field_info->column, .position = attack.target_position});
		}
		else
		{
			game_.place(*target_instance, domain::MonsterPlacement{.column = field_info->column, .position = attack.target_position});
		}

		return true;
	}

	auto Duel::calculate_battle(AttackState& attack) noexcept -> bool
	{
		const auto* attacker_instance = game_.find_instance(attack.attacker);
		if (attacker_instance == nullptr || !is_on_field(attack.attacker))
		{
			return false;
		}

		const auto attacker_attack = interpreter_.state_resolver().attack(*attacker_instance);
		const auto controller = attack.controller;
		const auto opponent = attack.opponent;

		attack.destroy_attacker.reset();
		attack.destroy_target.reset();
		attack.damage_to_opponent = 0;
		attack.damage_to_controller = 0;

		if (!attack.target.has_value())
		{
			attack.damage_to_opponent = attacker_attack;
		}
		else
		{
			const auto* target_instance = game_.find_instance(*attack.target);
			if (target_instance == nullptr || !is_on_field(*attack.target))
			{
				return false;
			}

			if (attack.target_position == domain::Position::FACE_UP_ATTACK)
			{
				if (const auto target_attack = interpreter_.state_resolver().attack(*target_instance);
					attacker_attack > target_attack)
				{
					attack.destroy_target = *attack.target;
					attack.damage_to_opponent = attacker_attack - target_attack;
				}
				else if (attacker_attack < target_attack)
				{
					attack.destroy_attacker = attack.attacker;
					attack.damage_to_controller = target_attack - attacker_attack;
				}
				else
				{
					attack.destroy_attacker = attack.attacker;
					attack.destroy_target = *attack.target;
				}
			}
			else
			{
				if (const auto target_defense = interpreter_.state_resolver().defense(*target_instance);
					attacker_attack > target_defense)
				{
					attack.destroy_target = *attack.target;
				}
				else if (attacker_attack < target_defense)
				{
					attack.damage_to_controller = target_defense - attacker_attack;
				}
			}
		}

		if (attack.damage_to_opponent != 0)
		{
			game_.change_life_points(opponent, -attack.damage_to_opponent);
		}
		if (attack.damage_to_controller != 0)
		{
			game_.change_life_points(controller, -attack.damage_to_controller);
		}

		return true;
	}

	auto Duel::finish_damage_step(AttackState& attack) noexcept -> void
	{
		std::vector<execution::EventWindow> windows{};

		const auto add_destroyed = [&](const std::optional<state::CardInstanceId>& card) -> void
		{
			if (!card.has_value() || !is_on_field(*card))
			{
				return;
			}

			auto* instance = game_.find_instance_mutable(*card);
			if (instance == nullptr)
			{
				return;
			}

			if (interpreter_.state_resolver().contains_protection(*instance, domain::Protection::BATTLE))
			{
				return;
			}

			game_.move_to_graveyard(*instance, {domain::Reason::BATTLE, domain::Reason::BATTLE_DESTROY});
			windows.emplace_back(dsl::TriggerSource{domain::EventCode::BATTLE_DESTROYED}, *card);
		};

		add_destroyed(attack.destroy_target);
		add_destroyed(attack.destroy_attacker);

		if (attack.target_flipped && attack.target.has_value())
		{
			windows.emplace_back(dsl::TriggerSource{domain::EventCode::FLIP}, attack.target);
		}

		windows.emplace_back(dsl::TriggerSource{dsl::PeriodRef{domain::DamageStep::END}}, std::nullopt);

		open_timing(windows);
	}

	auto Duel::open_after_calculation_window(AttackState& attack) noexcept -> void
	{
		std::vector<execution::EventWindow> windows{};

		if (attack.damage_to_opponent != 0)
		{
			windows.push_back(
				execution::EventWindow
				{
						.source = dsl::TriggerSource{domain::EventCode::BATTLE_DAMAGE},
						.card = attack.attacker,
						.player = attack.opponent,
						.amount = attack.damage_to_opponent,
				}
			);
		}

		if (attack.damage_to_controller != 0)
		{
			windows.push_back(
				execution::EventWindow
				{
						.source = dsl::TriggerSource{domain::EventCode::BATTLE_DAMAGE},
						.card = attack.attacker,
						.player = attack.controller,
						.amount = attack.damage_to_controller,
				}
			);
		}

		windows.push_back(
			execution::EventWindow
			{
					.source = dsl::TriggerSource{dsl::PeriodRef{domain::DamageStep::AFTER_CALCULATION}},
					.card = std::nullopt,
					.player = std::nullopt,
					.amount = std::nullopt,
			}
		);

		open_timing(windows);
	}

	Duel::Duel(execution::Selector& selector) noexcept
		:
		// game_{},
		// turn_{},
		// priority_{},
		// battle_{},
		// state_{},
		loser_{std::nullopt},
		// types_{},
		vocabulary_{types_},
		selector_{selector},
		interpreter_{game_, types_, vocabulary_, selector_} {}

	auto Duel::load_scripts(const std::filesystem::path& directory) noexcept -> bool
	{
		return interpreter_.load_scripts(directory);
	}

	auto Duel::load_script(const std::filesystem::path& file) noexcept -> bool
	{
		return interpreter_.load_script(file);
	}

	auto Duel::game() const noexcept -> const state::Game&
	{
		return game_;
	}

	auto Duel::turn() const noexcept -> const Turn&
	{
		return turn_;
	}

	auto Duel::priority() const noexcept -> const Priority&
	{
		return priority_;
	}

	auto Duel::battle() const noexcept -> const Battle&
	{
		return battle_;
	}

	auto Duel::pending_attack() const noexcept -> bool
	{
		return attack_.has_value();
	}

	auto Duel::pending_summon() const noexcept -> bool
	{
		return summon_.has_value();
	}

	auto Duel::chain() const noexcept -> const execution::Chain&
	{
		return chain_;
	}

	auto Duel::winner() const noexcept -> std::optional<domain::Player>
	{
		if (loser_.has_value())
		{
			return domain::opponent_of(*loser_);
		}

		const auto first = game_.life_points(domain::Player::FIRST);
		const auto second = game_.life_points(domain::Player::SECOND);

		if (first <= 0 && second <= 0)
		{
			return std::nullopt;
		}
		if (first <= 0)
		{
			return domain::Player::SECOND;
		}
		if (second <= 0)
		{
			return domain::Player::FIRST;
		}

		return std::nullopt;
	}

	auto Duel::draw_for_turn() noexcept -> bool
	{
		if (!chain_.empty())
		{
			return false;
		}

		if (turn_.phase() != domain::Phase::DRAW || state_.drew_in_draw_phase)
		{
			return false;
		}

		const auto player = turn_.turn_player();
		const auto deck = game_.collect_zone_card(player, domain::Zone::DECK);
		if (deck.empty())
		{
			// 必须抽卡时卡组为空 → 判负
			loser_ = player;
			return false;
		}

		auto* instance = game_.find_instance_mutable(deck.front());
		if (instance == nullptr)
		{
			return false;
		}

		// 移动到手牌
		game_.move_to_hand(*instance, {domain::Reason::DRAW}, domain::Position::FACE_DOWN_DEFENSE);

		// 抽卡后旧的事件时点不再有效(抽卡本身是否开时点待补)
		windows_.clear();

		// 已抽牌
		state_.drew_in_draw_phase = true;
		return true;
	}

	auto Duel::change_phase() noexcept -> bool
	{
		if (!chain_.empty() || summon_.has_value() || attack_.has_value() || !priority_.both_passed())
		{
			return false;
		}

		const auto previous_phase = turn_.phase();
		if (!turn_.advance_phase())
		{
			return false;
		}

		// 离开战斗阶段时清理战斗步骤
		if (previous_phase == domain::Phase::BATTLE)
		{
			battle_.reset();
		}

		priority_.reset();
		windows_.clear();
		return true;
	}

	auto Duel::pass_priority() noexcept -> bool
	{
		if (!chain_.empty())
		{
			// 连锁响应中:当前持有者放弃 → 整条连锁结算
			resolve_chain();
			return true;
		}

		priority_.pass();

		if (!priority_.both_passed())
		{
			return false;
		}

		// 双方通过后关闭事件时点
		windows_.clear();

		// 召唤宣言时点关闭:结算召唤
		if (summon_.has_value())
		{
			resolve_summon();
			return true;
		}

		// 战斗中:当前战斗/伤害步骤时点关闭 → 进入下一步骤
		if (attack_.has_value())
		{
			advance_battle();
		}

		return true;
	}

	auto Duel::end_turn() noexcept -> bool
	{
		if (turn_.phase() != domain::Phase::END || !chain_.empty() || summon_.has_value() || attack_.has_value())
		{
			return false;
		}

		turn_.end_turn();
		priority_.reset();
		battle_.reset();
		attack_.reset();
		windows_.clear();
		state_ = TurnState{};

		// 清除本回合的使用记录(DUEL范围保留)
		std::erase_if(
			usage_,
			[](const std::pair<const UsageKey&, const dsl::UsageRestriction::size_type>& p) noexcept -> bool
			{
				return p.first.kind == dsl::UsageKind::TURN_PER_CARD || p.first.kind == dsl::UsageKind::TURN_PER_NAME;
			}
		);

		mutex_choices_.clear();

		return true;
	}

	auto Duel::normal_summon(const state::CardInstanceId card, const domain::FieldColumn column) noexcept -> bool
	{
		return perform_normal_summon(card, {.column = column, .position = domain::Position::FACE_UP_ATTACK});
	}

	auto Duel::flip_summon(const state::CardInstanceId card) noexcept -> bool
	{
		if (summon_.has_value() || !chain_.empty())
		{
			return false;
		}

		if (const auto phase = turn_.phase(); phase != domain::Phase::MAIN_1 && phase != domain::Phase::MAIN_2)
		{
			return false;
		}

		auto* instance = game_.find_instance_mutable(card);
		if (instance == nullptr)
		{
			return false;
		}

		const auto controller = instance->controller();
		if (controller != turn_.turn_player() || priority_.holder_player(turn_.turn_player()) != controller)
		{
			return false;
		}

		if (game_.zone_of(card) != domain::Zone::MONSTER)
		{
			return false;
		}

		if (const auto it = set_turns_.find(card); it != set_turns_.end() && it->second >= turn_.turn_id())
		{
			return false;
		}

		const auto placement = instance->placement();
		const auto* monster_placement = std::get_if<domain::MonsterPlacement>(&placement);
		if (monster_placement == nullptr || monster_placement->position != domain::Position::FACE_DOWN_DEFENSE)
		{
			return false;
		}

		return begin_summon(
			SummonState
			{
					.controller = controller,
					.placement = domain::AnyMonsterPlacement{domain::MonsterPlacement{.column = monster_placement->column, .position = domain::Position::FACE_UP_ATTACK}},
					.method = domain::SummonMethod::FLIP,
					.card = std::ref(*instance),
					.materials = {},
			},
			domain::EventCode::FLIP_SUMMON_DECLARED
		);
	}

	auto Duel::set_monster(const state::CardInstanceId card, const domain::FieldColumn column) noexcept -> bool
	{
		return perform_normal_summon(card, {.column = column, .position = domain::Position::FACE_DOWN_DEFENSE});
	}

	auto Duel::tribute_summon(const state::CardInstanceId card, const domain::FieldColumn column, const std::span<const state::CardInstanceId> tributes) noexcept -> bool
	{
		return perform_tribute_summon(card, {.column = column, .position = domain::Position::FACE_UP_ATTACK}, tributes);
	}

	auto Duel::tribute_set(const state::CardInstanceId card, const domain::FieldColumn column, const std::span<const state::CardInstanceId> tributes) noexcept -> bool
	{
		return perform_tribute_summon(card, {.column = column, .position = domain::Position::FACE_DOWN_DEFENSE}, tributes);
	}

	auto Duel::synchro_summon(const state::CardInstanceId card, const domain::AnyMonsterPlacement placement, const std::span<const state::CardInstanceId> materials) noexcept -> bool
	{
		if (summon_.has_value())
		{
			return false;
		}

		// 必须是表侧攻击/守备表示
		if (const auto position = domain::position_of(placement);
			position != domain::Position::FACE_UP_ATTACK && position != domain::Position::FACE_UP_DEFENSE)
		{
			return false;
		}

		auto* instance = summonable_instance_of(card, placement);
		if (instance == nullptr)
		{
			return false;
		}

		if (const auto zone = game_.zone_of(card);
			zone != domain::Zone::EXTRA_DECK)
		{
			return false;
		}

		// 必须是同调子类型
		if (!interpreter_.state_resolver().contains_card_subtype(*instance, domain::CardSubtype::SYNCHRO))
		{
			return false;
		}

		// 必须存在对应的同调召唤 procedure
		const auto* procedure = find_summon_procedure(*instance, domain::SummonMethod::SYNCHRO);
		if (procedure == nullptr)
		{
			return false;
		}

		const auto* synchro = std::get_if<dsl::SynchroProcedure>(procedure);
		if (synchro == nullptr || !validate_synchro_materials(*instance, *synchro, materials))
		{
			return false;
		}

		// 素材在宣言前送入墓地(召唤被无效也不返还)
		std::vector<std::reference_wrapper<state::CardInstance>> summon_materials{};
		summon_materials.reserve(materials.size());
		for (const auto material: materials)
		{
			auto* material_instance = game_.find_instance_mutable(material);
			LIBASSERT_DEBUG_ASSERT(material_instance != nullptr);
			game_.move_to_graveyard(*material_instance, {domain::Reason::SYNCHRO});

			summon_materials.push_back(std::ref(*material_instance));
		}

		return begin_summon(
			SummonState
			{
					.controller = instance->controller(),
					.placement = placement,
					.method = domain::SummonMethod::SYNCHRO,
					.card = std::ref(*instance),
					.materials = std::move(summon_materials),
			},
			domain::EventCode::SPECIAL_SUMMON_DECLARED
		);
	}

	auto Duel::xyz_summon(const state::CardInstanceId card, const domain::AnyMonsterPlacement placement, const std::span<const state::CardInstanceId> materials) noexcept -> bool
	{
		if (summon_.has_value())
		{
			return false;
		}

		// 必须是表侧攻击/守备表示
		if (const auto position = domain::position_of(placement);
			position != domain::Position::FACE_UP_ATTACK && position != domain::Position::FACE_UP_DEFENSE)
		{
			return false;
		}

		auto* instance = summonable_instance_of(card, placement);
		if (instance == nullptr)
		{
			return false;
		}

		if (const auto zone = game_.zone_of(card);
			zone != domain::Zone::EXTRA_DECK)
		{
			return false;
		}

		// 必须是超量子类型
		if (!interpreter_.state_resolver().contains_card_subtype(*instance, domain::CardSubtype::XYZ))
		{
			return false;
		}

		// 必须存在对应的超量召唤 procedure
		const auto* procedure = find_summon_procedure(*instance, domain::SummonMethod::XYZ);
		if (procedure == nullptr)
		{
			return false;
		}

		const auto* xyz = std::get_if<dsl::XyzProcedure>(procedure);
		if (xyz == nullptr || !validate_xyz_materials(*instance, *xyz, materials))
		{
			return false;
		}

		// 素材在宣言前视为已使用(送入NOT_EXIST;召唤被无效时再送墓)
		std::vector<std::reference_wrapper<state::CardInstance>> summon_materials{};
		summon_materials.reserve(materials.size());
		for (const auto material: materials)
		{
			auto* material_instance = game_.find_instance_mutable(material);
			LIBASSERT_DEBUG_ASSERT(material_instance != nullptr);
			game_.place(*material_instance, {domain::NotExistPlacement{}});

			summon_materials.push_back(std::ref(*material_instance));
		}

		return begin_summon(
			SummonState
			{
					.controller = instance->controller(),
					.placement = placement,
					.method = domain::SummonMethod::XYZ,
					.card = std::ref(*instance),
					.materials = std::move(summon_materials),
			},
			domain::EventCode::SPECIAL_SUMMON_DECLARED
		);
	}

	auto Duel::fusion_summon(const state::CardInstanceId card, const domain::AnyMonsterPlacement placement, const std::span<const state::CardInstanceId> materials) noexcept -> bool
	{
		if (summon_.has_value())
		{
			return false;
		}

		// 必须是表侧攻击/守备表示
		if (const auto position = domain::position_of(placement);
			position != domain::Position::FACE_UP_ATTACK && position != domain::Position::FACE_UP_DEFENSE)
		{
			return false;
		}

		auto* instance = summonable_instance_of(card, placement);
		if (instance == nullptr || game_.zone_of(card) != domain::Zone::EXTRA_DECK)
		{
			return false;
		}

		// 必须是融合子类型
		if (!interpreter_.state_resolver().contains_card_subtype(*instance, domain::CardSubtype::FUSION))
		{
			return false;
		}

		// 必须存在对应的融合召唤 procedure
		const auto* procedure = find_summon_procedure(*instance, domain::SummonMethod::FUSION);
		if (procedure == nullptr)
		{
			return false;
		}

		const auto* fusion = std::get_if<dsl::FusionProcedure>(procedure);
		if (fusion == nullptr || !validate_fusion_materials(*instance, *fusion, materials))
		{
			return false;
		}

		// 素材在宣言前送入墓地(召唤被无效也不返还)
		std::vector<std::reference_wrapper<state::CardInstance>> summon_materials{};
		summon_materials.reserve(materials.size());
		for (const auto material: materials)
		{
			auto* material_instance = game_.find_instance_mutable(material);
			LIBASSERT_DEBUG_ASSERT(material_instance != nullptr);
			game_.move_to_graveyard(*material_instance, {domain::Reason::FUSION});

			summon_materials.push_back(std::ref(*material_instance));
		}

		return begin_summon(
			SummonState
			{
					.controller = instance->controller(),
					.placement = placement,
					.method = domain::SummonMethod::FUSION,
					.card = std::ref(*instance),
					.materials = std::move(summon_materials),
			},
			domain::EventCode::SPECIAL_SUMMON_DECLARED
		);
	}

	auto Duel::link_summon(const state::CardInstanceId card, const domain::AnyMonsterPlacement placement, const std::span<const state::CardInstanceId> materials) noexcept -> bool
	{
		if (summon_.has_value())
		{
			return false;
		}

		// 必须是表侧攻击
		if (const auto position = domain::position_of(placement);
			position != domain::Position::FACE_UP_ATTACK)
		{
			return false;
		}

		auto* instance = summonable_instance_of(card, placement);
		if (instance == nullptr)
		{
			return false;
		}

		if (const auto zone = game_.zone_of(card);
			zone != domain::Zone::EXTRA_DECK)
		{
			return false;
		}

		// 必须是连接子类型
		if (!interpreter_.state_resolver().contains_card_subtype(*instance, domain::CardSubtype::LINK))
		{
			return false;
		}

		// 必须存在对应的连接召唤 procedure
		const auto* procedure = find_summon_procedure(*instance, domain::SummonMethod::LINK);
		if (procedure == nullptr)
		{
			return false;
		}

		const auto* link = std::get_if<dsl::LinkProcedure>(procedure);
		if (link == nullptr || !validate_link_materials(*instance, *link, materials))
		{
			return false;
		}

		// 素材在宣言前送入墓地(召唤被无效也不返还)
		std::vector<std::reference_wrapper<state::CardInstance>> summon_materials{};
		summon_materials.reserve(materials.size());
		for (const auto material: materials)
		{
			auto* material_instance = game_.find_instance_mutable(material);
			LIBASSERT_DEBUG_ASSERT(material_instance != nullptr);
			game_.move_to_graveyard(*material_instance, {domain::Reason::LINK});

			summon_materials.push_back(std::ref(*material_instance));
		}

		return begin_summon(
			SummonState
			{
					.controller = instance->controller(),
					.placement = placement,
					.method = domain::SummonMethod::LINK,
					.card = std::ref(*instance),
					.materials = std::move(summon_materials),
			},
			domain::EventCode::SPECIAL_SUMMON_DECLARED
		);
	}

	auto Duel::ritual_summon(const state::CardInstanceId card, const domain::AnyMonsterPlacement placement, const std::span<const state::CardInstanceId> materials) noexcept -> bool
	{
		if (summon_.has_value())
		{
			return false;
		}

		// 必须是表侧攻击/守备表示
		if (const auto position = domain::position_of(placement);
			position != domain::Position::FACE_UP_ATTACK && position != domain::Position::FACE_UP_DEFENSE)
		{
			return false;
		}

		auto* instance = summonable_instance_of(card, placement);
		if (instance == nullptr)
		{
			return false;
		}

		if (const auto zone = game_.zone_of(card);
			zone != domain::Zone::HAND)
		{
			return false;
		}

		// 必须是仪式子类型
		if (!interpreter_.state_resolver().contains_card_subtype(*instance, domain::CardSubtype::RITUAL))
		{
			return false;
		}

		// 必须存在对应的仪式召唤 procedure
		const auto* procedure = find_summon_procedure(*instance, domain::SummonMethod::RITUAL);
		if (procedure == nullptr)
		{
			return false;
		}

		const auto* ritual = std::get_if<dsl::RitualProcedure>(procedure);
		if (ritual == nullptr || !validate_ritual_materials(*instance, *ritual, materials))
		{
			return false;
		}

		// 素材在宣言前送入墓地(召唤被无效也不返还)
		std::vector<std::reference_wrapper<state::CardInstance>> summon_materials{};
		summon_materials.reserve(materials.size());
		for (const auto material: materials)
		{
			auto* material_instance = game_.find_instance_mutable(material);
			LIBASSERT_DEBUG_ASSERT(material_instance != nullptr);
			game_.move_to_graveyard(*material_instance, {domain::Reason::RITUAL});

			summon_materials.push_back(std::ref(*material_instance));
		}

		return begin_summon(
			SummonState
			{
					.controller = instance->controller(),
					.placement = placement,
					.method = domain::SummonMethod::RITUAL,
					.card = std::ref(*instance),
					.materials = std::move(summon_materials),
			},
			domain::EventCode::SPECIAL_SUMMON_DECLARED
		);
	}

	auto Duel::self_special_summon(const state::CardInstanceId card, const domain::AnyMonsterPlacement placement) noexcept -> bool
	{
		if (summon_.has_value())
		{
			return false;
		}

		// 必须是表侧攻击/守备表示
		if (const auto position = domain::position_of(placement);
			position != domain::Position::FACE_UP_ATTACK && position != domain::Position::FACE_UP_DEFENSE)
		{
			return false;
		}

		auto* instance = summonable_instance_of(card, placement);
		if (instance == nullptr)
		{
			return false;
		}

		// 必须存在对应的特殊召唤 procedure
		const auto* procedure = find_summon_procedure(*instance, domain::SummonMethod::SPECIAL);
		if (procedure == nullptr)
		{
			return false;
		}

		const auto* special = std::get_if<dsl::SelfSpecialProcedure>(procedure);
		if (special == nullptr)
		{
			return false;
		}

		const auto source_zone = game_.zone_of(card);
		if (special->source_zones.empty())
		{
			if (source_zone != domain::Zone::HAND)
			{
				return false;
			}
		}
		else if (!special->source_zones.contains(source_zone))
		{
			return false;
		}

		execution::Context context{instance->controller(), instance->id()};
		if (special->condition.has_value())
		{
			const auto condition = interpreter_.evaluate(*special->condition, context);
			if (!condition.has_value())
			{
				return false;
			}

			const auto* value = std::get_if<execution::Boolean>(&*condition);
			if (value == nullptr || !*value)
			{
				return false;
			}
		}

		if (!special->cost.empty() && !interpreter_.execute(special->cost, context))
		{
			return false;
		}

		return begin_summon(
			SummonState
			{
					.controller = instance->controller(),
					.placement = placement,
					.method = domain::SummonMethod::SPECIAL,
					.card = std::ref(*instance),
					.materials = {},
			},
			domain::EventCode::SPECIAL_SUMMON_DECLARED
		);
	}

	auto Duel::activate_card(const state::CardInstanceId card) noexcept -> bool
	{
		auto* instance = game_.find_instance_mutable(card);
		if (instance == nullptr)
		{
			return false;
		}

		const auto controller = instance->controller();

		if (priority_.holder_player(turn_.turn_player()) != controller)
		{
			return false;
		}

		const auto is_spell = interpreter_.state_resolver().contains_card_type(*instance, domain::CardType::SPELL);
		const auto is_trap = interpreter_.state_resolver().contains_card_type(*instance, domain::CardType::TRAP);
		if (!is_spell && !is_trap)
		{
			return false;
		}

		const auto zone = game_.zone_of(card);

		// 先确定发动后的放置位置,避免支付代价后才发现位置不可用
		std::optional<domain::Placement> target_placement;

		if (zone == domain::Zone::HAND)
		{
			// 陷阱必须先盖放
			if (is_trap)
			{
				return false;
			}
			if (controller != turn_.turn_player())
			{
				return false;
			}
			if (turn_.phase() != domain::Phase::MAIN_1 && turn_.phase() != domain::Phase::MAIN_2)
			{
				return false;
			}

			if (interpreter_.state_resolver().contains_card_subtype(*instance, domain::CardSubtype::FIELD))
			{
				if (!game_.is_zone_empty(controller, domain::Zone::FIELD_SPELL))
				{
					return false;
				}

				target_placement = domain::Placement{domain::FieldSpellPlacement{.position = domain::Position::FACE_UP_ATTACK}};
			}
			else
			{
				std::optional<domain::FieldColumn> column;
				for (const auto candidate: domain::field_columns)
				{
					if (game_.is_location_empty(controller, domain::Location{domain::SpellTrapLocation{.column = candidate}}))
					{
						column = candidate;
						break;
					}
				}

				if (!column.has_value())
				{
					return false;
				}

				target_placement = domain::Placement{domain::SpellTrapPlacement{.column = *column, .position = domain::Position::FACE_UP_ATTACK}};
			}
		}
		else if (zone == domain::Zone::SPELL_TRAP)
		{
			// 盖放当回合不可发动
			if (const auto it = set_turns_.find(card);
				it == set_turns_.end() || it->second >= turn_.turn_id())
			{
				return false;
			}

			const auto placement = instance->placement();
			const auto* spell_trap = std::get_if<domain::SpellTrapPlacement>(&placement);

			if (spell_trap == nullptr || spell_trap->position != domain::Position::FACE_DOWN_DEFENSE)
			{
				return false;
			}

			// 盖放的卡:翻至表侧
			target_placement = domain::Placement{domain::SpellTrapPlacement{.column = spell_trap->column, .position = domain::Position::FACE_UP_ATTACK}};
		}
		else
		{
			return false;
		}

		const auto code = instance->origin_code();

		const auto* script = interpreter_.find_script(code);
		if (script == nullptr)
		{
			return false;
		}

		const dsl::ActivatedEffect* activated = nullptr;
		const dsl::Effect* activated_effect = nullptr;
		std::optional<execution::EventWindow> matched_window;
		auto has_card_activation = false;

		for (const auto& effect: script->effects)
		{
			const auto* candidate = std::get_if<dsl::ActivatedEffect>(&effect.body);
			if (candidate == nullptr)
			{
				continue;
			}

			const auto* card_activation = std::get_if<dsl::CardActivation>(&candidate->activation);
			if (card_activation == nullptr)
			{
				continue;
			}

			has_card_activation = true;

			// 事件型发动时点:当前时点须有匹配的事件
			if (card_activation->timing.has_value())
			{
				const auto matched = std::ranges::find_if(
					windows_,
					[&](const execution::EventWindow& window) noexcept -> bool
					{
						execution::Context timing_context{controller, card, window};

						return interpreter_.matches(
							dsl::Trigger{.spec = *card_activation->timing, .optional = false},
							timing_context
						);
					}
				);

				if (matched == windows_.end())
				{
					continue;
				}

				matched_window = *matched;
			}

			activated = candidate;
			activated_effect = &effect;
			break;
		}

		// 声明了卡发动效果但当前时点不匹配 → 不可作为纯静态卡发动
		if (activated == nullptr && has_card_activation)
		{
			return false;
		}

		// 召唤宣言窗口的第一环必须是召唤无效效果
		if (
			summon_.has_value() &&
			chain_.empty() &&
			(activated_effect == nullptr || !activated_effect->properties.contains(dsl::EffectProperty::SUMMON_NEGATE))
		)
		{
			return false;
		}

		if (!can_activate_in_damage_step(*instance, activated_effect))
		{
			// 伤害步骤内该卡/效果不属于可发动类别
			return false;
		}

		// 同一效果不能在同一连锁上重复发动;不同效果可以
		if (contains_duplicated_effect(card, activated_effect))
		{
			return false;
		}

		const auto speed = activated != nullptr
			                   ? activation_speed(*instance, activated->activation)
			                   : activation_speed(*instance, dsl::ActivationKind{dsl::CardActivation{}});

		if (!can_chain(speed))
		{
			return false;
		}

		// 纯静态卡(如场地魔法)可以没有卡自身发动效果
		if (activated_effect != nullptr && !can_use(*activated_effect, *instance))
		{
			return false;
		}

		execution::Context context{controller, card, {.chain = std::ref(chain_), .link_index = chain_.size()}};
		if (matched_window.has_value())
		{
			context.event = *matched_window;
		}

		if (activated != nullptr && !interpreter_.activate(*activated, context))
		{
			return false;
		}

		game_.place(*instance, *target_placement);

		const auto mutex_inserted = activated_effect != nullptr && mark_used(*activated_effect, *instance);

		execution::ChainLink link
		{
				controller,
				speed,
				card,
				activated,
				activated_effect,
				std::move(context),
		};
		link.set_mutex_inserted(mutex_inserted);

		chain_.push_back(std::move(link));

		// 已经进入连锁,本时点关闭;发动者的对方获得优先权
		windows_.clear();
		priority_.transfer_from(controller, turn_.turn_player());

		return true;
	}

	auto Duel::set_spell_trap(const state::CardInstanceId card, const domain::FieldColumn column) noexcept -> bool
	{
		if (!chain_.empty())
		{
			return false;
		}

		auto* instance = game_.find_instance_mutable(card);
		if (instance == nullptr)
		{
			return false;
		}

		const auto controller = instance->controller();
		if (controller != turn_.turn_player())
		{
			return false;
		}

		if (priority_.holder_player(turn_.turn_player()) != controller)
		{
			return false;
		}

		if (turn_.phase() != domain::Phase::MAIN_1 && turn_.phase() != domain::Phase::MAIN_2)
		{
			return false;
		}

		if (game_.zone_of(card) != domain::Zone::HAND)
		{
			return false;
		}

		if (!interpreter_.state_resolver().contains_card_types(*instance, {domain::CardType::SPELL, domain::CardType::TRAP}))
		{
			return false;
		}

		if (!game_.is_location_empty(controller, domain::Location{domain::SpellTrapLocation{.column = column}}))
		{
			return false;
		}

		game_.place(*instance, domain::SpellTrapPlacement{.column = column, .position = domain::Position::FACE_DOWN_DEFENSE});

		set_turns_.insert_or_assign(card, turn_.turn_id());

		// 盖放后旧的事件时点不再有效(盖放是否开时点待补)
		windows_.clear();

		return true;
	}

	auto Duel::declare_attack(const state::CardInstanceId attacker, const std::optional<state::CardInstanceId> target) noexcept -> bool
	{
		if (!chain_.empty())
		{
			return false;
		}

		if (turn_.phase() != domain::Phase::BATTLE)
		{
			return false;
		}

		if (attack_.has_value())
		{
			return false;
		}

		const auto* instance = game_.find_instance(attacker);
		if (instance == nullptr)
		{
			return false;
		}

		const auto controller = instance->controller();
		if (controller != turn_.turn_player())
		{
			return false;
		}

		// 只有当前优先权持有者可以宣言攻击
		if (priority_.holder_player(turn_.turn_player()) != controller)
		{
			return false;
		}

		if (const auto attacker_zone = game_.zone_of(attacker);
			attacker_zone != domain::Zone::MONSTER && attacker_zone != domain::Zone::EXTRA_MONSTER)
		{
			return false;
		}

		{
			const auto placement = instance->placement();
			const auto field_info = domain::field_info_of(placement);
			if (!field_info.has_value())
			{
				return false;
			}

			if (field_info->position != domain::Position::FACE_UP_ATTACK)
			{
				return false;
			}
		}

		if (std::ranges::contains(state_.attacked_monsters, attacker))
		{
			return false;
		}

		const auto opponent = domain::opponent_of(controller);

		if (target.has_value())
		{
			const auto* target_instance = game_.find_instance(*target);
			if (target_instance == nullptr)
			{
				return false;
			}

			if (const auto target_controller = target_instance->controller();
				target_controller != opponent)
			{
				return false;
			}

			if (const auto target_zone = game_.zone_of(*target);
				target_zone != domain::Zone::MONSTER && target_zone != domain::Zone::EXTRA_MONSTER)
			{
				return false;
			}
		}
		else if (!game_.is_zones_empty(opponent, {domain::Zone::MONSTER, domain::Zone::EXTRA_MONSTER}))
		{
			// 对方场上有怪兽时不能直接攻击
			return false;
		}

		// 攻击宣言只开时点;伤害步骤由 pass_priority 推进
		windows_.clear();

		state_.attacked_monsters.push_back(attacker);
		state_.attacked = true;
		state_.entered_battle_phase = true;

		attack_ =
		{
				.attacker = attacker,
				.target = target,
				.controller = controller,
				.opponent = opponent,
		};

		battle_.reset();
		// START → BATTLE
		battle_.advance_battle_step();

		const std::array windows
		{
				execution::EventWindow
				{
						.source = dsl::TriggerSource{domain::EventCode::ATTACK_DECLARED},
						.card = attacker,
						.player = std::nullopt,
						.amount = std::nullopt,
				},
		};
		open_timing(windows);

		return true;
	}

	auto Duel::replay_attack(const std::optional<state::CardInstanceId> target) noexcept -> bool
	{
		if (!attack_.has_value() || !attack_->replay_required)
		{
			return false;
		}

		if (turn_.phase() != domain::Phase::BATTLE || !chain_.empty() || summon_.has_value())
		{
			return false;
		}

		const auto* attacker = game_.find_instance(attack_->attacker);
		if (attacker == nullptr || !is_on_field(attack_->attacker))
		{
			return false;
		}

		if (const auto field_info = domain::field_info_of(attacker->placement());
			!field_info.has_value() || field_info->position != domain::Position::FACE_UP_ATTACK)
		{
			return false;
		}

		if (target.has_value())
		{
			const auto* target_instance = game_.find_instance(*target);
			if (target_instance == nullptr || target_instance->controller() != attack_->opponent)
			{
				return false;
			}

			if (const auto zone = game_.zone_of(*target); zone != domain::Zone::MONSTER && zone != domain::Zone::EXTRA_MONSTER)
			{
				return false;
			}
		}
		else if (!game_.is_zones_empty(attack_->opponent, {domain::Zone::MONSTER, domain::Zone::EXTRA_MONSTER}))
		{
			return false;
		}

		attack_->target = target;
		attack_->replay_required = false;
		attack_->target_flipped = false;
		attack_->target_position = domain::Position::FACE_UP_ATTACK;
		attack_->destroy_attacker.reset();
		attack_->destroy_target.reset();
		attack_->damage_to_opponent = 0;
		attack_->damage_to_controller = 0;

		battle_.reset();
		battle_.advance_battle_step();

		const std::array windows
		{
				execution::EventWindow
				{
						.source = dsl::TriggerSource{domain::EventCode::ATTACK_DECLARED},
						.card = attack_->attacker,
						.player = std::nullopt,
						.amount = std::nullopt,
				},
		};
		open_timing(windows);

		return true;
	}

	auto Duel::activate_effect(const state::CardInstanceId card, const std::size_t effect_index) noexcept -> bool
	{
		const auto* instance = game_.find_instance(card);
		if (instance == nullptr)
		{
			return false;
		}

		const auto controller = instance->controller();

		// 只有当前优先权持有者可以发动
		if (priority_.holder_player(turn_.turn_player()) != controller)
		{
			return false;
		}

		const auto code = instance->origin_code();

		const auto* script = interpreter_.find_script(code);
		if (script == nullptr)
		{
			// 指定实例不存在卡牌脚本
			return false;
		}

		if (effect_index >= script->effects.size())
		{
			// 指定实例的卡牌脚本没有这么多效果
			return false;
		}

		const auto& effect = script->effects[effect_index];

		const auto* activated = std::get_if<dsl::ActivatedEffect>(&effect.body);
		if (activated == nullptr)
		{
			// 指定实例的卡牌效果不是发动效果
			return false;
		}

		// 同一效果不能在同一连锁上重复发动;不同效果可以
		if (contains_duplicated_effect(card, &effect))
		{
			return false;
		}

		if (!can_use(effect, *instance))
		{
			// 指定实例的卡牌效果无法发动
			return false;
		}

		if (const auto* ignition = std::get_if<dsl::Ignition>(&activated->activation);
			ignition != nullptr)
		{
			// 启动效果:回合玩家、主要阶段、开放状态
			if (controller != turn_.turn_player())
			{
				return false;
			}
			if (turn_.phase() != domain::Phase::MAIN_1 && turn_.phase() != domain::Phase::MAIN_2)
			{
				return false;
			}
			if (!chain_.empty())
			{
				return false;
			}
		}
		else if (!std::holds_alternative<dsl::Quick>(activated->activation))
		{
			// 卡发动/诱发效果不可手动激活
			return false;
		}

		// 召唤宣言窗口的第一环必须是召唤无效效果
		if (summon_.has_value() && chain_.empty() && !effect.properties.contains(dsl::EffectProperty::SUMMON_NEGATE))
		{
			return false;
		}

		if (!can_activate_in_damage_step(*instance, &effect))
		{
			// 伤害步骤内该效果不属于可发动类别
			return false;
		}

		const auto speed = activation_speed(*instance, activated->activation);
		if (!can_chain(speed))
		{
			return false;
		}

		if (!activated->activation_zones.empty() && !activated->activation_zones.contains(game_.zone_of(card)))
		{
			return false;
		}

		execution::Context context{controller, card, {.chain = std::ref(chain_), .link_index = chain_.size()}};
		if (!interpreter_.activate(*activated, context))
		{
			return false;
		}

		const auto mutex_inserted = mark_used(effect, *instance);

		execution::ChainLink link
		{
				controller,
				speed,
				card,
				activated,
				&effect,
				std::move(context),
		};
		link.set_mutex_inserted(mutex_inserted);

		chain_.push_back(std::move(link));

		// 已经进入连锁,本时点关闭;发动者的对方获得优先权
		windows_.clear();
		priority_.transfer_from(controller, turn_.turn_player());

		return true;
	}

	auto Duel::game() noexcept -> state::Game&
	{
		return game_;
	}

	auto Duel::find_script(const domain::CardCode code) const noexcept -> const dsl::CardScript*
	{
		return interpreter_.find_script(code);
	}

	auto Duel::windows() const noexcept -> std::span<const execution::EventWindow>
	{
		return windows_;
	}

	auto Duel::trigger_event(const dsl::TriggerSource source, const std::optional<state::CardInstanceId> event_card) noexcept -> bool
	{
		if (!chain_.empty())
		{
			// 连锁中产生的事件应在连锁结算后处理
			return false;
		}

		const execution::EventWindow window{.source = source, .card = event_card, .player = std::nullopt, .amount = std::nullopt};
		return open_timing({&window, 1});
	}
}

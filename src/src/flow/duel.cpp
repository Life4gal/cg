#include <cg/flow/duel.hpp>

#include <algorithm>
#include <array>
#include <ranges>

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

		windows_.assign(windows.begin(), windows.end());

		execution::Context context{game_, turn_.turn_player()};
		context.card_state = &card_state_;

		const auto candidates = interpreter_.collect_triggered(scripts_, context, windows_, turn_.turn_player());

		auto added = false;
		for (const auto& entry: candidates)
		{
			const auto& instance = entry.instance.get();
			const auto& effect = entry.effect.get();

			// 前一条可能已占用同卡/同名次数
			if (!can_use(effect, instance))
			{
				continue;
			}

			auto link_context = context.make_child(entry.controller, instance.id());
			link_context.trigger_source = entry.source;
			link_context.event_card = entry.event_card;

			if (!interpreter_.activate(entry.activated.get(), link_context))
			{
				continue;
			}

			const auto mutex_inserted = mark_used(effect, instance);

			execution::ChainLink link
			{
					entry.controller,
					// 诱发效果固定为速度1
					domain::EffectSpeed::NORMAL,
					instance.id(),
					&entry.activated.get(),
					&effect,
					std::move(link_context),
					mutex_inserted
			};

			chain_.push_back(std::move(link));

			added = true;
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

	auto Duel::resolve_chain() noexcept -> void
	{
		if (chain_.empty())
		{
			return;
		}

		// 连锁开始结算后不能再插入发动,时点关闭
		windows_.clear();

		static_cast<void>(interpreter_.resolve(chain_));

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

		chain_.clear();
		priority_.reset();

		// 结算中产生的事件:合并为下一个时点
		process_pending_events();
	}

	auto Duel::settle_chain() noexcept -> void
	{
		while (!chain_.empty())
		{
			pass_priority();
		}
	}

	auto Duel::activation_speed(const state::CardInstance& card, const dsl::ActivationKind& activation) const noexcept -> domain::EffectSpeed
	{
		const auto type = [&] noexcept -> domain::CardType
		{
			const auto types = card_state_.card_types(card);

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

		return dsl::speed_of(type, card_state_.card_subtypes(card), activation);
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
		if (card_state_.is_card_type(card, domain::CardType::MONSTER))
		{
			return true;
		}

		return card_state_.is_card_subtype(
			card,
			{
					domain::CardSubtype::CONTINUOUS,
					domain::CardSubtype::FIELD,
					domain::CardSubtype::EQUIP,
			}
		);
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

		if (const auto monster = card_state_.is_card_type(*instance, domain::CardType::MONSTER);
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

	auto Duel::perform_normal_summon(const state::CardInstanceId card, const domain::MonsterPlacement placement) noexcept -> bool
	{
		// 如果已通招则召唤失败
		if (state_.normal_summoned)
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
		if (const auto level = card_state_.level(*instance);
			level >= 5)
		{
			return false;
		}

		// 放置到场上
		game_.place(*instance, {placement});

		// 已通常召唤
		state_.normal_summoned = true;

		// 召唤/盖放的成功时点
		if (placement.position == domain::Position::FACE_UP_ATTACK)
		{
			trigger_event(dsl::TriggerSource{domain::EventCode::NORMAL_SUMMON_SUCCESS}, card);
		}
		else
		{
			trigger_event(dsl::TriggerSource{domain::EventCode::MONSTER_SET}, card);
		}

		return true;
	}

	auto Duel::perform_tribute_summon(const state::CardInstanceId card, const domain::MonsterPlacement placement, const std::span<const state::CardInstanceId> tributes) noexcept -> bool
	{
		// 如果已通招则召唤失败
		if (state_.normal_summoned)
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

		const auto level = card_state_.level(*instance);
		// 等级 5~6 需要 1 个祭品,等级 7+ 需要 2 个
		if (const auto required = level >= 7 ? std::size_t{2} : (level >= 5 ? std::size_t{1} : std::size_t{0});
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

		// 解放祭品
		for (const auto tribute: tributes)
		{
			auto* tribute_instance = game_.find_instance_mutable(tribute);
			game_.move_to_graveyard(*tribute_instance, {domain::Reason::RELEASE});
		}

		// 放置到场上
		game_.place(*instance, {placement});

		// 已通常召唤
		state_.normal_summoned = true;

		// 祭品召唤/盖放的成功时点
		if (placement.position == domain::Position::FACE_UP_ATTACK)
		{
			trigger_event(dsl::TriggerSource{domain::EventCode::NORMAL_SUMMON_SUCCESS}, card);
		}
		else
		{
			trigger_event(dsl::TriggerSource{domain::EventCode::MONSTER_SET}, card);
		}

		return true;
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
			windows.push_back({.source = dsl::TriggerSource{code}, .card = card});
		}

		open_timing(windows);
	}

	auto Duel::usage_owner(const dsl::UsageKind kind, const state::CardInstance& card) const noexcept -> UsageKey::owner_type
	{
		if (kind == dsl::UsageKind::TURN_PER_CARD)
		{
			return card.id().raw();
		}

		return std::to_underlying(card_state_.effective_code(card));
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
				.owner = card_state_.effective_code(card),
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

	Duel::Duel() noexcept
		:
		// game_{},
		// turn_{},
		// priority_{},
		// battle_{},
		// state_{},
		loser_{std::nullopt},
		// types_{},
		vocabulary_{types_},
		// scripts_{},
		interpreter_{vocabulary_},
		card_state_{game_, scripts_, vocabulary_, interpreter_} {}

	auto Duel::game() noexcept -> state::Game&
	{
		return game_;
	}

	auto Duel::game() const noexcept -> const state::Game&
	{
		return game_;
	}

	// auto Duel::turn() noexcept -> Turn&
	// {
	// 	return turn_;
	// }

	auto Duel::turn() const noexcept -> const Turn&
	{
		return turn_;
	}

	// auto Duel::priority() noexcept -> Priority&
	// {
	// 	return priority_;
	// }

	auto Duel::priority() const noexcept -> const Priority&
	{
		return priority_;
	}

	// auto Duel::battle() noexcept -> Battle&
	// {
	// 	return battle_;
	// }

	auto Duel::battle() const noexcept -> const Battle&
	{
		return battle_;
	}

	auto Duel::chain() const noexcept -> const execution::Chain&
	{
		return chain_;
	}

	auto Duel::windows() const noexcept -> std::span<const execution::EventWindow>
	{
		return windows_;
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

	auto Duel::load_scripts(const std::filesystem::path& directory) noexcept -> bool
	{
		return scripts_.load_directory(directory, types_, vocabulary_);
	}

	// auto Duel::scripts() noexcept -> execution::ScriptRegistry&
	// {
	// 	return scripts_;
	// }

	auto Duel::scripts() const noexcept -> const execution::ScriptRegistry&
	{
		return scripts_;
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
		if (!chain_.empty() || !priority_.both_passed())
		{
			return false;
		}

		if (!turn_.advance_phase())
		{
			return false;
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
		return true;
	}

	auto Duel::end_turn() noexcept -> bool
	{
		if (turn_.phase() != domain::Phase::END || !chain_.empty())
		{
			return false;
		}

		turn_.end_turn();
		priority_.reset();
		battle_.reset();
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
		if (!card_state_.is_card_subtype(*instance, domain::CardSubtype::SYNCHRO))
		{
			return false;
		}

		// 必须有同调素材(必须有吗?)
		if (materials.empty())
		{
			return false;
		}

		const auto controller = instance->controller();

		// 等级+调整 满足要求
		const auto target_level = card_state_.level(*instance);
		std::underlying_type_t<domain::LevelRank> total_level{};
		auto has_tuner{false};
		auto has_non_tuner{false};

		for (const auto material: materials)
		{
			// 素材不能是自己且不能重复
			if (material == card || std::ranges::count(materials, material) != 1)
			{
				return false;
			}

			const auto* material_instance = game_.find_instance(material);

			if (material_instance == nullptr)
			{
				return false;
			}

			// 必须是自己控制的怪兽
			if (const auto material_controller = material_instance->controller();
				material_controller != controller)
			{
				return false;
			}

			// 必须是场上的怪兽
			if (const auto zone = game_.zone_of(material);
				zone != domain::Zone::MONSTER && zone != domain::Zone::EXTRA_MONSTER)
			{
				return false;
			}

			total_level += card_state_.level(*material_instance);
			if (card_state_.is_card_subtype(*material_instance, domain::CardSubtype::TUNER))
			{
				has_tuner = true;
			}
			else
			{
				has_non_tuner = true;
			}
		}

		if (!has_tuner || !has_non_tuner || total_level != target_level)
		{
			return false;
		}

		// 素材送入墓地
		for (const auto material: materials)
		{
			auto* material_instance = game_.find_instance_mutable(material);
			LIBASSERT_DEBUG_ASSERT(material_instance != nullptr);
			game_.move_to_graveyard(*material_instance, {domain::Reason::SYNCHRO});
		}

		// 先记录来的位置
		const auto from = instance->placement();

		// 放置到场上
		game_.place(*instance, domain::placement_of(placement));

		// 记录召唤信息
		game_.set_summon_record(
			*instance,
			{
					.player = controller,
					.method = domain::SummonMethod::SYNCHRO,
					.from = from,
					.materials = {materials.begin(), materials.end()},
			}
		);

		priority_.reset();

		// 特殊召唤的成功时点
		trigger_event(dsl::TriggerSource{domain::EventCode::SPECIAL_SUMMON_SUCCESS}, card);

		return true;
	}

	auto Duel::xyz_summon(const state::CardInstanceId card, const domain::AnyMonsterPlacement placement, const std::span<const state::CardInstanceId> materials) noexcept -> bool
	{
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
		if (!card_state_.is_card_subtype(*instance, domain::CardSubtype::XYZ))
		{
			return false;
		}

		// 必须有超量素材(必须有吗?)
		if (materials.size() < 2)
		{
			return false;
		}

		const auto controller = instance->controller();

		// 等级+数量 满足要求
		const auto target_level = card_state_.level(*instance);
		std::optional<std::underlying_type_t<domain::LevelRank>> material_level{};
		std::vector<const state::CardInstance*> material_instances{};

		for (const auto material: materials)
		{
			// 素材不能是自己且不能重复
			if (material == card || std::ranges::count(materials, material) != 1)
			{
				return false;
			}

			const auto* material_instance = game_.find_instance(material);

			if (material_instance == nullptr)
			{
				return false;
			}

			// 必须是自己控制的怪兽
			if (const auto material_controller = material_instance->controller();
				material_controller != controller)
			{
				return false;
			}

			// 必须是场上的怪兽
			if (const auto zone = game_.zone_of(material);
				zone != domain::Zone::MONSTER && zone != domain::Zone::EXTRA_MONSTER)
			{
				return false;
			}

			const auto level = card_state_.level(*material_instance);
			if (!material_level.has_value())
			{
				material_level = level;
			}
			else if (material_level != level)
			{
				return false;
			}

			material_instances.push_back(material_instance);
		}

		if (material_level != target_level)
		{
			return false;
		}

		// 先记录来的位置
		const auto from = instance->placement();

		// 放置到场上
		game_.place(*instance, domain::placement_of(placement));

		// 记录召唤信息
		game_.set_summon_record(
			*instance,
			{
					.player = controller,
					.method = domain::SummonMethod::XYZ,
					.from = from,
					.materials = std::vector<state::CardInstanceId>{materials.begin(), materials.end()},
			}
		);

		// 将素材叠放
		for (const auto material: materials)
		{
			// 素材送入未知区域(超量素材)
			auto* material_instance = game_.find_instance_mutable(material);
			LIBASSERT_DEBUG_ASSERT(material_instance != nullptr);
			game_.place(*material_instance, {domain::NotExistPlacement{}});

			// 添加超量素材
			game_.add_overlay(*instance, *material_instance);
		}

		priority_.reset();

		// 特殊召唤的成功时点
		trigger_event(dsl::TriggerSource{domain::EventCode::SPECIAL_SUMMON_SUCCESS}, card);

		return true;
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

		// 同一张卡在一个连锁上只能发动一次(含不同效果)
		if (std::ranges::contains(chain_, card, &execution::ChainLink::source))
		{
			return false;
		}

		const auto is_spell = card_state_.is_card_type(*instance, domain::CardType::SPELL);
		const auto is_trap = card_state_.is_card_type(*instance, domain::CardType::TRAP);
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

			if (card_state_.is_card_subtype(*instance, domain::CardSubtype::FIELD))
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

		const auto* script = scripts_.find(code);
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
						execution::Context timing_context{game_, controller, card};
						timing_context.card_state = &card_state_;
						timing_context.trigger_source = window.source;
						timing_context.event_card = window.card;

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

		execution::Context context{game_, controller, card};
		context.card_state = &card_state_;
		if (matched_window.has_value())
		{
			context.trigger_source = matched_window->source;
			context.event_card = matched_window->card;
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
				mutex_inserted
		};
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

		if (!card_state_.is_card_type(*instance, domain::CardType::SPELL) && !card_state_.is_card_type(*instance, domain::CardType::TRAP))
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

		// 攻击宣言开启新的窗口
		windows_.clear();

		state_.attacked_monsters.push_back(attacker);
		state_.attacked = true;
		state_.entered_battle_phase = true;

		const auto attacker_attack = card_state_.attack(*instance);

		// 战斗步骤:攻击宣言 → 伤害步骤
		battle_.reset();
		// START → BATTLE
		battle_.advance_battle_step();
		// BATTLE → DAMAGE
		battle_.advance_battle_step();

		// 伤害步骤开始:里侧守备 → 表侧守备
		auto target_position = domain::Position::FACE_UP_ATTACK;
		auto target_flipped{false};
		if (target.has_value())
		{
			auto* target_instance = game_.find_instance_mutable(*target);

			const auto placement = target_instance->placement();
			const auto field_info = domain::field_info_of(placement);
			if (!field_info.has_value())
			{
				return false;
			}

			target_position = field_info->position;
			if (target_position == domain::Position::FACE_DOWN_DEFENSE)
			{
				target_position = domain::Position::FACE_UP_DEFENSE;
				target_flipped = true;

				if (std::holds_alternative<domain::ExtraMonsterPlacement>(placement))
				{
					game_.place(*target_instance, domain::ExtraMonsterPlacement{.column = field_info->column, .position = target_position});
				}
				else
				{
					game_.place(*target_instance, domain::MonsterPlacement{.column = field_info->column, .position = target_position});
				}
			}
		}

		// START → BEFORE_CALCULATION
		battle_.advance_damage_step();
		// BEFORE_CALCULATION → PERFORM_CALCULATION
		battle_.advance_damage_step();

		// 伤害计算
		std::optional<state::CardInstanceId> destroy_attacker{};
		std::optional<state::CardInstanceId> destroy_target{};
		std::underlying_type_t<domain::AttackDefense> damage_to_opponent = 0;
		std::underlying_type_t<domain::AttackDefense> damage_to_controller = 0;

		if (!target.has_value())
		{
			damage_to_opponent = attacker_attack;
		}
		else
		{
			const auto* target_instance = game_.find_instance(*target);
			if (target_instance == nullptr)
			{
				return false;
			}

			if (target_position == domain::Position::FACE_UP_ATTACK)
			{
				if (const auto target_attack = card_state_.attack(*target_instance);
					attacker_attack > target_attack)
				{
					destroy_target = *target;
					damage_to_opponent = attacker_attack - target_attack;
				}
				else if (attacker_attack < target_attack)
				{
					destroy_attacker = attacker;
					damage_to_controller = target_attack - attacker_attack;
				}
				else
				{
					destroy_attacker = attacker;
					destroy_target = *target;
				}
			}
			else
			{
				if (const auto target_defense = card_state_.defense(*target_instance);
					attacker_attack > target_defense)
				{
					destroy_target = *target;
				}
				else if (attacker_attack < target_defense)
				{
					damage_to_controller = target_defense - attacker_attack;
				}
			}
		}

		// PERFORM_CALCULATION → AFTER_CALCULATION
		battle_.advance_damage_step();
		// AFTER_CALCULATION → END
		battle_.advance_damage_step();

		// 应用破坏与战斗伤害
		auto target_destroyed{false};
		auto attacker_destroyed{false};
		if (destroy_target.has_value())
		{
			if (auto* target_instance = game_.find_instance_mutable(*destroy_target);
				target_instance == nullptr || !card_state_.protections(*target_instance).contains(domain::Protection::BATTLE))
			{
				game_.move_to_graveyard(*target_instance, {domain::Reason::BATTLE, domain::Reason::BATTLE_DESTROY});
				target_destroyed = true;
			}
		}
		if (destroy_attacker.has_value())
		{
			if (auto* attacker_instance = game_.find_instance_mutable(*destroy_attacker);
				attacker_instance == nullptr || !card_state_.protections(*attacker_instance).contains(domain::Protection::BATTLE))
			{
				game_.move_to_graveyard(*attacker_instance, {domain::Reason::BATTLE, domain::Reason::BATTLE_DESTROY});
				attacker_destroyed = true;
			}
		}
		if (damage_to_opponent != 0)
		{
			game_.change_life_points(opponent, -damage_to_opponent);
		}
		if (damage_to_controller != 0)
		{
			game_.change_life_points(controller, -damage_to_controller);
		}

		// DAMAGE → END
		battle_.advance_battle_step();

		// 翻转效果 → 战斗破坏时点
		// TODO(战斗时点):目前战斗仍是同步结算,先把每组触发链结算完再进入下一个事件;
		// 完整窗口(攻击宣言/伤害计算/伤害步骤结束)待战斗流程重构时拆分。
		if (target_flipped && target.has_value())
		{
			trigger_event(dsl::TriggerSource{domain::EventCode::FLIP}, *target);
			settle_chain();
		}
		if (target_destroyed)
		{
			trigger_event(dsl::TriggerSource{domain::EventCode::BATTLE_DESTROYED}, *destroy_target);
			settle_chain();
		}
		if (attacker_destroyed)
		{
			trigger_event(dsl::TriggerSource{domain::EventCode::BATTLE_DESTROYED}, attacker);
			settle_chain();
		}

		// 战斗已同步处理完,不留下过期时点
		windows_.clear();

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

		// 同一张卡在一个连锁上只能发动一次(含不同效果)
		if (std::ranges::contains(chain_, card, &execution::ChainLink::source))
		{
			return false;
		}

		const auto code = instance->origin_code();

		const auto* script = scripts_.find(code);
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

		const auto speed = activation_speed(*instance, activated->activation);
		if (!can_chain(speed))
		{
			return false;
		}

		if (!activated->activation_zones.empty() && !activated->activation_zones.contains(game_.zone_of(card)))
		{
			return false;
		}

		execution::Context context{game_, controller, card};
		context.card_state = &card_state_;
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
				mutex_inserted
		};
		chain_.push_back(std::move(link));

		// 已经进入连锁,本时点关闭;发动者的对方获得优先权
		windows_.clear();
		priority_.transfer_from(controller, turn_.turn_player());

		return true;
	}

	auto Duel::trigger_event(const dsl::TriggerSource source, const std::optional<state::CardInstanceId> event_card) noexcept -> bool
	{
		if (!chain_.empty())
		{
			// 连锁中产生的事件应在连锁结算后处理
			return false;
		}

		const std::array windows{execution::EventWindow{.source = source, .card = event_card}};
		return open_timing(windows);
	}
}

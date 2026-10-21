#include <cg/flow/duel.hpp>

#include <algorithm>
#include <ranges>

#include <cg/execution/chain.hpp>

namespace cg::flow
{
	auto Duel::perform_tribute_summon(
		const state::CardInstanceId card,
		const domain::FieldColumn column,
		const std::span<const state::CardInstanceId> tributes,
		const domain::Position position
	) noexcept -> bool
	{
		if (turn_.phase() != domain::Phase::MAIN_1 && turn_.phase() != domain::Phase::MAIN_2)
		{
			return false;
		}
		if (state_.normal_summoned)
		{
			return false;
		}

		const auto* instance = game_.find_instance(card);
		if (instance == nullptr)
		{
			return false;
		}

		const auto controller = instance->controller();
		if (controller != turn_.turn_player())
		{
			return false;
		}

		if (game_.zone_of(card) != domain::Zone::HAND)
		{
			return false;
		}

		if (const auto monster = instance->is_monster_card();
			!monster)
		{
			return false;
		}

		const auto level = instance->level();

		// 等级 5~6 需要 1 个祭品,等级 7+ 需要 2 个
		if (const auto required = level >= 7 ? std::size_t{2} : (level >= 5 ? std::size_t{1} : std::size_t{0});
			tributes.size() != required)
		{
			return false;
		}

		for (const auto [index, tribute]: tributes | std::views::enumerate)
		{
			const auto* tribute_instance = game_.find_instance(tribute);

			if (tribute_instance == nullptr)
			{
				return false;
			}

			if (const auto tribute_controller = tribute_instance->controller();
				tribute_controller != controller)
			{
				return false;
			}

			if (const auto zone = game_.zone_of(tribute);
				zone != domain::Zone::MONSTER && zone != domain::Zone::EXTRA_MONSTER)
			{
				return false;
			}

			if (std::ranges::contains(tributes.begin() + index + 1, tributes.end(), tribute))
			{
				return false;
			}
		}

		if (!game_.is_location_empty(controller, domain::MonsterLocation{.column = column}))
		{
			return false;
		}

		// 解放祭品
		for (const auto tribute: tributes)
		{
			if (!game_.move_to_graveyard(tribute, {domain::Reason::RELEASE}))
			{
				return false;
			}
		}

		if (!game_.place(card, domain::MonsterPlacement{.column = column, .position = position}))
		{
			return false;
		}

		state_.normal_summoned = true;
		return true;
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
		stats_{game_, scripts_, interpreter_} {}

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
		if (turn_.phase() != domain::Phase::DRAW || state_.drew_in_draw_phase)
		{
			return false;
		}

		const auto player = turn_.turn_player();
		const auto deck = game_.collect_deck_zones_card(player);
		if (deck.empty())
		{
			// 必须抽卡时卡组为空 → 判负
			loser_ = player;
			return false;
		}

		if (!game_.move_to_hand(deck.front(), {domain::Reason::DRAW}, domain::Position::FACE_DOWN_DEFENSE))
		{
			return false;
		}

		state_.drew_in_draw_phase = true;
		return true;
	}

	auto Duel::change_phase() noexcept -> bool
	{
		if (!priority_.both_passed())
		{
			return false;
		}

		if (!turn_.advance_phase())
		{
			return false;
		}

		priority_.reset();
		return true;
	}

	auto Duel::pass_priority() noexcept -> bool
	{
		priority_.pass();
		return priority_.both_passed();
	}

	auto Duel::end_turn() noexcept -> bool
	{
		if (turn_.phase() != domain::Phase::END)
		{
			return false;
		}

		turn_.end_turn();
		priority_.reset();
		battle_.reset();
		state_ = TurnState{};

		// 清除本回合的使用记录(DUEL范围保留)
		std::erase_if(
			usage_,
			[](const std::pair<const UsageKey&, const UsageKey::size_type>& p) noexcept -> bool
			{
				return p.first.scope == dsl::UsageScope::TURN;
			}
		);

		return true;
	}

	auto Duel::normal_summon(const state::CardInstanceId card, const domain::FieldColumn column) noexcept -> bool
	{
		if (turn_.phase() != domain::Phase::MAIN_1 && turn_.phase() != domain::Phase::MAIN_2)
		{
			return false;
		}
		if (state_.normal_summoned)
		{
			return false;
		}

		const auto* instance = game_.find_instance(card);
		if (instance == nullptr)
		{
			return false;
		}

		const auto controller = instance->controller();
		if (controller != turn_.turn_player())
		{
			return false;
		}

		if (game_.zone_of(card) != domain::Zone::HAND)
		{
			return false;
		}

		if (const auto monster = instance->is_monster_card();
			!monster)
		{
			return false;
		}

		// 等级 5 以上需要祭品(使用 tribute_summon / tribute_set)
		if (const auto level = instance->level();
			level >= 5)
		{
			return false;
		}

		if (!game_.is_location_empty(controller, domain::MonsterLocation{.column = column}))
		{
			return false;
		}

		if (!game_.place(card, domain::MonsterPlacement{.column = column, .position = domain::Position::FACE_UP_ATTACK}))
		{
			return false;
		}

		state_.normal_summoned = true;
		return true;
	}

	auto Duel::set_monster(const state::CardInstanceId card, const domain::FieldColumn column) noexcept -> bool
	{
		if (turn_.phase() != domain::Phase::MAIN_1 && turn_.phase() != domain::Phase::MAIN_2)
		{
			return false;
		}
		if (state_.normal_summoned)
		{
			return false;
		}

		const auto* instance = game_.find_instance(card);
		if (instance == nullptr)
		{
			return false;
		}

		const auto controller = instance->controller();
		if (controller != turn_.turn_player())
		{
			return false;
		}

		if (game_.zone_of(card) != domain::Zone::HAND)
		{
			return false;
		}

		if (const auto monster = instance->is_monster_card();
			!monster)
		{
			return false;
		}

		// 等级 5 以上需要祭品(使用 tribute_summon / tribute_set)
		if (const auto level = instance->level();
			level >= 5)
		{
			return false;
		}

		if (!game_.is_location_empty(controller, domain::MonsterLocation{.column = column}))
		{
			return false;
		}

		if (!game_.place(card, domain::MonsterPlacement{.column = column, .position = domain::Position::FACE_DOWN_DEFENSE}))
		{
			return false;
		}

		state_.normal_summoned = true;
		return true;
	}

	auto Duel::tribute_summon(const state::CardInstanceId card, const domain::FieldColumn column, const std::span<const state::CardInstanceId> tributes) noexcept -> bool
	{
		return perform_tribute_summon(card, column, tributes, domain::Position::FACE_UP_ATTACK);
	}

	auto Duel::tribute_set(const state::CardInstanceId card, const domain::FieldColumn column, const std::span<const state::CardInstanceId> tributes) noexcept -> bool
	{
		return perform_tribute_summon(card, column, tributes, domain::Position::FACE_DOWN_DEFENSE);
	}

	auto Duel::set_spell_trap(const state::CardInstanceId card, const domain::FieldColumn column) noexcept -> bool
	{
		const auto* instance = game_.find_instance(card);
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

		if (!instance->is_spell_card() && !instance->is_trap_card())
		{
			return false;
		}

		if (!game_.is_location_empty(controller, domain::SpellTrapLocation{.column = column}))
		{
			return false;
		}

		if (!game_.place(card, domain::SpellTrapPlacement{.column = column, .position = domain::Position::FACE_DOWN_DEFENSE}))
		{
			return false;
		}

		set_turns_.insert_or_assign(card, turn_.turn_id());

		return true;
	}

	auto Duel::activate_card(const state::CardInstanceId card) noexcept -> bool
	{
		const auto* instance = game_.find_instance(card);
		if (instance == nullptr)
		{
			return false;
		}

		const auto controller = instance->controller();

		if (priority_.holder_player(turn_.turn_player()) != controller)
		{
			return false;
		}

		const auto is_spell = instance->is_spell_card();
		const auto is_trap = instance->is_trap_card();
		if (!is_spell && !is_trap)
		{
			return false;
		}

		const auto zone = game_.zone_of(card);

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

			if (const auto* spell_trap = std::get_if<domain::SpellTrapPlacement>(&placement);
				spell_trap == nullptr || spell_trap->position != domain::Position::FACE_DOWN_DEFENSE)
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		const auto code = instance->code();

		const auto* script = scripts_.find(code);
		if (script == nullptr)
		{
			return false;
		}

		const dsl::ActivatedEffect* activated = nullptr;
		std::size_t effect_index = 0;
		for (const auto [index, effect]: script->effects | std::views::enumerate)
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

			// TODO: 事件时点(CardActivation::timing)尚未接入
			if (card_activation->timing.has_value())
			{
				return false;
			}

			activated = candidate;
			effect_index = index;
			break;
		}

		// 纯静态卡(如场地魔法)可以没有卡自身发动效果
		if (activated != nullptr && !can_use(*script, card, static_cast<UsageKey::index_type>(effect_index)))
		{
			return false;
		}

		execution::Context context{game_, controller, card};
		if (activated != nullptr && !interpreter_.activate(*activated, context))
		{
			return false;
		}

		// 放置/翻面
		if (zone == domain::Zone::HAND)
		{
			if (const auto field = instance->is_card_subtype(domain::CardSubtype::FIELD);
				field)
			{
				if (!game_.is_field_spell_zones_empty(controller))
				{
					return false;
				}

				if (!game_.place(card, domain::FieldSpellPlacement{.position = domain::Position::FACE_UP_ATTACK}))
				{
					return false;
				}
			}
			else
			{
				std::optional<domain::FieldColumn> column;
				for (const auto candidate: domain::field_columns)
				{
					if (game_.is_location_empty(controller, domain::SpellTrapLocation{.column = candidate}))
					{
						column = candidate;
						break;
					}
				}

				if (!column.has_value())
				{
					return false;
				}

				if (!game_.place(card, domain::SpellTrapPlacement{.column = *column, .position = domain::Position::FACE_UP_ATTACK}))
				{
					return false;
				}
			}
		}
		else
		{
			// 盖放的卡:翻至表侧
			const auto placement = instance->placement();
			const auto* spell_trap = std::get_if<domain::SpellTrapPlacement>(&placement);

			if (spell_trap == nullptr)
			{
				return false;
			}

			if (!game_.place(card, domain::SpellTrapPlacement{.column = spell_trap->column, .position = domain::Position::FACE_UP_ATTACK}))
			{
				return false;
			}
		}

		if (activated != nullptr)
		{
			if (!interpreter_.resolve(*activated, context))
			{
				return false;
			}

			mark_used(*script, card, static_cast<UsageKey::index_type>(effect_index));
		}

		// 留场:魔法(永续/场地/装备) / 陷阱(永续);其余送墓
		if (const auto stays = instance->is_card_subtype({domain::CardSubtype::CONTINUOUS, domain::CardSubtype::FIELD, domain::CardSubtype::EQUIP});
			!stays)
		{
			game_.move_to_graveyard(card, {domain::Reason::RULE});
		}

		priority_.reset();

		return true;
	}

	auto Duel::declare_attack(const state::CardInstanceId attacker, const std::optional<state::CardInstanceId> target) noexcept -> bool
	{
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
		else if (!game_.is_monster_zones_empty(opponent) || !game_.is_extra_monster_zones_empty(opponent))
		{
			// 对方场上有怪兽时不能直接攻击
			return false;
		}

		state_.attacked_monsters.push_back(attacker);
		state_.attacked = true;
		state_.entered_battle_phase = true;

		const auto attacker_attack = stats_.attack(*instance);

		// 战斗步骤:攻击宣言 → 伤害步骤
		battle_.reset();
		// START → BATTLE
		battle_.advance_battle_step();
		// BATTLE → DAMAGE
		battle_.advance_battle_step();

		// 伤害步骤开始:里侧守备 → 表侧守备
		auto target_position = domain::Position::FACE_UP_ATTACK;
		if (target.has_value())
		{
			const auto placement = game_.placement(*target);
			if (!placement.has_value())
			{
				return false;
			}

			const auto field_info = domain::field_info_of(*placement);
			if (!field_info.has_value())
			{
				return false;
			}

			target_position = field_info->position;
			if (target_position == domain::Position::FACE_DOWN_DEFENSE)
			{
				target_position = domain::Position::FACE_UP_DEFENSE;

				if (std::holds_alternative<domain::ExtraMonsterPlacement>(*placement))
				{
					if (!game_.place(*target, domain::ExtraMonsterPlacement{.column = field_info->column, .position = target_position}))
					{
						return false;
					}
				}
				else
				{
					if (!game_.place(*target, domain::MonsterPlacement{.column = field_info->column, .position = target_position}))
					{
						return false;
					}
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
				if (const auto target_attack = stats_.attack(*target_instance);
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
				if (const auto target_defense = stats_.defense(*target_instance);
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
		if (destroy_target.has_value())
		{
			game_.move_to_graveyard(*destroy_target, {domain::Reason::BATTLE, domain::Reason::BATTLE_DESTROY});
		}
		if (destroy_attacker.has_value())
		{
			game_.move_to_graveyard(*destroy_attacker, {domain::Reason::BATTLE, domain::Reason::BATTLE_DESTROY});
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

		return true;
	}

	auto Duel::usage_owner(const dsl::UsageBasis basis, const state::CardInstanceId card) const noexcept -> UsageKey::owner_type
	{
		if (basis == dsl::UsageBasis::THIS_CARD)
		{
			return card.raw();
		}

		const auto* instance = game_.find_instance(card);
		if (instance == nullptr)
		{
			return 0;
		}

		const auto code = instance->code();
		return std::to_underlying(code);
	}

	auto Duel::can_use(const dsl::CardScript& script, const state::CardInstanceId card, const UsageKey::index_type effect_index) const noexcept -> bool
	{
		for (const auto [index, restriction]: script.usage_restrictions | std::views::enumerate)
		{
			if (!std::ranges::contains(restriction.refs, effect_index))
			{
				continue;
			}

			const UsageKey key
			{
					.owner = usage_owner(restriction.basis, card),
					.scope = restriction.scope,
					.basis = restriction.basis,
					.index = static_cast<UsageKey::size_type>(index),
			};

			const auto it = usage_.find(key);
			if (const auto used = it == usage_.end() ? 0u : it->second;
				used >= restriction.count)
			{
				return false;
			}
		}

		return true;
	}

	auto Duel::mark_used(const dsl::CardScript& script, const state::CardInstanceId card, const UsageKey::index_type effect_index) noexcept -> void
	{
		for (const auto [index, restriction]: script.usage_restrictions | std::views::enumerate)
		{
			if (!std::ranges::contains(restriction.refs, effect_index))
			{
				continue;
			}

			const UsageKey key
			{
					.owner = usage_owner(restriction.basis, card),
					.scope = restriction.scope,
					.basis = restriction.basis,
					.index = static_cast<UsageKey::size_type>(index),
			};
			auto& count = usage_[key];

			count += 1;
		}
	}

	auto Duel::activate_effect(const state::CardInstanceId card, const UsageKey::index_type effect_index) noexcept -> bool
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

		const auto code = instance->code();

		const auto* script = scripts_.find(code);
		if (script == nullptr || effect_index >= script->effects.size())
		{
			return false;
		}

		const auto* activated = std::get_if<dsl::ActivatedEffect>(&script->effects[effect_index].body);
		if (activated == nullptr)
		{
			return false;
		}

		if (!can_use(*script, card, effect_index))
		{
			return false;
		}

		if (const auto* ignition = std::get_if<dsl::Ignition>(&activated->activation);
			ignition != nullptr)
		{
			// 启动效果:回合玩家、主要阶段
			if (controller != turn_.turn_player())
			{
				return false;
			}
			if (turn_.phase() != domain::Phase::MAIN_1 && turn_.phase() != domain::Phase::MAIN_2)
			{
				return false;
			}
		}
		else if (!std::holds_alternative<dsl::Quick>(activated->activation))
		{
			// 卡发动/诱发效果不可手动激活
			return false;
		}

		if (!activated->activation_zones.empty() && !activated->activation_zones.contains(game_.zone_of(card)))
		{
			return false;
		}

		if (execution::Context context{game_, controller, card};
			!interpreter_.execute(*activated, context))
		{
			return false;
		}

		mark_used(*script, card, effect_index);

		// 连锁结算后回到回合玩家的开放状态
		priority_.reset();

		return true;
	}

	auto Duel::trigger_event(const dsl::TriggerSource source, const std::optional<state::CardInstanceId> event_card) noexcept -> bool
	{
		execution::Context context{game_, turn_.turn_player()};
		context.trigger_source = source;
		context.event_card = event_card;

		const auto triggered = interpreter_.collect_triggered(scripts_, context, turn_.turn_player());
		if (triggered.empty())
		{
			return false;
		}

		auto chain = interpreter_.build_chain(triggered, context);
		if (chain.empty())
		{
			return false;
		}

		return interpreter_.resolve(chain);
	}
}

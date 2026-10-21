#include <cg/flow/duel.hpp>

#include <algorithm>
#include <ranges>

#include <cg/execution/chain.hpp>

namespace cg::flow
{
	namespace
	{
		class MonsterPosition
		{
		public:
			domain::FieldColumn column;
			domain::Position position;
			bool extra;
		};

		[[nodiscard]] auto monster_position(const state::Game& game, const state::CardInstanceId card) noexcept -> std::optional<MonsterPosition>
		{
			const auto placement = game.placement(card);
			if (!placement.has_value())
			{
				return std::nullopt;
			}

			if (const auto* p = std::get_if<domain::MonsterPlacement>(&*placement))
			{
				return MonsterPosition{.column = p->column, .position = p->position, .extra = false};
			}

			if (const auto* p = std::get_if<domain::ExtraMonsterPlacement>(&*placement))
			{
				return MonsterPosition{.column = p->column, .position = p->position, .extra = true};
			}

			return std::nullopt;
		}
	}

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

		const auto controller = game_.controller(card);
		if (!controller.has_value() || *controller != turn_.turn_player())
		{
			return false;
		}

		if (game_.zone_of(card) != domain::Zone::HAND)
		{
			return false;
		}

		if (const auto monster = game_.is_monster_card(card);
			!monster.has_value() || !*monster)
		{
			return false;
		}

		const auto level = game_.level(card);
		if (!level.has_value())
		{
			return false;
		}

		// 等级 5~6 需要 1 个祭品,等级 7+ 需要 2 个
		if (const auto required = *level >= 7 ? std::size_t{2} : (*level >= 5 ? std::size_t{1} : std::size_t{0});
			tributes.size() != required)
		{
			return false;
		}

		for (std::size_t i = 0; i < tributes.size(); ++i)
		{
			if (const auto tribute_controller = game_.controller(tributes[i]);
				!tribute_controller.has_value() || *tribute_controller != *controller)
			{
				return false;
			}

			if (const auto zone = game_.zone_of(tributes[i]);
				zone != domain::Zone::MONSTER && zone != domain::Zone::EXTRA_MONSTER)
			{
				return false;
			}

			for (std::size_t j = i + 1; j < tributes.size(); ++j)
			{
				if (tributes[i] == tributes[j])
				{
					return false;
				}
			}
		}

		if (!game_.is_location_empty(*controller, domain::MonsterLocation{.column = column}))
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

		if (!game_.place(card, domain::Placement{domain::MonsterPlacement{.column = column, .position = position}}))
		{
			return false;
		}

		state_.normal_summoned = true;
		return true;
	}

	auto Duel::attack_value(const state::CardInstanceId card) noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		const auto base = game_.attack(card).value_or(0);

		execution::Context context{game_, game_.controller(card).value_or(turn_.turn_player())};
		const auto bonus = interpreter_.static_numeric_bonus(scripts_, card, domain::NumericProperty::ATTACK, context);

		return static_cast<std::underlying_type_t<domain::AttackDefense>>(base + bonus);
	}

	auto Duel::defense_value(const state::CardInstanceId card) noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		const auto base = game_.defense(card).value_or(0);

		execution::Context context{game_, game_.controller(card).value_or(turn_.turn_player())};
		const auto bonus = interpreter_.static_numeric_bonus(scripts_, card, domain::NumericProperty::DEFENSE, context);

		return static_cast<std::underlying_type_t<domain::AttackDefense>>(base + bonus);
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
		interpreter_{vocabulary_} {}

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

		const auto controller = game_.controller(card);
		if (!controller.has_value() || *controller != turn_.turn_player())
		{
			return false;
		}

		if (game_.zone_of(card) != domain::Zone::HAND)
		{
			return false;
		}

		if (const auto monster = game_.is_monster_card(card);
			!monster.has_value() || !*monster)
		{
			return false;
		}

		// 等级 5 以上需要祭品(使用 tribute_summon / tribute_set)
		if (const auto level = game_.level(card);
			!level.has_value() || *level >= 5)
		{
			return false;
		}

		if (!game_.is_location_empty(*controller, domain::MonsterLocation{.column = column}))
		{
			return false;
		}

		if (!game_.place(card, domain::Placement{domain::MonsterPlacement{.column = column, .position = domain::Position::FACE_UP_ATTACK}}))
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

		const auto controller = game_.controller(card);
		if (!controller.has_value() || *controller != turn_.turn_player())
		{
			return false;
		}

		if (game_.zone_of(card) != domain::Zone::HAND)
		{
			return false;
		}

		if (const auto monster = game_.is_monster_card(card);
			!monster.has_value() || !*monster)
		{
			return false;
		}

		// 等级 5 以上需要祭品(使用 tribute_summon / tribute_set)
		if (const auto level = game_.level(card);
			!level.has_value() || *level >= 5)
		{
			return false;
		}

		if (!game_.is_location_empty(*controller, domain::MonsterLocation{.column = column}))
		{
			return false;
		}

		if (!game_.place(card, domain::Placement{domain::MonsterPlacement{.column = column, .position = domain::Position::FACE_DOWN_DEFENSE}}))
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

	auto Duel::declare_attack(const state::CardInstanceId attacker, const std::optional<state::CardInstanceId> target) noexcept -> bool
	{
		if (turn_.phase() != domain::Phase::BATTLE)
		{
			return false;
		}

		const auto controller = game_.controller(attacker);
		if (!controller.has_value() || *controller != turn_.turn_player())
		{
			return false;
		}

		if (const auto attacker_zone = game_.zone_of(attacker);
			attacker_zone != domain::Zone::MONSTER && attacker_zone != domain::Zone::EXTRA_MONSTER)
		{
			return false;
		}

		if (const auto attacker_position = monster_position(game_, attacker);
			!attacker_position.has_value() || attacker_position->position != domain::Position::FACE_UP_ATTACK)
		{
			return false;
		}

		if (std::ranges::contains(state_.attacked_monsters, attacker))
		{
			return false;
		}

		const auto opponent = domain::opponent_of(*controller);

		if (target.has_value())
		{
			if (const auto target_controller = game_.controller(*target);
				!target_controller.has_value() || *target_controller != opponent)
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

		const auto attacker_attack = attack_value(attacker);

		// 战斗步骤:攻击宣言 → 伤害步骤
		battle_.reset();
		// START → BATTLE
		battle_.advance_battle_step();
		// BATTLE → DAMAGE
		battle_.advance_battle_step();

		// 伤害步骤开始:里侧守备 → 表侧守备
		std::optional<MonsterPosition> target_position{};
		if (target.has_value())
		{
			target_position = monster_position(game_, *target);
			if (!target_position.has_value())
			{
				return false;
			}

			if (target_position->position == domain::Position::FACE_DOWN_DEFENSE)
			{
				const auto placement = target_position->extra
					                       ? domain::Placement{domain::ExtraMonsterPlacement{.column = target_position->column, .position = domain::Position::FACE_UP_DEFENSE}}
					                       : domain::Placement{domain::MonsterPlacement{.column = target_position->column, .position = domain::Position::FACE_UP_DEFENSE}};

				if (!game_.place(*target, placement))
				{
					return false;
				}

				target_position->position = domain::Position::FACE_UP_DEFENSE;
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
		else if (target_position->position == domain::Position::FACE_UP_ATTACK)
		{
			if (const auto target_attack = attack_value(*target);
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
			if (const auto target_defense = defense_value(*target);
				attacker_attack > target_defense)
			{
				destroy_target = *target;
			}
			else if (attacker_attack < target_defense)
			{
				damage_to_controller = target_defense - attacker_attack;
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
			game_.change_life_points(*controller, -damage_to_controller);
		}

		// DAMAGE → END
		battle_.advance_battle_step();

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

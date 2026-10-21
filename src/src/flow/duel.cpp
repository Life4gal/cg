#include <cg/flow/duel.hpp>

#include <algorithm>

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

	Duel::Duel() noexcept
	// : game_{},
	// turn_{},
	// priority_{},
	// battle_{} {}
	= default;

	auto Duel::game() noexcept -> state::Game&
	{
		return game_;
	}

	auto Duel::game() const noexcept -> const state::Game&
	{
		return game_;
	}

	auto Duel::turn() noexcept -> Turn&
	{
		return turn_;
	}

	auto Duel::turn() const noexcept -> const Turn&
	{
		return turn_;
	}

	auto Duel::priority() noexcept -> Priority&
	{
		return priority_;
	}

	auto Duel::priority() const noexcept -> const Priority&
	{
		return priority_;
	}

	auto Duel::battle() noexcept -> Battle&
	{
		return battle_;
	}

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

		const auto attack_value = game_.attack(attacker).value_or(0);

		if (!target.has_value())
		{
			game_.change_life_points(opponent, -attack_value);
			return true;
		}

		auto target_position = monster_position(game_, *target);
		if (!target_position.has_value())
		{
			return false;
		}

		// 里侧守备 → 表侧守备(伤害计算前翻面)
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

		const auto destroy = [&](const state::CardInstanceId card) noexcept -> bool
		{
			return game_.move_to_graveyard(card, {domain::Reason::BATTLE, domain::Reason::BATTLE_DESTROY});
		};

		// 攻击表示
		if (target_position->position == domain::Position::FACE_UP_ATTACK)
		{
			if (const auto target_attack = game_.attack(*target).value_or(0);
				attack_value > target_attack)
			{
				destroy(*target);
				game_.change_life_points(opponent, -(attack_value - target_attack));
			}
			else if (attack_value < target_attack)
			{
				destroy(attacker);
				game_.change_life_points(*controller, -(target_attack - attack_value));
			}
			else
			{
				destroy(attacker);
				destroy(*target);
			}

			return true;
		}

		// 守备表示
		if (const auto target_defense = game_.defense(*target).value_or(0);
			attack_value > target_defense)
		{
			destroy(*target);
		}
		else if (attack_value < target_defense)
		{
			game_.change_life_points(*controller, -(target_defense - attack_value));
		}

		return true;
	}

	auto Duel::change_phase() noexcept -> bool
	{
		return turn_.advance_phase();
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
}

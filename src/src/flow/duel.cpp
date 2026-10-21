#include <cg/flow/duel.hpp>

namespace cg::flow
{
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

	auto Duel::draw_for_turn() noexcept -> bool
	{
		if (turn_.phase() != domain::Phase::DRAW || turn_.state().drew_in_draw_phase)
		{
			return false;
		}

		const auto player = turn_.turn_player();
		const auto deck = game_.collect_deck_zones_card(player);
		if (deck.empty())
		{
			// 抽卡失败(卡组耗尽)
			return false;
		}

		if (!game_.move_to_hand(deck.front(), {domain::Reason::DRAW}, domain::Position::FACE_DOWN_DEFENSE))
		{
			return false;
		}

		turn_.state().drew_in_draw_phase = true;
		return true;
	}

	auto Duel::normal_summon(const state::CardInstanceId card, const domain::FieldColumn column) noexcept -> bool
	{
		if (turn_.phase() != domain::Phase::MAIN_1 && turn_.phase() != domain::Phase::MAIN_2)
		{
			return false;
		}
		if (turn_.state().normal_summoned)
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

		if (!game_.is_location_empty(*controller, domain::MonsterLocation{.column = column}))
		{
			return false;
		}

		if (!game_.place(card, domain::Placement{domain::MonsterPlacement{.column = column, .position = domain::Position::FACE_UP_ATTACK}}))
		{
			return false;
		}

		turn_.state().normal_summoned = true;
		return true;
	}

	auto Duel::set_monster(const state::CardInstanceId card, const domain::FieldColumn column) noexcept -> bool
	{
		if (turn_.phase() != domain::Phase::MAIN_1 && turn_.phase() != domain::Phase::MAIN_2)
		{
			return false;
		}
		if (turn_.state().normal_summoned)
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

		if (!game_.is_location_empty(*controller, domain::MonsterLocation{.column = column}))
		{
			return false;
		}

		if (!game_.place(card, domain::Placement{domain::MonsterPlacement{.column = column, .position = domain::Position::FACE_DOWN_DEFENSE}}))
		{
			return false;
		}

		turn_.state().normal_summoned = true;
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

		return true;
	}
}

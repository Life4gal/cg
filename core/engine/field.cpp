#include <core/engine/field.hpp>

#include <core/engine/card.hpp>
#include <core/engine/duel.hpp>

namespace cg::engine
{
	auto Field::duel() const noexcept -> Duel&
	{
		return duel_.get();
	}

	Field::PlaygroundHandler::PlaygroundHandler(Field& field) noexcept
		: field_{field} {}

	auto Field::PlaygroundHandler::playground() noexcept -> Playground&
	{
		return field_.get().playground_;
	}

	auto Field::PlaygroundHandler::playground() const noexcept -> const Playground&
	{
		return field_.get().playground_;
	}

	auto Field::PlaygroundHandler::set_life_point(const domain::Player player, const domain::life_point_type life_point) noexcept -> void
	{
		playground().set_life_point(player, life_point);
	}

	auto Field::PlaygroundHandler::set_start_hand(const domain::Player player, const size_type start_hand) noexcept -> void
	{
		playground().set_start_hand(player, start_hand);
	}

	auto Field::PlaygroundHandler::set_draw_count(const domain::Player player, const size_type draw_count) noexcept -> void
	{
		playground().set_draw_count(player, draw_count);
	}

	auto Field::PlaygroundHandler::life_point(const domain::Player player) const noexcept -> domain::life_point_type
	{
		return playground().life_point(player);
	}

	auto Field::PlaygroundHandler::start_hand(const domain::Player player) const noexcept -> size_type
	{
		return playground().start_hand(player);
	}

	auto Field::PlaygroundHandler::draw_count(const domain::Player player) const noexcept -> size_type
	{
		return playground().draw_count(player);
	}

	auto Field::PlaygroundHandler::update_life_point(const domain::Player player, const domain::life_point_type delta) noexcept -> void
	{
		playground().update_life_point(player, delta);
	}

	auto Field::PlaygroundHandler::start_draw(const domain::Player player) noexcept -> void
	{
		playground().start_draw(player);
	}

	auto Field::PlaygroundHandler::draw(const domain::Player player, const size_type count) noexcept -> void
	{
		playground().draw(player, count);
	}

	auto Field::PlaygroundHandler::shuffle_deck(const domain::Player player) noexcept -> void
	{
		playground().shuffle_deck(player, field_.get().duel().random());

		// todo: 广播洗牌
	}

	auto Field::PlaygroundHandler::reverse_deck(const domain::Player player) noexcept -> void
	{
		playground().reverse_deck(player);

		// todo: 广播反转
	}

	auto Field::PlaygroundHandler::shuffle_extra_deck(const domain::Player player) noexcept -> void
	{
		playground().shuffle_extra_deck(player, field_.get().duel().random());

		// todo: 广播洗牌
	}

	auto Field::PlaygroundHandler::shuffle_hand(const domain::Player player) noexcept -> void
	{
		playground().shuffle_hand(player, field_.get().duel().random());

		// todo: 广播洗牌
	}

	auto Field::PlaygroundHandler::occupied(const domain::Player player, const domain::Zone zone) const noexcept -> bool
	{
		return playground().occupied(player, zone);
	}

	auto Field::PlaygroundHandler::select(const domain::Player player, const domain::Zone zone) const noexcept -> CardOptional
	{
		return playground().select(player, zone);
	}

	auto Field::PlaygroundHandler::count(const domain::Player player, const domain::Zone zone) const noexcept -> size_type
	{
		return playground().count(player, zone);
	}

	auto Field::PlaygroundHandler::free_area(const domain::Player player, const domain::Zone zone) const noexcept -> std::vector<domain::Zone::size_type>
	{
		return playground().free_area(player, zone);
	}

	auto Field::PlaygroundHandler::remove_card(const CardReference card) noexcept -> void
	{
		if (playground().remove_card(card))
		{
			//
		}

		//
	}

	auto Field::PlaygroundHandler::move_card(const CardReference card, const domain::Player player, const domain::Zone zone) noexcept -> void
	{
		if (playground().move_card(card, player, zone))
		{
			//
		}

		//
	}

	Field::TossInfoHandler::TossInfoHandler(Field& field) noexcept
		: field_{field} {}

	auto Field::TossInfoHandler::toss_info() noexcept -> TossInfo&
	{
		return field_.get().toss_;
	}

	auto Field::TossInfoHandler::toss_info() const noexcept -> const TossInfo&
	{
		return field_.get().toss_;
	}

	auto Field::TossInfoHandler::toss_dice(const domain::Player player, const std::size_t count) noexcept -> void
	{
		auto& result = toss_info().toss_dice(count, field_.get().duel().random());

		// todo: 效果替换?
		std::ignore = player;
		std::ignore = result;

		// todo: 广播掷骰子
	}

	auto Field::TossInfoHandler::toss_coin(const domain::Player player, const std::size_t count) noexcept -> void
	{
		auto& result = toss_info().toss_coin(count, field_.get().duel().random());

		// todo: 效果替换?
		std::ignore = player;
		std::ignore = result;

		// todo: 广播掷硬币
	}

	auto Field::TossInfoHandler::get_dice(const domain::Player player) const noexcept -> const TossInfo::dice_results_type&
	{
		return toss_info().get_dice();
	}

	auto Field::TossInfoHandler::get_coin(const domain::Player player) const noexcept -> const TossInfo::coin_results_type&
	{
		return toss_info().get_coin();
	}

	Field::TurnHandler::TurnHandler(Field& field) noexcept
		: field_{field} {}

	auto Field::TurnHandler::turn_info() noexcept -> TurnInfo&
	{
		return field_.get().turn_;
	}

	auto Field::TurnHandler::turn_info() const noexcept -> const TurnInfo&
	{
		return field_.get().turn_;
	}

	auto Field::TurnHandler::set_turn_id(const domain::TurnId id) noexcept -> void
	{
		turn_info().set_turn_id(id);
	}

	auto Field::TurnHandler::set_phase(const domain::Phase phase) noexcept -> void
	{
		turn_info().set_phase(phase);
	}

	auto Field::TurnHandler::set_turn_phase(const domain::TurnPhase turn_phase) noexcept -> void
	{
		turn_info().set_turn_phase(turn_phase);
	}

	auto Field::TurnHandler::set_battle_step(const domain::BattleStep battle_step) noexcept -> void
	{
		turn_info().set_battle_step(battle_step);
	}

	auto Field::TurnHandler::set_damage_step(const domain::DamageStep damage_step) noexcept -> void
	{
		turn_info().set_damage_step(damage_step);
	}

	auto Field::TurnHandler::set_player(const domain::Player player) noexcept -> void
	{
		turn_info().set_player(player);
	}

	auto Field::TurnHandler::set_can_battle(const bool can_battle) noexcept -> void
	{
		turn_info().set_can_battle(can_battle);
	}

	auto Field::TurnHandler::set_can_normal_summon(const bool can_normal_summon) noexcept -> void
	{
		turn_info().set_can_normal_summon(can_normal_summon);
	}

	auto Field::TurnHandler::set_can_special_summon(const bool can_special_summon) noexcept -> void
	{
		turn_info().set_can_special_summon(can_special_summon);
	}

	auto Field::TurnHandler::turn_id() const noexcept -> domain::TurnId
	{
		return turn_info().turn_id();
	}

	auto Field::TurnHandler::phase() const noexcept -> domain::Phase
	{
		return turn_info().phase();
	}

	auto Field::TurnHandler::turn_phase() const noexcept -> domain::TurnPhase
	{
		return turn_info().turn_phase();
	}

	auto Field::TurnHandler::battle_step() const noexcept -> domain::BattleStep
	{
		return turn_info().battle_step();
	}

	auto Field::TurnHandler::damage_step() const noexcept -> domain::DamageStep
	{
		return turn_info().damage_step();
	}

	auto Field::TurnHandler::can_battle() const noexcept -> bool
	{
		return turn_info().can_battle();
	}

	auto Field::TurnHandler::can_normal_summon() const noexcept -> bool
	{
		return turn_info().can_normal_summon();
	}

	auto Field::TurnHandler::can_special_summon() const noexcept -> bool
	{
		return turn_info().can_special_summon();
	}

	auto Field::TurnHandler::is_turn_id(const domain::TurnId expected_id) const noexcept -> bool
	{
		return turn_info().is_turn_id(expected_id);
	}

	auto Field::TurnHandler::is_phase(const domain::Phase expected_phase) const noexcept -> bool
	{
		return turn_info().is_phase(expected_phase);
	}

	auto Field::TurnHandler::is_turn_phase(const domain::TurnPhase expected_turn_phase) const noexcept -> bool
	{
		return turn_info().is_turn_phase(expected_turn_phase);
	}

	auto Field::TurnHandler::is_battle_step(const domain::BattleStep expected_battle_step) const noexcept -> bool
	{
		return turn_info().is_battle_step(expected_battle_step);
	}

	auto Field::TurnHandler::is_damage_step(const domain::DamageStep expected_damage_step) const noexcept -> bool
	{
		return turn_info().is_damage_step(expected_damage_step);
	}

	auto Field::TurnHandler::new_turn(const domain::Player player) noexcept -> void
	{
		turn_info().new_turn(player);

		// todo: 广播抽卡阶段开始
	}

	auto Field::TurnHandler::advance_phase(const domain::TurnPhase turn_phase) noexcept -> void
	{
		turn_info().advance_phase(turn_phase);

		// todo: 广播指定阶段开始
	}

	Field::Field(Duel& duel) noexcept
		: duel_{duel}
	// playground_{},
	// toss_{},
	// turn_{}
	{
		//
	}

	auto Field::playground() noexcept -> PlaygroundHandler
	{
		return PlaygroundHandler{*this};
	}

	auto Field::random() noexcept -> TossInfoHandler
	{
		return TossInfoHandler{*this};
	}

	auto Field::turn() noexcept -> TurnHandler
	{
		return TurnHandler{*this};
	}
}

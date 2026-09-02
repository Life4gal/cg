#include <core/engine/turn_info.hpp>

namespace cg::engine
{
	TurnInfo::TurnInfo() noexcept
		: id_{static_cast<domain::TurnId>(0)},
		  phase_
		  {
				  .turn = domain::TurnPhase::DRAW,
				  .battle = domain::BattleStep::START,
				  .damage = domain::DamageStep::START,
		  },
		  player_{domain::Player::FIRST},
		  can_battle_{false},
		  can_normal_summon_{false},
		  can_special_summon_{false}
	{
		//
	}

	auto TurnInfo::set_turn_id(const domain::TurnId id) noexcept -> void
	{
		id_ = id;
	}

	auto TurnInfo::set_phase(const domain::Phase phase) noexcept -> void
	{
		phase_ = phase;
	}

	auto TurnInfo::set_turn_phase(const domain::TurnPhase turn_phase) noexcept -> void
	{
		phase_.turn = turn_phase;
	}

	auto TurnInfo::set_battle_step(const domain::BattleStep battle_step) noexcept -> void
	{
		phase_.battle = battle_step;
	}

	auto TurnInfo::set_damage_step(const domain::DamageStep damage_step) noexcept -> void
	{
		phase_.damage = damage_step;
	}

	auto TurnInfo::set_player(const domain::Player player) noexcept -> void
	{
		player_ = player;
	}

	auto TurnInfo::set_can_battle(const bool can_battle) noexcept -> void
	{
		can_battle_ = can_battle;
	}

	auto TurnInfo::set_can_normal_summon(const bool can_normal_summon) noexcept -> void
	{
		can_normal_summon_ = can_normal_summon;
	}

	auto TurnInfo::set_can_special_summon(const bool can_special_summon) noexcept -> void
	{
		can_special_summon_ = can_special_summon;
	}

	auto TurnInfo::turn_id() const noexcept -> domain::TurnId
	{
		return id_;
	}

	auto TurnInfo::phase() const noexcept -> domain::Phase
	{
		return phase_;
	}

	auto TurnInfo::turn_phase() const noexcept -> domain::TurnPhase
	{
		return phase_.turn;
	}

	auto TurnInfo::battle_step() const noexcept -> domain::BattleStep
	{
		return phase_.battle;
	}

	auto TurnInfo::damage_step() const noexcept -> domain::DamageStep
	{
		return phase_.damage;
	}

	auto TurnInfo::can_battle() const noexcept -> bool
	{
		return can_battle_;
	}

	auto TurnInfo::can_normal_summon() const noexcept -> bool
	{
		return can_normal_summon_;
	}

	auto TurnInfo::can_special_summon() const noexcept -> bool
	{
		return can_special_summon_;
	}

	auto TurnInfo::is_turn_id(const domain::TurnId expected_id) const noexcept -> bool
	{
		return turn_id() == expected_id;
	}

	auto TurnInfo::is_phase(const domain::Phase expected_phase) const noexcept -> bool
	{
		return phase_ == expected_phase;
	}

	auto TurnInfo::is_turn_phase(const domain::TurnPhase expected_turn_phase) const noexcept -> bool
	{
		return phase_.turn == expected_turn_phase;
	}

	auto TurnInfo::is_battle_step(const domain::BattleStep expected_battle_step) const noexcept -> bool
	{
		return phase_.battle == expected_battle_step;
	}

	auto TurnInfo::is_damage_step(const domain::DamageStep expected_damage_step) const noexcept -> bool
	{
		return phase_.damage == expected_damage_step;
	}

	auto TurnInfo::new_turn(const domain::Player player) noexcept -> void
	{
		// todo: 回合结束处理?

		const auto current_turn_id = turn_id();
		const auto next_turn_id = static_cast<domain::TurnId>(std::to_underlying(current_turn_id) + 1);

		// 设置回合ID
		set_turn_id(next_turn_id);
		// 设置当前阶段
		set_turn_phase(domain::TurnPhase::DRAW);
		set_battle_step(domain::BattleStep::START);
		set_damage_step(domain::DamageStep::START);
		// 设置回合玩家
		set_player(player);
		// 设置允许攻击
		set_can_battle(true);
		// 设置可以通常召唤
		set_can_normal_summon(true);
		// 设置可以特殊召唤
		set_can_special_summon(true);
	}

	auto TurnInfo::advance_phase(const domain::TurnPhase turn_phase) noexcept -> void
	{
		// 设置当前阶段
		set_turn_phase(turn_phase);
		set_battle_step(domain::BattleStep::START);
		set_damage_step(domain::DamageStep::START);
	}
}

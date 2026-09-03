#include <core/engine/turn_info.hpp>

namespace cg::engine
{
	TurnInfo::TurnInfo() noexcept
		: id{static_cast<domain::TurnId>(0)},
		  phase
		  {
				  .turn = domain::TurnPhase::DRAW,
				  .battle = domain::BattleStep::START,
				  .damage = domain::DamageStep::START,
		  },
		  player{domain::Player::FIRST},
		  can_battle{false},
		  can_normal_summon{false},
		  can_special_summon{false} {}

	auto TurnInfo::on_turn_end(const domain::Player target_player) noexcept -> void
	{
		// todo
		std::ignore = target_player;
	}

	auto TurnInfo::on_turn_begin(const domain::Player target_player) noexcept -> void
	{
		const auto next_id = static_cast<domain::TurnId>(std::to_underlying(id) + 1);

		// 设置回合ID
		id = next_id;
		// 设置当前阶段
		phase =
		{
				.turn = domain::TurnPhase::DRAW,
				.battle = domain::BattleStep::START,
				.damage = domain::DamageStep::START,
		};
		// 设置回合玩家
		player = target_player;
		// 设置允许攻击
		can_battle = true;
		// 设置可以通常召唤
		can_normal_summon = true;
		// 设置可以特殊召唤
		can_special_summon = true;
	}

	auto TurnInfo::advance_phase(const domain::TurnPhase turn_phase) noexcept -> void
	{
		// 设置当前阶段
		phase.turn = turn_phase;
		phase =
		{
				.turn = turn_phase,
				.battle = domain::BattleStep::START,
				.damage = domain::DamageStep::START,
		};
	}
}

#pragma once

#include <core/domain/turn.hpp>
#include <core/domain/player.hpp>

namespace cg::engine
{
	class Field;

	class TurnInfo
	{
	public:
		// 当前回合数
		domain::TurnId id;
		// 当前阶段
		domain::Phase phase;
		// 当前玩家
		domain::Player player;
		// 当前回合是否允许攻击
		bool can_battle;
		// 当前回合是否可通常召唤
		bool can_normal_summon;
		// 当前回合是否可以特殊召唤
		bool can_special_summon;

		TurnInfo() noexcept;

		// 指定玩家的回合结束
		auto on_turn_end(domain::Player target_player) noexcept -> void;
		// 指定玩家的回合开始
		auto on_turn_begin(domain::Player target_player) noexcept -> void;
		// 将当前的阶段前进到指定阶段
		auto advance_phase(domain::TurnPhase turn_phase) noexcept -> void;
	};
}

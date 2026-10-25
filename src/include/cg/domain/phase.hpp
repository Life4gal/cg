#pragma once

#include <cstdint>

namespace cg::domain
{
	// 回合ID
	enum class TurnId : std::uint16_t {};

	// 回合阶段
	enum class Phase : std::uint8_t
	{
		// 抽卡阶段
		DRAW,
		// 准备阶段
		STANDBY,
		// 主要阶段1
		MAIN_1,
		// 战斗阶段
		BATTLE,
		// 主要阶段2
		MAIN_2,
		// 结束阶段
		END,
	};

	// 战斗步骤
	// phase == Phase::BATTLE
	enum class BattleStep : std::uint8_t
	{
		// 开始阶段
		START,
		// 战斗阶段
		BATTLE,
		// 伤害计算阶段
		DAMAGE,
		// 结束阶段
		END,
	};

	// 伤害步骤
	// battle_step == BattleStep::DAMAGE
	enum class DamageStep : std::uint8_t
	{
		// 伤害步骤开始
		START,
		// 伤害计算前
		BEFORE_CALCULATION,
		// 伤害计算时
		PERFORM_CALCULATION,
		// 伤害计算后
		AFTER_CALCULATION,
		// 伤害步骤结束
		END,
	};
}

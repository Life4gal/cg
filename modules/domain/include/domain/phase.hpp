#pragma once

#include <cstdint>

namespace cg::domain
{
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
	enum class DamageStep : std::uint8_t
	{
		// 伤害步骤开始(里侧怪不翻开,处理"伤害步骤开始时"的效果)
		CONFIRM,
		// 伤害计算前(里侧怪翻开,增减攻防效果的最后发动时机)
		BEFORE_CALCULATE,
		// 伤害计算时(实际计算伤害,处理伤害)
		CALCULATING,
		// 伤害计算后(处理反转效果、战斗伤害诱发效果)
		AFTER_CALCULATE,
		// 伤害步骤结束(战斗破坏确定送去墓地,触发遗言效果)
		END,
	};
}

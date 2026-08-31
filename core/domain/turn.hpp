#pragma once

#include <cstdint>

namespace cg::domain
{
	// 回合阶段
	enum class TurnPhase : std::uint8_t
	{
		// 抽卡阶段
		DRAW,
		// 准备阶段
		STANDBY,
		// 主要阶段1
		MAIN1,
		// 战斗阶段
		BATTLE,
		// 主要阶段2
		MAIN2,
		// 结束阶段
		END,
	};

	// 战斗步骤(当且仅当TurnPhase为BATTLE时,才会有BattleStep)
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

	// 伤害步骤(当且仅当BattleStep为DAMAGE时,才会有DamageStep)
	enum class DamageStep : std::uint8_t
	{
		// 伤害步骤开始时(里侧怪不翻开,处理"伤害步骤开始时"的效果)
		START,
		// 伤害计算前(里侧怪翻开,增减攻防效果的最后发动时机)
		BEFORE_CALCULATION,
		// 伤害计算时(实际计算伤害,处理伤害)
		CALCULATION,
		// 伤害计算后(处理反转效果、战斗伤害诱发效果)
		AFTER_CALCULATION,
		// 伤害步骤结束时(战斗破坏确定送去墓地,触发遗言效果)
		END,
	};

	class Phase
	{
	public:
		TurnPhase turn;
		BattleStep battle;
		DamageStep damage;

		// 是否处于主要阶段
		[[nodiscard]] constexpr auto is_main_phase() const noexcept -> bool
		{
			return turn == TurnPhase::MAIN1 || turn == TurnPhase::MAIN2;
		}
	};
}

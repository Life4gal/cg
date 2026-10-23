#pragma once

#include <array>
#include <optional>

#include <cg/domain/phase.hpp>

namespace cg::flow
{
	// 战斗/伤害步骤 状态机
	class Battle
	{
	public:
		// 战斗步骤顺序
		constexpr static std::array step_order
		{
				domain::BattleStep::START,
				domain::BattleStep::BATTLE,
				domain::BattleStep::DAMAGE,
				domain::BattleStep::END,
		};

		// 伤害步骤顺序
		constexpr static std::array damage_step_order
		{
				domain::DamageStep::START,
				domain::DamageStep::BEFORE_CALCULATION,
				domain::DamageStep::PERFORM_CALCULATION,
				domain::DamageStep::AFTER_CALCULATION,
				domain::DamageStep::END,
		};

	private:
		// 当前战斗步骤
		domain::BattleStep battle_step_;
		// 当前伤害步骤 -- 仅在伤害步骤时有效
		domain::DamageStep damage_step_;

	public:
		Battle() noexcept;

		// 重置步骤信息
		// 进入战斗阶段时调用
		auto reset() noexcept -> void;

		// 当前战斗步骤
		[[nodiscard]] auto battle_step() const noexcept -> domain::BattleStep;

		// 下一个战斗步骤,如果是最后一个则返回null-opt
		[[nodiscard]] auto next_battle_step() const noexcept -> std::optional<domain::BattleStep>;

		// 推进到下一个战斗步骤
		auto advance_battle_step() noexcept -> bool;

		// 是否处于伤害步骤
		[[nodiscard]] auto is_in_damage_step() const noexcept -> bool;

		// 当前伤害步骤 -- 仅在伤害步骤时有效
		[[nodiscard]] auto damage_step() const noexcept -> domain::DamageStep;

		// 下一个伤害步骤,如果是最后一个则返回null-opt
		[[nodiscard]] auto next_damage_step() const noexcept -> std::optional<domain::DamageStep>;

		// 推进到下一个伤害步骤
		auto advance_damage_step() noexcept -> bool;
	};
}

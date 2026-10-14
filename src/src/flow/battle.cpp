#include <cg/flow/battle.hpp>

#include <algorithm>

#include <libassert/assert.hpp>

namespace cg::flow
{
	Battle::Battle() noexcept
		: battle_step_{domain::BattleStep::START},
		  damage_step_{domain::DamageStep::START} {}

	auto Battle::reset() noexcept -> void
	{
		battle_step_ = domain::BattleStep::START;
		damage_step_ = domain::DamageStep::START;
	}

	auto Battle::battle_step() const noexcept -> domain::BattleStep
	{
		return battle_step_;
	}

	auto Battle::next_battle_step() const noexcept -> std::optional<domain::BattleStep>
	{
		const auto it = std::ranges::find(step_order, battle_step_);
		if (it == step_order.end())
		{
			// 不存在的步骤?
			// 理论上不可能
			return std::nullopt;
		}
		const auto next_it = std::ranges::next(it);
		if (next_it == step_order.end())
		{
			// 结束步骤
			return std::nullopt;
		}

		return *next_it;
	}

	auto Battle::advance_battle_step() noexcept -> bool
	{
		const auto next = next_battle_step();
		if (!next.has_value())
		{
			return false;
		}

		// 设置为下一步骤
		battle_step_ = *next;

		// 如果下一步骤是伤害计算步骤则重置状态
		if (battle_step_ == domain::BattleStep::DAMAGE)
		{
			damage_step_ = domain::DamageStep::START;
		}

		return true;
	}

	auto Battle::is_in_damage_step() const noexcept -> bool
	{
		return battle_step_ == domain::BattleStep::DAMAGE;
	}

	auto Battle::damage_step() const noexcept -> domain::DamageStep
	{
		LIBASSERT_DEBUG_ASSERT(is_in_damage_step());
		// 如果真就发生意外情况,静默不处理

		return damage_step_;
	}

	auto Battle::next_damage_step() const noexcept -> std::optional<domain::DamageStep>
	{
		LIBASSERT_DEBUG_ASSERT(is_in_damage_step());
		// 如果真就发生意外情况,静默失败
		if (!is_in_damage_step())
		{
			return std::nullopt;
		}

		const auto it = std::ranges::find(damage_step_order, damage_step_);
		if (it == damage_step_order.end())
		{
			// 不存在的步骤?
			// 理论上不可能
			return std::nullopt;
		}
		const auto next_it = std::ranges::next(it);
		if (next_it == damage_step_order.end())
		{
			// 结束步骤
			return std::nullopt;
		}

		return *next_it;
	}

	auto Battle::advance_damage_step() noexcept -> bool
	{
		LIBASSERT_DEBUG_ASSERT(is_in_damage_step());
		// 如果真就发生意外情况,静默失败
		if (!is_in_damage_step())
		{
			return false;
		}

		const auto next = next_damage_step();
		if (!next.has_value())
		{
			return false;
		}

		// 设置为下一步骤
		damage_step_ = *next;

		return true;
	}
}

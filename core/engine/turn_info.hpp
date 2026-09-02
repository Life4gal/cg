#pragma once

#include <core/domain/turn.hpp>
#include <core/domain/player.hpp>

namespace cg::engine
{
	class TurnInfo
	{
	public:
		//

	private:
		// 当前回合数
		domain::TurnId id_;
		// 当前阶段
		domain::Phase phase_;
		// 当前玩家
		domain::Player player_;
		// 当前回合是否允许攻击
		bool can_battle_;
		// 当前回合是否可通常召唤
		bool can_normal_summon_;
		// 当前回合是否可以特殊召唤
		bool can_special_summon_;

	public:
		TurnInfo() noexcept;

		auto set_turn_id(domain::TurnId id) noexcept -> void;
		auto set_phase(domain::Phase phase) noexcept -> void;
		auto set_turn_phase(domain::TurnPhase turn_phase) noexcept -> void;
		auto set_battle_step(domain::BattleStep battle_step) noexcept -> void;
		auto set_damage_step(domain::DamageStep damage_step) noexcept -> void;
		auto set_player(domain::Player player) noexcept -> void;
		auto set_can_battle(bool can_battle) noexcept -> void;
		auto set_can_normal_summon(bool can_normal_summon) noexcept -> void;
		auto set_can_special_summon(bool can_special_summon) noexcept -> void;

		[[nodiscard]] auto turn_id() const noexcept -> domain::TurnId;
		[[nodiscard]] auto phase() const noexcept -> domain::Phase;
		[[nodiscard]] auto turn_phase() const noexcept -> domain::TurnPhase;
		[[nodiscard]] auto battle_step() const noexcept -> domain::BattleStep;
		[[nodiscard]] auto damage_step() const noexcept -> domain::DamageStep;
		[[nodiscard]] auto can_battle() const noexcept -> bool;
		[[nodiscard]] auto can_normal_summon() const noexcept -> bool;
		[[nodiscard]] auto can_special_summon() const noexcept -> bool;

		[[nodiscard]] auto is_turn_id(domain::TurnId expected_id) const noexcept -> bool;
		[[nodiscard]] auto is_phase(domain::Phase expected_phase) const noexcept -> bool;
		[[nodiscard]] auto is_turn_phase(domain::TurnPhase expected_turn_phase) const noexcept -> bool;
		[[nodiscard]] auto is_battle_step(domain::BattleStep expected_battle_step) const noexcept -> bool;
		[[nodiscard]] auto is_damage_step(domain::DamageStep expected_damage_step) const noexcept -> bool;

		// 开始下一回合
		auto new_turn(domain::Player player) noexcept -> void;
		// 前进到指定阶段(可用于跳过某个阶段)
		auto advance_phase(domain::TurnPhase turn_phase) noexcept -> void;
	};
}

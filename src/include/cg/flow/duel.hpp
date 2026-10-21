#pragma once

#include <optional>
#include <span>
#include <vector>

#include <cg/state/game.hpp>

#include <cg/flow/battle.hpp>
#include <cg/flow/priority.hpp>
#include <cg/flow/turn.hpp>

namespace cg::flow
{
	// 最小决斗驱动:状态 + 回合/优先权/战斗流程 + 核心动作
	class Duel
	{
	public:
		// 回合内状态
		class TurnState
		{
		public:
			// 是否已抽卡(抽卡阶段)
			bool drew_in_draw_phase = false;
			// 是否已进行通常召唤
			bool normal_summoned = false;
			// 是否已进行灵摆召唤
			bool pendulum_summoned = false;
			// 本回合是否已进入过战斗阶段
			bool entered_battle_phase = false;
			// 本回合是否已进行过攻击
			bool attacked = false;

			// 本回合已攻击过的怪兽
			std::vector<state::CardInstanceId> attacked_monsters;
		};

	private:
		state::Game game_;

		Turn turn_;
		Priority priority_;
		Battle battle_;

		TurnState state_;

		std::optional<domain::Player> loser_;

		auto perform_tribute_summon(state::CardInstanceId card, domain::FieldColumn column, std::span<const state::CardInstanceId> tributes, domain::Position position) noexcept -> bool;

	public:
		Duel() noexcept;

		[[nodiscard]] auto game() noexcept -> state::Game&;
		[[nodiscard]] auto game() const noexcept -> const state::Game&;

		[[nodiscard]] auto turn() noexcept -> Turn&;
		[[nodiscard]] auto turn() const noexcept -> const Turn&;

		[[nodiscard]] auto priority() noexcept -> Priority&;
		[[nodiscard]] auto priority() const noexcept -> const Priority&;

		[[nodiscard]] auto battle() noexcept -> Battle&;
		[[nodiscard]] auto battle() const noexcept -> const Battle&;

		// 胜负(生命值 ≤ 0)
		[[nodiscard]] auto winner() const noexcept -> std::optional<domain::Player>;

		// 抽卡阶段抽1张;卡组为空返回 false
		auto draw_for_turn() noexcept -> bool;

		// 通常召唤(表侧攻击表示)
		auto normal_summon(state::CardInstanceId card, domain::FieldColumn column) noexcept -> bool;

		// 盖放怪兽(里侧守备表示)
		auto set_monster(state::CardInstanceId card, domain::FieldColumn column) noexcept -> bool;

		// 上级召唤(祭品数 = 等级5~6:1,等级7+:2)
		auto tribute_summon(state::CardInstanceId card, domain::FieldColumn column, std::span<const state::CardInstanceId> tributes) noexcept -> bool;

		// 上级盖放(里侧守备表示)
		auto tribute_set(state::CardInstanceId card, domain::FieldColumn column, std::span<const state::CardInstanceId> tributes) noexcept -> bool;

		// 攻击宣言:target = 空表示直接攻击
		auto declare_attack(state::CardInstanceId attacker, std::optional<state::CardInstanceId> target) noexcept -> bool;

		// 推进阶段(委托Turn::advance_phase)
		auto change_phase() noexcept -> bool;

		// 结束回合(仅END阶段有效)
		auto end_turn() noexcept -> bool;
	};
}

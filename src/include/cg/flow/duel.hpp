#pragma once

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
		//

	private:
		state::Game game_;
		Turn turn_;
		Priority priority_;
		Battle battle_;

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

		// 抽卡阶段抽1张;卡组为空返回 false
		auto draw_for_turn() noexcept -> bool;

		// 通常召唤(表侧攻击表示)
		auto normal_summon(state::CardInstanceId card, domain::FieldColumn column) noexcept -> bool;

		// 盖放怪兽(里侧守备表示)
		auto set_monster(state::CardInstanceId card, domain::FieldColumn column) noexcept -> bool;

		// 推进阶段(委托Turn::advance_phase)
		auto change_phase() noexcept -> bool;

		// 结束回合(仅END阶段有效)
		auto end_turn() noexcept -> bool;
	};
}

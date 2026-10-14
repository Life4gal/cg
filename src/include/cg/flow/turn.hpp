#pragma once

#include <array>
#include <span>
#include <optional>

#include <cg/domain/phase.hpp>
#include <cg/domain/player.hpp>

namespace cg::flow
{
	class Turn
	{
	public:
		static_assert(std::to_underlying(domain::Phase::DRAW) == 0);

		// 完整阶段顺序
		constexpr static std::array full_order
		{
				// 抽卡
				domain::Phase::DRAW,
				// 准备阶段
				domain::Phase::STANDBY,
				// 主要阶段1
				domain::Phase::MAIN_1,
				// 战斗阶段
				domain::Phase::BATTLE,
				// 主要阶段2
				domain::Phase::MAIN_2,
				// 结束阶段
				domain::Phase::END,
		};

		// 第一回合阶段顺序
		constexpr static std::array first_turn_order
		{
				// 抽卡(不抽卡)
				domain::Phase::DRAW,
				// 准备阶段
				domain::Phase::STANDBY,
				// 主要阶段1
				domain::Phase::MAIN_1,
				// 结束阶段
				domain::Phase::END,
		};

		// 回合内玩家状态
		// TODO: 这个状态放这里吗?
		class State
		{
		public:
			// 是否已抽卡(抽卡阶段)
			bool drew_in_draw_phase;
			// 是否已进行通常召唤
			bool normal_summoned;
			// 是否已进行灵摆召唤
			bool pendulum_summoned;
			// 本回合是否已进入过战斗阶段
			bool entered_battle_phase;
			// 本回合是否已进行过攻击
			bool attacked;
		};

		// 回合数/ID
		using id_type = std::uint16_t;

	private:
		// 当前回合玩家 -- 逐回合切换
		domain::Player turn_player_;
		// 当前回合数/ID
		id_type turn_id_;
		// 当前阶段
		domain::Phase phase_;
		// 当前回合玩家的回合内状态
		State state_;

		[[nodiscard]] static auto default_state() noexcept -> State;

	public:
		explicit Turn() noexcept;

		// 当前回合玩家
		[[nodiscard]] auto turn_player() const noexcept -> domain::Player;

		// 当前回合数/ID
		[[nodiscard]] auto turn_id() const noexcept -> id_type;

		// 当前是否是第一回合
		[[nodiscard]] auto is_first_turn() const noexcept -> bool;

		// 当前阶段
		[[nodiscard]] auto phase() const noexcept -> domain::Phase;

		// 当前回合玩家的回合内状态
		[[nodiscard]] auto state() noexcept -> State&;
		[[nodiscard]] auto state() const noexcept -> const State&;

		// 可推进到的阶段顺序 -- 不是当前阶段可推进到的阶段顺序,而是当前回合可推进到的阶段顺序
		[[nodiscard]] auto available_phase_order() const noexcept -> std::span<const domain::Phase>;

		// 当前阶段的下一个阶段,如果是最后一个阶段则返回null-opt
		[[nodiscard]] auto next_phase() const noexcept -> std::optional<domain::Phase>;

		// 推进到下一个阶段
		// 返回是否成功,已经是最后一个阶段时失败,此时需要使用end_turn结束回合
		auto advance_phase() noexcept -> bool;

		// 结束当前回合
		// 当且仅当 phase == domain::Phase::END 时有效
		auto end_turn() noexcept -> void;
	};
}

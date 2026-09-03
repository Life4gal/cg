#pragma once

#include <algorithm>

#include <core/domain/player.hpp>
#include <core/domain/toss.hpp>
#include <core/domain/turn.hpp>
#include <core/domain/zone.hpp>
#include <core/domain/summon.hpp>

namespace cg::domain
{
	class GameEvent
	{
	public:
		using size_type = Zone::size_type;

		// ==================== 决斗 ====================

		class DuelStarted
		{
		public:
			[[nodiscard]] constexpr auto operator==(const DuelStarted& other) const noexcept -> bool = default;
		};

		// ==================== 掷骰子/硬币 ====================

		class TossDice
		{
		public:
			Player player;
			dice_results_type results;

			[[nodiscard]] constexpr auto operator==(const TossDice& other) const noexcept -> bool
			{
				return player == other.player && std::ranges::equal(results, other.results);
			}
		};

		class TossCoin
		{
		public:
			Player player;
			coin_results_type results;

			[[nodiscard]] constexpr auto operator==(const TossCoin& other) const noexcept -> bool
			{
				return player == other.player && std::ranges::equal(results, other.results);
			}
		};

		// ==================== 回合/阶段 ====================

		class TurnStarted
		{
		public:
			Player player;
			TurnId turn_id;

			[[nodiscard]] constexpr auto operator==(const TurnStarted& other) const noexcept -> bool = default;
		};

		class PhaseChanged
		{
		public:
			Phase phase;

			[[nodiscard]] constexpr auto operator==(const PhaseChanged& other) const noexcept -> bool = default;
		};

		// ==================== 卡组 ====================

		class ShuffleDeck
		{
		public:
			Player player;
			Zone zone;

			[[nodiscard]] constexpr auto operator==(const ShuffleDeck& other) const noexcept -> bool = default;
		};

		class ReverseDeck
		{
		public:
			Player player;
			Zone zone;

			[[nodiscard]] constexpr auto operator==(const ReverseDeck& other) const noexcept -> bool = default;
		};

		// ==================== 召唤 ====================

		class Summon
		{
		public:
			SummonKind kind;
			// todo: 卡牌信息

			[[nodiscard]] constexpr auto operator==(const Summon& other) const noexcept -> bool = default;
		};

		class SetSummon
		{
		public:
			// todo: 卡牌信息

			[[nodiscard]] constexpr auto operator==(const SetSummon& other) const noexcept -> bool = default;
		};

		class FlipSummon
		{
		public:
			// todo: 卡牌信息

			[[nodiscard]] constexpr auto operator==(const FlipSummon& other) const noexcept -> bool = default;
		};

		class SummonNegated
		{
		public:
			// todo: 卡牌信息

			[[nodiscard]] constexpr auto operator==(const SummonNegated& other) const noexcept -> bool = default;
		};

		// ==================== 战斗 ====================

		class AttackAnnounced
		{
		public:
			// todo: 卡牌信息

			[[nodiscard]] constexpr auto operator==(const AttackAnnounced& other) const noexcept -> bool = default;
		};

		// ==================== | ====================

		class None
		{
		public:
			[[nodiscard]] constexpr auto operator==(const None& other) const noexcept -> bool = default;
		};

		using event_type = std::variant<
			// ==================== 决斗 ====================
			DuelStarted,
			// ==================== 掷骰子/硬币 ====================
			TossDice,
			TossCoin,
			// ==================== 回合/阶段 ====================
			TurnStarted,
			PhaseChanged,
			// ==================== 卡组 ====================
			ShuffleDeck,
			ReverseDeck,
			// ==================== 召唤 ====================
			Summon,
			SetSummon,
			FlipSummon,
			SummonNegated,
			// ==================== 战斗 ====================
			AttackAnnounced,
			// ==================== | ====================
			None
		>;

		constexpr static None none{};

	private:
		event_type event_;

	public:
		GameEvent() noexcept
			: event_{none} {}

		template<typename Event>
		constexpr explicit(false) GameEvent(Event e) noexcept
			requires std::is_constructible_v<event_type, Event>
			: event_{std::move(e)} {}

		[[nodiscard]] constexpr auto operator==(const GameEvent& other) const noexcept -> bool = default;

		// 检查是否是指定事件
		template<typename Event>
		[[nodiscard]] constexpr auto is() const noexcept -> bool
		{
			return std::holds_alternative<Event>(event_);
		}

		// 获取指定事件的数据
		template<typename Event>
		[[nodiscard]] constexpr auto get() const noexcept -> decltype(auto)
		{
			return std::get<Event>(event_);
		}

		// visit
		template<typename Visitor>
		constexpr auto visit(Visitor visitor) const noexcept -> auto //
			requires requires { std::visit(visitor, event_); }
		{
			return std::visit(visitor, event_);
		}
	};
}

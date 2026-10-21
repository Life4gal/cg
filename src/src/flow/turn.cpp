#include <cg/flow/turn.hpp>

#include <algorithm>

namespace cg::flow
{
	Turn::Turn() noexcept
		: turn_player_{domain::Player::FIRST},
		  turn_id_{1},
		  phase_{domain::Phase::DRAW} {}

	auto Turn::turn_player() const noexcept -> domain::Player
	{
		return turn_player_;
	}

	auto Turn::turn_id() const noexcept -> id_type
	{
		return turn_id_;
	}

	auto Turn::is_first_turn() const noexcept -> bool
	{
		return turn_id_ == 1;
	}

	auto Turn::phase() const noexcept -> domain::Phase
	{
		return phase_;
	}

	auto Turn::available_phase_order() const noexcept -> std::span<const domain::Phase>
	{
		if (is_first_turn())
		{
			return first_turn_order;
		}

		return full_order;
	}

	auto Turn::next_phase() const noexcept -> std::optional<domain::Phase>
	{
		const auto order = available_phase_order();
		const auto it = std::ranges::find(order, phase_);
		if (it == order.end())
		{
			// 不存在的阶段?
			// 理论上不可能
			return std::nullopt;
		}
		const auto next_it = std::ranges::next(it);
		if (next_it == order.end())
		{
			// 结束阶段
			return std::nullopt;
		}

		return *next_it;
	}

	auto Turn::advance_phase() noexcept -> bool
	{
		const auto next = next_phase();
		if (!next.has_value())
		{
			return false;
		}

		// 设置为下一阶段
		phase_ = *next;

		return true;
	}

	auto Turn::end_turn() noexcept -> void
	{
		if (phase_ != domain::Phase::END)
		{
			return;
		}

		turn_player_ = domain::opponent_of(turn_player_);
		turn_id_ += 1;
		phase_ = domain::Phase::DRAW;
	}
}

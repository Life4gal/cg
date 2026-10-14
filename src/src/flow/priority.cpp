#include <cg/flow/priority.hpp>

namespace cg::flow
{
	Priority::Priority() noexcept
		: holder_{Holder::TURN_PLAYER},
		  passed_{false} {}

	auto Priority::holder() const noexcept -> Holder
	{
		return holder_;
	}

	auto Priority::passed() const noexcept -> bool
	{
		return passed_;
	}

	auto Priority::both_passed() const noexcept -> bool
	{
		// 非回合玩家持有优先权且已放弃 → 双方都放弃了
		return holder_ == Holder::NON_TURN_PLAYER && passed_;
	}

	auto Priority::pass() noexcept -> void
	{
		if (!passed_)
		{
			// 当前持有者第一次放弃
			passed_ = true;
			if (holder_ == Holder::TURN_PLAYER)
			{
				// 回合玩家放弃 → 转移给非回合玩家
				holder_ = Holder::NON_TURN_PLAYER;
				passed_ = false;
			}
			// 非回合玩家放弃 → 保持 NON_TURN_PLAYER + passed_ = true
		}
		// 如果已经 passed_，重复调用无效果
	}

	auto Priority::reset() noexcept -> void
	{
		holder_ = Holder::TURN_PLAYER;
		passed_ = false;
	}

	auto Priority::holder_player(const domain::Player turn_player) const noexcept -> domain::Player
	{
		if (holder_ == Holder::TURN_PLAYER)
		{
			return turn_player;
		}

		return domain::opponent_of(turn_player);
	}
}

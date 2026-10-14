#pragma once

#include <utility>

namespace cg::domain
{
	// 玩家绝对代号
	enum class Player : std::uint8_t
	{
		// 先攻玩家
		FIRST = 0,
		// 后攻玩家
		SECOND = 1,
	};

	[[nodiscard]] constexpr auto opponent_of(const Player player) noexcept -> Player
	{
		return static_cast<Player>(1 - std::to_underlying(player));
	}

	static_assert(opponent_of(Player::FIRST) == Player::SECOND);
	static_assert(opponent_of(Player::SECOND) == Player::FIRST);

	// 玩家指代代号
	enum class RelativePlayer : std::uint8_t
	{
		// 自己
		SELF,
		// 对方
		OPPONENT,
	};
}

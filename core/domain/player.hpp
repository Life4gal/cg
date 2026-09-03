#pragma once

#include <utility>

namespace cg::domain
{
	enum class Player : std::uint8_t
	{
		FIRST = 0,
		SECOND = 1,
	};

	constexpr auto player_count = static_cast<std::size_t>(2);

	[[nodiscard]] constexpr auto opponent_of(const Player player) noexcept -> Player
	{
		return static_cast<Player>(1 - std::to_underlying(player));
	}

	enum class PlayerReference : std::uint8_t
	{
		// 先手玩家
		FIRST,
		// 后手玩家
		SECOND,
		// 无玩家
		NONE,
		// 双方玩家
		BOTH,
	};
}

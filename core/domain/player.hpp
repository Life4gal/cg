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
}

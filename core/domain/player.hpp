#pragma once

#include <utility>

namespace cg::domain
{
	enum class Player : std::uint8_t
	{
		RED = 0,
		BLUE = 1,

		COUNT = 2,
	};

	[[nodiscard]] constexpr auto opponent_of(const Player player) noexcept -> Player
	{
		return static_cast<Player>(1 - std::to_underlying(player));
	}
}

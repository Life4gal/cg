#pragma once

#include <utility>

namespace cg::domain
{
	enum class DiceValue : std::uint8_t
	{
		ONE = 1,
		TWO = 2,
		THREE = 3,
		FOUR = 4,
		FIVE = 5,
		SIX = 6,

		COUNT = 6,
	};

	enum class CoinSide : std::uint8_t
	{
		HEADS = 0,
		TAILS = 1,

		COUNT = 2,
	};
}

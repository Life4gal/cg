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
	};

	constexpr auto dice_min_value = DiceValue::ONE;
	constexpr auto dice_max_value = DiceValue::SIX;

	enum class CoinSide : std::uint8_t
	{
		HEADS = 0,
		TAILS = 1,
	};

	constexpr auto coin_min_value = CoinSide::HEADS;
	constexpr auto coin_max_value = CoinSide::TAILS;
}

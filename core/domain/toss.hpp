#pragma once

#include <type_traits>

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

	constexpr auto dice_min_value = static_cast<std::underlying_type_t<DiceValue>>(DiceValue::ONE);
	constexpr auto dice_max_value = static_cast<std::underlying_type_t<DiceValue>>(DiceValue::SIX);

	enum class CoinSide : std::uint8_t
	{
		HEADS = 0,
		TAILS = 1,
	};

	constexpr auto coin_min_value = static_cast<std::underlying_type_t<CoinSide>>(CoinSide::HEADS);
	constexpr auto coin_max_value = static_cast<std::underlying_type_t<CoinSide>>(CoinSide::TAILS);
}

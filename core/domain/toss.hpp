#pragma once

#include <span>

namespace cg::domain
{
	// ==================== 骰子 ====================

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

	// 最多可以掷多少个骰子
	constexpr auto dice_count_max = static_cast<std::underlying_type_t<DiceValue>>(5);
	// 掷骰子的结果
	using dice_results_type = std::span<const DiceValue>;

	// ==================== 硬币 ====================

	enum class CoinSide : std::underlying_type_t<DiceValue>
	{
		HEADS = 0,
		TAILS = 1,
	};

	constexpr auto coin_min_value = static_cast<std::underlying_type_t<CoinSide>>(CoinSide::HEADS);
	constexpr auto coin_max_value = static_cast<std::underlying_type_t<CoinSide>>(CoinSide::TAILS);

	// 最多可以掷多少个硬币
	constexpr auto coin_count_max = static_cast<std::underlying_type_t<CoinSide>>(5);
	// 掷硬币的结果
	using coin_results_type = std::span<const CoinSide>;
}

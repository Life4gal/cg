#pragma once

#include <array>

#include <core/utility/random.hpp>

#include <core/domain/toss.hpp>

namespace cg::engine
{
	class Field;

	class TossInfo
	{
	public:
		using dices_type = std::array<domain::DiceValue, domain::dice_count_max>;
		using coins_type = std::array<domain::CoinSide, domain::coin_count_max>;

		using dice_results_type = std::span<domain::DiceValue>;
		using coin_results_type = std::span<domain::CoinSide>;

		using size_type = std::underlying_type_t<domain::DiceValue>;

		dice_results_type dice_results;
		coin_results_type coin_results;
		size_type dice_count;
		size_type coin_count;

		TossInfo() noexcept;

		auto toss_dice(size_type count, utility::Random& random) noexcept -> void;
		auto toss_coin(size_type count, utility::Random& random) noexcept -> void;
	};
}

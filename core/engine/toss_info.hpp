#pragma once

#include <vector>

#include <core/utility/random.hpp>

#include <core/domain/toss.hpp>

namespace cg::engine
{
	class TossInfo
	{
	public:
		using dice_results_type = std::vector<domain::DiceValue>;
		using coin_results_type = std::vector<domain::CoinSide>;

	private:
		dice_results_type dice_results_;
		coin_results_type coin_results_;

	public:
		TossInfo() noexcept;

		auto toss_dice(std::size_t count, utility::Random& random) noexcept -> dice_results_type&;
		auto toss_coin(std::size_t count, utility::Random& random) noexcept -> coin_results_type&;

		[[nodiscard]] auto get_dice() const noexcept -> const dice_results_type&;
		[[nodiscard]] auto get_coin() const noexcept -> const coin_results_type&;
	};
}

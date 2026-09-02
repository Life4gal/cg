#include <core/engine/toss_info.hpp>

namespace cg::engine
{
	TossInfo::TossInfo() noexcept = default;
	// 	: dice_results_{},
	// 	  coin_results_{}
	// {
	// 	//
	// }

	auto TossInfo::toss_dice(const std::size_t count, utility::Random& random) noexcept -> dice_results_type&
	{
		dice_results_.clear();
		dice_results_.reserve(count);

		std::ranges::generate_n(
			std::back_inserter(dice_results_),
			static_cast<std::ptrdiff_t>(count),
			[&random] noexcept -> domain::DiceValue
			{
				const auto value = random.int_inclusive(static_cast<std::uint32_t>(domain::dice_min_value), domain::dice_max_value);
				return static_cast<domain::DiceValue>(value);
			}
		);

		return dice_results_;
	}

	auto TossInfo::toss_coin(const std::size_t count, utility::Random& random) noexcept -> coin_results_type&
	{
		coin_results_.clear();
		coin_results_.reserve(count);

		std::ranges::generate_n(
			std::back_inserter(coin_results_),
			static_cast<std::ptrdiff_t>(count),
			[&random] noexcept -> domain::CoinSide
			{
				const auto value = random.int_inclusive(static_cast<std::uint32_t>(domain::coin_min_value), domain::coin_max_value);
				return static_cast<domain::CoinSide>(value);
			}
		);

		return coin_results_;
	}

	auto TossInfo::get_dice() const noexcept -> const dice_results_type&
	{
		return dice_results_;
	}

	auto TossInfo::get_coin() const noexcept -> const coin_results_type&
	{
		return coin_results_;
	}
}

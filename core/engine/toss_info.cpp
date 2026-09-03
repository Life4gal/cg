#include <core/engine/toss_info.hpp>

#include <algorithm>

#include <core/engine/field.hpp>

namespace cg::engine
{
	TossInfo::TossInfo() noexcept
		: //dice_results{},
		//coin_results{},
		dice_count{0},
		coin_count{0} {}

	auto TossInfo::toss_dice(size_type count, utility::Random& random) noexcept -> void
	{
		count = std::ranges::min(count, domain::dice_count_max);
		dice_count = count;

		std::ranges::generate_n(
			dice_results.begin(),
			static_cast<std::ptrdiff_t>(count),
			[&random] noexcept -> domain::DiceValue
			{
				const auto value = random.int_inclusive(static_cast<std::uint32_t>(domain::dice_min_value), domain::dice_max_value);
				return static_cast<domain::DiceValue>(value);
			}
		);
	}

	auto TossInfo::toss_coin(size_type count, utility::Random& random) noexcept -> void
	{
		count = std::ranges::min(count, domain::coin_count_max);
		coin_count = count;

		std::ranges::generate_n(
			coin_results.begin(),
			static_cast<std::ptrdiff_t>(count),
			[&random] noexcept -> domain::CoinSide
			{
				const auto value = random.int_inclusive(static_cast<std::uint32_t>(domain::coin_min_value), domain::coin_max_value);
				return static_cast<domain::CoinSide>(value);
			}
		);
	}
}

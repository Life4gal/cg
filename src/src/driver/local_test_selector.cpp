#include <cg/driver/local_test_selector.hpp>

#include <algorithm>
#include <ranges>
#include <numeric>

namespace cg::driver
{
	namespace
	{
		// 缺省选择:取前 min 张
		[[nodiscard]] auto select_first(
			[[maybe_unused]] domain::Player player,
			const execution::Group& candidates,
			const std::size_t min,
			[[maybe_unused]] const std::size_t max
		) noexcept -> execution::Group
		{
			const auto count = std::ranges::min(min, candidates.size());
			return {candidates.begin(), candidates.begin() + static_cast<std::ptrdiff_t>(count)};
		}

		// 缺省触发选择:必发全选;选发默认不选
		[[nodiscard]] auto choose_mandatory(const execution::TriggerDecision& decision) noexcept -> std::vector<std::size_t>
		{
			if (!decision.optional)
			{
				std::vector<std::size_t> indices(decision.candidates.size());
				std::ranges::iota(indices, std::size_t{0});
				return indices;
			}

			return {};
		}
	}

	LocalTestSelector::LocalTestSelector() noexcept
		: LocalTestSelector{nullptr, nullptr} {}

	LocalTestSelector::LocalTestSelector(select_type select, choose_type choose) noexcept
		: select_{select ? std::move(select) : select_first},
		  choose_{choose ? std::move(choose) : choose_mandatory} {}

	auto LocalTestSelector::select_cards(const domain::Player player, const execution::Group& candidates, const std::size_t min, const std::size_t max) noexcept -> execution::Group
	{
		return select_(player, candidates, min, max);
	}

	auto LocalTestSelector::choose_triggers(const execution::TriggerDecision& decision) noexcept -> std::vector<std::size_t>
	{
		return choose_(decision);
	}
}

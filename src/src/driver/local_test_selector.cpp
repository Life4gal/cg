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

		// 默认位置选择:取第一个候选
		template<typename Location>
		[[nodiscard]] auto select_first_location(
			[[maybe_unused]] const domain::Player player,
			[[maybe_unused]] const state::CardInstanceId card,
			const std::span<const Location> candidates
		) noexcept -> std::optional<Location>
		{
			if (candidates.empty())
			{
				return std::nullopt;
			}

			return candidates.front();
		}

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
		: LocalTestSelector{nullptr, nullptr, nullptr, nullptr} {}

	LocalTestSelector::LocalTestSelector(
		select_card_type select,
		select_monster_location_type select_monster_location,
		select_spell_trap_location_type select_spell_trap_location,
		choose_trigger_type choose
	) noexcept
		: select_{select ? std::move(select) : select_first},
		  select_monster_location_{select_monster_location ? std::move(select_monster_location) : select_first_location<domain::MonsterLocation>},
		  select_spell_trap_location_{select_spell_trap_location ? std::move(select_spell_trap_location) : select_first_location<domain::SpellTrapLocation>},
		  choose_{choose ? std::move(choose) : choose_mandatory} {}

	auto LocalTestSelector::select_cards(const domain::Player player, const execution::Group& candidates, const std::size_t min, const std::size_t max) noexcept -> execution::Group
	{
		return select_(player, candidates, min, max);
	}

	auto LocalTestSelector::select_monster_location(
		const domain::Player player,
		const state::CardInstanceId card,
		const std::span<const domain::MonsterLocation> candidates
	) noexcept -> std::optional<domain::MonsterLocation>
	{
		return select_monster_location_(player, card, candidates);
	}

	auto LocalTestSelector::select_spell_trap_location(
		const domain::Player player,
		const state::CardInstanceId card,
		const std::span<const domain::SpellTrapLocation> candidates
	) noexcept -> std::optional<domain::SpellTrapLocation>
	{
		return select_spell_trap_location_(player, card, candidates);
	}

	auto LocalTestSelector::choose_triggers(const execution::TriggerDecision& decision) noexcept -> std::vector<std::size_t>
	{
		return choose_(decision);
	}
}

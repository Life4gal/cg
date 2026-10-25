#pragma once

#include <functional>

#include <cg/execution/decision.hpp>

namespace cg::driver
{
	// 测试用 Selector:默认只选必发,选发/目标选择需注入回调.
	// 仅 tests/ 使用;不属于稳定驱动 API.
	class LocalTestSelector final : public execution::Selector
	{
	public:
		using select_card_type = std::function<
			auto(
				domain::Player player,
				const execution::Group& candidates,
				std::size_t min,
				std::size_t max
			) /*noexcept*/ -> execution::Group>;
		using select_monster_location_type = std::function<
			auto(
				domain::Player player,
				state::CardInstanceId card,
				std::span<const domain::MonsterLocation> candidates
			) /*noexcept*/ -> std::optional<domain::MonsterLocation>>;
		using select_spell_trap_location_type = std::function<
			auto(
				domain::Player player,
				state::CardInstanceId card,
				std::span<const domain::SpellTrapLocation> candidates
			) /*noexcept*/ -> std::optional<domain::SpellTrapLocation>>;

		using choose_trigger_type = std::function<
			auto(
				const execution::TriggerDecision& decision
			) /*noexcept*/ -> std::vector<std::size_t>>;

	private:
		select_card_type select_;
		select_monster_location_type select_monster_location_;
		select_spell_trap_location_type select_spell_trap_location_;

		choose_trigger_type choose_;

	public:
		LocalTestSelector() noexcept;

		LocalTestSelector(
			select_card_type select,
			select_monster_location_type select_monster_location,
			select_spell_trap_location_type select_spell_trap_location,
			choose_trigger_type choose
		) noexcept;

		auto select_cards(domain::Player player, const execution::Group& candidates, std::size_t min, std::size_t max) noexcept -> execution::Group override;

		auto select_monster_location(
			domain::Player player,
			state::CardInstanceId card,
			std::span<const domain::MonsterLocation> candidates
		) noexcept -> std::optional<domain::MonsterLocation> override;

		auto select_spell_trap_location(
			domain::Player player,
			state::CardInstanceId card,
			std::span<const domain::SpellTrapLocation> candidates
		) noexcept -> std::optional<domain::SpellTrapLocation> override;

		auto choose_triggers(const execution::TriggerDecision& decision) noexcept -> std::vector<std::size_t> override;
	};
}

#pragma once

#include <functional>

#include <cg/execution/decision.hpp>

namespace cg::driver
{
	class LocalTestSelector final : public execution::Selector
	{
	public:
		using select_type = std::function<auto(domain::Player player, const execution::Group& candidates, std::size_t min, std::size_t max) /*noexcept*/ -> execution::Group>;
		using choose_type = std::function<auto(const execution::TriggerDecision& decision) /*noexcept*/ -> std::vector<std::size_t>>;

	private:
		select_type select_;
		choose_type choose_;

	public:
		LocalTestSelector() noexcept;

		LocalTestSelector(select_type select, choose_type choose) noexcept;

		auto select_cards(domain::Player player, const execution::Group& candidates, std::size_t min, std::size_t max) noexcept -> execution::Group override;

		auto choose_triggers(const execution::TriggerDecision& decision) noexcept -> std::vector<std::size_t> override;
	};
}

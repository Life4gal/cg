#include <core/card_target.hpp>

namespace cg
{
	TargetCard::TargetCard(card_type card) noexcept
		: card_{std::move(card)}
	{
		//
	}

	TargetCard::TargetCard(TargetCard&&) noexcept = default;

	auto TargetCard::operator=(TargetCard&&) noexcept -> TargetCard& = default;

	TargetCard::~TargetCard() noexcept = default;

	TargetCards::TargetCards() noexcept = default;

	auto TargetCards::reserve(const size_type capacity) noexcept -> void
	{
		targets_.reserve(capacity);
	}

	auto TargetCards::add(TargetCard&& target) noexcept -> void
	{
		targets_.push_back(std::move(target));
	}

	auto TargetCards::size() const noexcept -> size_type
	{
		return targets_.size();
	}
}

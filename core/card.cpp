#include <core/card.hpp>

namespace cg
{
	Card::Card() noexcept = default;

	Card::Card(Card&&) noexcept = default;

	auto Card::operator=(Card&&) noexcept -> Card& = default;

	Card::~Card() noexcept = default;

	auto Card::card_category() const noexcept -> CardCategory
	{
		return card_category_;
	}
}

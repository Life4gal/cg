#include <core/card_material.hpp>

namespace cg
{
	SummonMaterialCard::SummonMaterialCard(card_type card) noexcept
		: card_{std::move(card)}
	{
		//
	}

	SummonMaterialCard::SummonMaterialCard(SummonMaterialCard&&) noexcept = default;

	auto SummonMaterialCard::operator=(SummonMaterialCard&&) noexcept -> SummonMaterialCard& = default;

	SummonMaterialCard::~SummonMaterialCard() noexcept = default;

	SummonMaterialCards::SummonMaterialCards() noexcept = default;

	auto SummonMaterialCards::reserve(const size_type capacity) noexcept -> void
	{
		materials_.reserve(capacity);
	}

	auto SummonMaterialCards::add(SummonMaterialCard&& material) noexcept -> void
	{
		materials_.push_back(std::move(material));
	}

	auto SummonMaterialCards::size() const noexcept -> size_type
	{
		return materials_.size();
	}

	XyzMaterialCard::XyzMaterialCard(card_type card) noexcept
		: card_{std::move(card)}
	{
		//
	}

	XyzMaterialCard::XyzMaterialCard(XyzMaterialCard&&) noexcept = default;

	auto XyzMaterialCard::operator=(XyzMaterialCard&&) noexcept -> XyzMaterialCard& = default;

	XyzMaterialCard::~XyzMaterialCard() noexcept = default;

	XyzMaterialCards::XyzMaterialCards() noexcept = default;

	auto XyzMaterialCards::reserve(const size_type capacity) noexcept -> void
	{
		materials_.reserve(capacity);
	}

	auto XyzMaterialCards::add(XyzMaterialCard&& material) noexcept -> void
	{
		materials_.push_back(std::move(material));
	}

	auto XyzMaterialCards::remove(const size_type index) noexcept -> bool
	{
		if (index >= materials_.size())
		{
			return false;
		}

		const auto it = std::ranges::next(materials_.begin(), static_cast<std::ptrdiff_t>(index));
		materials_.erase(it);

		return true;
	}

	auto XyzMaterialCards::size() const noexcept -> size_type
	{
		return materials_.size();
	}
}

#pragma once

#include <functional>

namespace cg::engine
{
	class Card;

	class CardReference : public std::reference_wrapper<Card>
	{
	public:
		using reference_wrapper::reference_wrapper;

		[[nodiscard]] constexpr auto operator==(const CardReference& other) const noexcept -> bool
		{
			// 必须引用同一个实例
			return &get() == &other.get();
		}
	};

	class CardOptional
	{
	public:
		// todo: 一个更安全的可选保有Card实例的包装类,避免危险的裸指针

	private:
		Card* card_;

	public:
		constexpr CardOptional() noexcept
			: CardOptional{nullptr} {}

		constexpr explicit(false) CardOptional(std::nullptr_t) noexcept
			: card_{nullptr} {}

		constexpr explicit CardOptional(Card* card) noexcept
			: card_{card} {}

		constexpr explicit CardOptional(const CardReference& reference) noexcept
			: card_{&reference.get()} {}

		[[nodiscard]] constexpr auto operator*() const noexcept -> Card&
		{
			return *card_;
		}

		// 不传播const
		// [[nodiscard]] constexpr auto operator*() const noexcept -> const Card&
		// {
		// 	return *card_;
		// }

		[[nodiscard]] constexpr auto operator->() const noexcept -> Card*
		{
			return card_;
		}

		// 不传播const
		// [[nodiscard]] constexpr auto operator->() const noexcept -> const Card*
		// {
		// 	return card_;
		// }

		[[nodiscard]] constexpr explicit operator bool() const noexcept
		{
			return card_ != nullptr;
		}

		[[nodiscard]] constexpr auto operator==(const Card* other) const noexcept -> bool
		{
			return card_ == other;
		}

		[[nodiscard]] constexpr auto operator==(const CardOptional& other) const noexcept -> bool
		{
			return *this == other.card_;
		}

		[[nodiscard]] constexpr auto operator==(const CardReference& other) const noexcept -> bool
		{
			return card_ != nullptr && card_ == &other.get();
		}

		[[nodiscard]] friend constexpr auto operator==(const CardReference& lhs, const CardOptional& rhs) noexcept -> bool
		{
			return rhs == lhs;
		}
	};
}

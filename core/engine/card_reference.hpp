#pragma once

#include <functional>

namespace cg::engine
{
	using card_instance_id_type = std::uint64_t;

	class Card;

	class CardReference : public std::reference_wrapper<Card>
	{
	public:
		using reference_wrapper::reference_wrapper;
	};

	class CardOptional
	{
	public:
		// todo: 一个更安全的可选保有Card实例的包装类,避免危险的裸指针

	private:
		Card* card_;

	public:
		constexpr CardOptional() noexcept
			: card_{nullptr} {}

		constexpr explicit CardOptional(Card* card) noexcept
			: card_{card} {}

		constexpr explicit CardOptional(const CardReference reference) noexcept
			: card_{&reference.get()} {}

		[[nodiscard]] constexpr auto get() const noexcept -> Card*
		{
			return card_;
		}

		[[nodiscard]] constexpr auto operator==(const Card* other) const noexcept -> bool
		{
			return card_ == other;
		}

		[[nodiscard]] constexpr auto operator==(const CardOptional& other) const noexcept -> bool
		{
			return *this == other.card_;
		}

		// todo
	};
}

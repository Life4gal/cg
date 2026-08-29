#pragma once

#include <utility>

namespace cg::utility
{
	enum class EnumSupport : std::uint8_t
	{
		// ==, !=
		EQUALITY,
		// >, >=, <, <=
		COMPARISON,
		// +, +=, -, -=
		ARITHMETIC,
		// &, &=, |, |=, ^, ^=, ~
		BIT_OPERATION,
		// <<, <<=, >>, >>=
		BIT_SHIFT,
		// implicit -> underlying
		IMPLICIT_UNDERLYING,
	};

	template<typename EnumType, EnumSupport... Supports>
		requires std::is_scoped_enum_v<EnumType>
	class Enum
	{
	public:
		using enum_type = EnumType;
		using value_type = std::underlying_type_t<enum_type>;

		constexpr static std::size_t support_count = sizeof...(Supports);
		constexpr static EnumSupport supports[support_count]{Supports...};

		template<EnumSupport Support>
		[[nodiscard]] constexpr static auto supported() noexcept -> bool
		{
			return []<std::size_t... Index>(const std::index_sequence<Index...>) constexpr -> bool
			{
				return ((supports[Index] == Support) || ...);
			}(std::make_index_sequence<support_count>{});
		}

	private:
		// alias: using my_enum_type = Enum<EnumType, Supports...>;
		// inherit: class my_enum_type : public Enum<EnumType, Supports...> {};
		template<typename T>
		constexpr static auto is_alias_or_inherit = std::is_same_v<T, Enum> or std::is_base_of_v<Enum, T>;

		enum_type enum_;

	public:
		constexpr explicit(false) Enum(const enum_type e) noexcept
			: enum_{e} {}

		// ==============================================================================
		// EnumSupport::EQUALITY
		// ==============================================================================

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::EQUALITY>())
		[[nodiscard]] constexpr auto operator==(const E& other) const noexcept -> bool
		{
			return enum_ == other;
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::EQUALITY>())
		[[nodiscard]] constexpr auto operator==(const E& other) const noexcept -> bool
		{
			return *this == other.enum_;
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::EQUALITY>())
		[[nodiscard]] constexpr auto operator!=(const E& other) const noexcept -> bool
		{
			return !(*this == other);
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::EQUALITY>())
		[[nodiscard]] constexpr auto operator!=(const E& other) const noexcept -> bool
		{
			return !(*this == other);
		}

		// ==============================================================================
		// EnumSupport::COMPARISON
		// ==============================================================================

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::COMPARISON>())
		[[nodiscard]] constexpr auto operator>(const E& other) const noexcept -> bool
		{
			return std::to_underlying(enum_) > std::to_underlying(other);
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::COMPARISON>())
		[[nodiscard]] constexpr auto operator>(const E& other) const noexcept -> bool
		{
			return *this > other.enum_;
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::COMPARISON>())
		[[nodiscard]] constexpr auto operator>=(const E& other) const noexcept -> bool
		{
			return std::to_underlying(enum_) >= std::to_underlying(other);
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::COMPARISON>())
		[[nodiscard]] constexpr auto operator>=(const E& other) const noexcept -> bool
		{
			return *this >= other.enum_;
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::COMPARISON>())
		[[nodiscard]] constexpr auto operator<(const E& other) const noexcept -> bool
		{
			return std::to_underlying(enum_) < std::to_underlying(other);
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::COMPARISON>())
		[[nodiscard]] constexpr auto operator<(const E& other) const noexcept -> bool
		{
			return *this < other.enum_;
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::COMPARISON>())
		[[nodiscard]] constexpr auto operator<=(const E& other) const noexcept -> bool
		{
			return std::to_underlying(enum_) <= std::to_underlying(other);
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::COMPARISON>())
		[[nodiscard]] constexpr auto operator<=(const E& other) const noexcept -> bool
		{
			return *this <= other.enum_;
		}

		// ==============================================================================
		// EnumSupport::ARITHMETIC
		// ==============================================================================

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::ARITHMETIC>())
		[[nodiscard]] constexpr auto operator+(const E& other) const noexcept -> Enum
		{
			const auto value = std::to_underlying(enum_) + std::to_underlying(other);

			return Enum{static_cast<enum_type>(value)};
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::ARITHMETIC>())
		[[nodiscard]] constexpr auto operator+(const E& other) const noexcept -> Enum
		{
			return *this + other.enum_;
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::ARITHMETIC>())
		constexpr auto operator+=(const E& other) noexcept -> Enum&
		{
			enum_ = static_cast<enum_type>(std::to_underlying(enum_) + std::to_underlying(other));

			return *this;
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::ARITHMETIC>())
		constexpr auto operator+=(const E& other) noexcept -> Enum&
		{
			return *this += other.enum_;
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::ARITHMETIC>())
		[[nodiscard]] constexpr auto operator-(const E& other) const noexcept -> Enum
		{
			const auto value = std::to_underlying(enum_) - std::to_underlying(other);

			return Enum{static_cast<enum_type>(value)};
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::ARITHMETIC>())
		[[nodiscard]] constexpr auto operator-(const E& other) const noexcept -> Enum
		{
			return *this - other.enum_;
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::ARITHMETIC>())
		constexpr auto operator-=(const E& other) noexcept -> Enum&
		{
			enum_ = static_cast<enum_type>(std::to_underlying(enum_) - std::to_underlying(other));

			return *this;
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::ARITHMETIC>())
		constexpr auto operator-=(const E& other) noexcept -> Enum&
		{
			return *this -= other.enum_;
		}

		// ==============================================================================
		// EnumSupport::BIT_OPERATION
		// ==============================================================================

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::BIT_OPERATION>())
		[[nodiscard]] constexpr auto operator&(const E& other) const noexcept -> Enum
		{
			const auto value = std::to_underlying(enum_) & std::to_underlying(other);

			return Enum{static_cast<enum_type>(value)};
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::BIT_OPERATION>())
		[[nodiscard]] constexpr auto operator&(const E& other) const noexcept -> Enum
		{
			return *this & other.enum_;
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::BIT_OPERATION>())
		constexpr auto operator&=(const E& other) noexcept -> Enum&
		{
			enum_ = static_cast<enum_type>(std::to_underlying(enum_) & std::to_underlying(other));

			return *this;
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::BIT_OPERATION>())
		constexpr auto operator&=(const E& other) noexcept -> Enum&
		{
			return *this &= other.enum_;
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::BIT_OPERATION>())
		[[nodiscard]] constexpr auto operator|(const E& other) const noexcept -> Enum
		{
			const auto value = std::to_underlying(enum_) | std::to_underlying(other);

			return Enum{static_cast<enum_type>(value)};
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::BIT_OPERATION>())
		[[nodiscard]] constexpr auto operator|(const E& other) const noexcept -> Enum
		{
			return *this | other.enum_;
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::BIT_OPERATION>())
		constexpr auto operator|=(const E& other) noexcept -> Enum&
		{
			enum_ = static_cast<enum_type>(std::to_underlying(enum_) | std::to_underlying(other));

			return *this;
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::BIT_OPERATION>())
		constexpr auto operator|=(const E& other) noexcept -> Enum&
		{
			return *this |= other.enum_;
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::BIT_OPERATION>())
		[[nodiscard]] constexpr auto operator^(const E& other) const noexcept -> Enum
		{
			const auto value = std::to_underlying(enum_) ^ std::to_underlying(other);

			return Enum{static_cast<enum_type>(value)};
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::BIT_OPERATION>())
		[[nodiscard]] constexpr auto operator^(const E& other) const noexcept -> Enum
		{
			return *this ^ other.enum_;
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::BIT_OPERATION>())
		constexpr auto operator^=(const E& other) noexcept -> Enum&
		{
			enum_ = static_cast<enum_type>(std::to_underlying(enum_) ^ std::to_underlying(other));

			return *this;
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::BIT_OPERATION>())
		constexpr auto operator^=(const E& other) noexcept -> Enum&
		{
			return *this ^= other.enum_;
		}

		[[nodiscard]] constexpr auto operator~() const noexcept -> Enum //
			requires(supported<EnumSupport::BIT_OPERATION>())
		{
			const auto value = ~std::to_underlying(enum_);

			return Enum{static_cast<enum_type>(value)};
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::BIT_OPERATION>())
		[[nodiscard]] constexpr auto all_of(const E& other) const noexcept -> bool
		{
			return (std::to_underlying(enum_) & std::to_underlying(other)) == std::to_underlying(other);
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::BIT_OPERATION>())
		[[nodiscard]] constexpr auto all_of(const E& other) const noexcept -> bool
		{
			return this->all_of(other.enum_);
		}

		template<std::same_as<enum_type> E>
			requires(supported<EnumSupport::BIT_OPERATION>())
		[[nodiscard]] constexpr auto any_of(const E& other) const noexcept -> bool
		{
			return (std::to_underlying(enum_) & std::to_underlying(other)) != value_type{0};
		}

		template<typename E>
			requires(is_alias_or_inherit<E> && supported<EnumSupport::BIT_OPERATION>())
		[[nodiscard]] constexpr auto any_of(const E& other) const noexcept -> bool
		{
			return this->any_of(other.enum_);
		}

		// ==============================================================================
		// EnumSupport::BIT_SHIFT
		// ==============================================================================

		[[nodiscard]] constexpr auto operator<<(const std::size_t shift) const noexcept -> Enum //
			requires(supported<EnumSupport::BIT_SHIFT>())
		{
			const auto value = std::to_underlying(enum_) << shift;

			return Enum{static_cast<enum_type>(value)};
		}

		constexpr auto operator<<=(const std::size_t shift) noexcept -> Enum& //
			requires(supported<EnumSupport::BIT_SHIFT>())
		{
			enum_ = static_cast<enum_type>(std::to_underlying(enum_) << shift);

			return *this;
		}

		[[nodiscard]] constexpr auto operator>>(const std::size_t shift) const noexcept -> Enum //
			requires(supported<EnumSupport::BIT_SHIFT>())
		{
			const auto value = std::to_underlying(enum_) >> shift;

			return Enum{static_cast<enum_type>(value)};
		}

		constexpr auto operator>>=(const std::size_t shift) noexcept -> Enum& //
			requires(supported<EnumSupport::BIT_SHIFT>())
		{
			enum_ = static_cast<enum_type>(std::to_underlying(enum_) >> shift);

			return *this;
		}

		// ==============================================================================
		// EnumSupport::IMPLICIT_UNDERLYING
		// ==============================================================================

		[[nodiscard]] constexpr explicit(!supported<EnumSupport::IMPLICIT_UNDERLYING>()) operator value_type() const noexcept
		{
			return std::to_underlying(enum_);
		}

		// ==============================================================================
		// static_cast<T>(Enum)
		// ==============================================================================

		template<std::integral T>
			requires(!std::is_same_v<T, value_type>)
		[[nodiscard]] constexpr explicit operator T() const noexcept
		{
			return static_cast<T>(std::to_underlying(enum_));
		}
	};
}

#pragma once

#include <variant>
#include <vector>

#include <cg/state/id.hpp>

namespace cg::execution
{
	class Void {};

	class Boolean
	{
	public:
		// dsl::Literal::boolean_type
		using value_type = bool;

	private:
		value_type value_;

	public:
		constexpr explicit Boolean(const value_type value) noexcept
			: value_{value} {}

		[[nodiscard]] constexpr explicit(false) operator value_type() const noexcept
		{
			return value_;
		}

		[[nodiscard]] constexpr auto operator==(const Boolean& other) const noexcept -> bool
		{
			return value_ == other.value_;
		}
	};

	class Integer
	{
	public:
		// dsl::Literal::number_type
		// dsl::Literal::Symbol
		using value_type = std::int64_t;

	private:
		value_type value_;

	public:
		constexpr explicit Integer(const value_type value) noexcept
			: value_{value} {}

		[[nodiscard]] constexpr explicit(false) operator value_type() const noexcept
		{
			return value_;
		}

		[[nodiscard]] constexpr auto operator==(const Integer& other) const noexcept -> bool
		{
			return value_ == other.value_;
		}
	};

	class Card
	{
	public:
		using value_type = state::CardInstanceId;

	private:
		value_type value_;

	public:
		constexpr explicit Card(const value_type value) noexcept
			: value_{value} {}

		[[nodiscard]] constexpr explicit(false) operator value_type() const noexcept
		{
			return value_;
		}

		[[nodiscard]] constexpr auto operator==(const Card& other) const noexcept -> bool
		{
			return value_ == other.value_;
		}
	};

	class Group
	{
	public:
		using value_type = std::vector<state::CardInstanceId>;

		using element_type = value_type::value_type;
		using size_type = value_type::size_type;
		using iterator = value_type::iterator;
		using const_iterator = value_type::const_iterator;

	private:
		value_type value_;

	public:
		constexpr Group() noexcept = default;

		// template<typename Range>
		// 	requires std::is_constructible_v<value_type, Range>
		// constexpr explicit Group(Range&& range) noexcept
		// 	: value_{std::forward<Range>(range)} {}

		constexpr explicit Group(value_type value) noexcept
			: value_{std::move(value)} {}

		template<typename Iterator>
			requires std::is_constructible_v<value_type, Iterator, Iterator>
		constexpr Group(Iterator begin, Iterator end) noexcept
			: value_{std::move(begin), std::move(end)} {}

		[[nodiscard]] constexpr auto empty() const noexcept -> bool
		{
			return value_.empty();
		}

		[[nodiscard]] constexpr auto size() const noexcept -> size_type
		{
			return value_.size();
		}

		[[nodiscard]] constexpr auto begin() noexcept -> iterator
		{
			return value_.begin();
		}

		[[nodiscard]] constexpr auto begin() const noexcept -> const_iterator
		{
			return value_.begin();
		}

		[[nodiscard]] constexpr auto end() noexcept -> iterator
		{
			return value_.end();
		}

		[[nodiscard]] constexpr auto end() const noexcept -> const_iterator
		{
			return value_.end();
		}

		constexpr auto reserve(const size_type capacity) noexcept -> void
		{
			value_.reserve(capacity);
		}

		constexpr auto push_back(const element_type card) noexcept -> void
		{
			value_.push_back(card);
		}

		template<typename Range>
		constexpr auto append_range(Range&& range) noexcept -> void //
			requires requires { value_.append_range(std::forward<Range>(range)); }
		{
			value_.append_range(std::forward<Range>(range));
		}

		[[nodiscard]] constexpr explicit(false) operator value_type&() noexcept
		{
			return value_;
		}

		[[nodiscard]] constexpr explicit(false) operator const value_type&() const noexcept
		{
			return value_;
		}

		[[nodiscard]] constexpr auto operator==(const Group& other) const noexcept -> bool
		{
			return value_ == other.value_;
		}
	};

	class EnumMask
	{
	public:
		// dsl::TypeRegistry::symbol_underlying_type
		using value_type = std::uint64_t;

	private:
		value_type value_;

	public:
		constexpr explicit EnumMask(const value_type value) noexcept
			: value_{value} {}

		[[nodiscard]] constexpr explicit(false) operator value_type() const noexcept
		{
			return value_;
		}

		[[nodiscard]] constexpr auto operator==(const EnumMask& other) const noexcept -> bool
		{
			return value_ == other.value_;
		}
	};

	// 表达式求值结果
	class Value : public std::variant<
				// dsl::ValueClass::VOID
				Void,
				// dsl::ValueClass::BOOL
				Boolean,
				// dsl::ValueClass::INT
				Integer,
				// dsl::ValueClass::CARD
				Card,
				// dsl::ValueClass::GROUP
				Group,
				// dsl::ValueClass::ENUM_SET
				EnumMask
			>
	{
	public:
		using variant::variant;
	};
}

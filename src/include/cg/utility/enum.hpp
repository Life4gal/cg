#pragma once

#include <bit>
#include <utility>

#include <magic_enum/magic_enum_flags.hpp>

namespace cg::utility
{
	namespace enum_detail
	{
		template<std::size_t N>
		[[nodiscard]] consteval auto select_type_tag() noexcept -> auto
		{
			if constexpr (N < 8)
			{
				return std::uint8_t{};
			}
			else if constexpr (N < 16)
			{
				return std::uint16_t{};
			}
			else if constexpr (N < 32)
			{
				return std::uint32_t{};
			}
			else if constexpr (N < 64)
			{
				return std::uint64_t{};
			}
			else
			{
				std::unreachable();
			}
		}

		template<std::size_t N>
		struct select_type
		{
			using type = std::decay_t<decltype(select_type_tag<N>())>;
		};

		template<std::size_t N>
		using select_type_t = typename select_type<N>::type;
	}

	template<typename EnumType>
		requires std::is_scoped_enum_v<EnumType>
	class EnumMask
	{
		constexpr static auto values_are_contiguous() noexcept -> bool
		{
			std::underlying_type_t<EnumType> i = 0;
			for (const auto value: magic_enum::enum_values<EnumType>())
			{
				if (std::to_underlying(value) != i)
				{
					return false;
				}
				i += 1;
			}
			return true;
		}

		static_assert(values_are_contiguous());

	public:
		using enum_type = EnumType;

		constexpr static auto enums_count = magic_enum::enum_count<enum_type>();
		static_assert(enums_count > 0);

		// 枚举值数量:
		//  - 1~7: uint8_t
		//  - 8~15: uint16_t
		//  - 16~31: uint32_t
		//  - 32~63: uint64_t
		using value_type = enum_detail::select_type_t<enums_count>;

		// 所有枚举值皆有时的值 -- 不支持空位
		constexpr static auto all_enums_value = static_cast<value_type>((static_cast<std::uint64_t>(1) << enums_count) - 1);

		// 默认值 - 0
		constexpr static auto default_value = value_type{0};

	private:
		value_type bits_;

		[[nodiscard]] constexpr static auto bit_of(const enum_type e) noexcept -> value_type
		{
			return static_cast<value_type>(value_type{1} << std::to_underlying(e));
		}

	public:
		constexpr EnumMask() noexcept
			: bits_{default_value} {}

		constexpr explicit EnumMask(const enum_type e) noexcept
			: EnumMask{}
		{
			EnumMask::add_with(e);
		}

		constexpr explicit EnumMask(const value_type value) noexcept
			: bits_{static_cast<value_type>(value & all_enums_value)} {}

		// template<std::same_as<enum_type>... Es>
		// constexpr explicit(false) EnumMask(Es... es) noexcept
		// 	: EnumMask{}
		// {
		// 	(EnumMask::add(es), ...);
		// }

		constexpr explicit(false) EnumMask(const std::initializer_list<enum_type> list) noexcept
			: EnumMask{}
		{
			for (const auto e: list)
			{
				EnumMask::add_with(e);
			}
		}

		// ===================================================================
		// 查询
		// ===================================================================

		[[nodiscard]] constexpr auto contains(const enum_type e) const noexcept -> bool
		{
			return (bits_ & EnumMask::bit_of(e)) != value_type{0};
		}

		[[nodiscard]] constexpr auto empty() const noexcept -> bool
		{
			return bits_ == default_value;
		}

		[[nodiscard]] constexpr auto size() const noexcept -> std::size_t
		{
			return std::popcount(bits_);
		}

		[[nodiscard]] constexpr auto value() const noexcept -> value_type
		{
			return bits_;
		}

		// ===================================================================
		// 位修改
		// ===================================================================

		constexpr auto add_with(const enum_type e) noexcept -> EnumMask&
		{
			bits_ |= EnumMask::bit_of(e);
			return *this;
		}

		[[nodiscard]] constexpr auto add(const enum_type e) const noexcept -> EnumMask
		{
			auto t = *this;
			t.add_with(e);
			return t;
		}

		constexpr auto remove_with(const enum_type e) noexcept -> EnumMask&
		{
			bits_ &= ~EnumMask::bit_of(e);
			return *this;
		}

		[[nodiscard]] constexpr auto remove(const enum_type e) const noexcept -> EnumMask
		{
			auto t = *this;
			t.remove_with(e);
			return t;
		}

		constexpr auto toggle_with(const enum_type e) noexcept -> EnumMask&
		{
			bits_ ^= EnumMask::bit_of(e);
			return *this;
		}

		[[nodiscard]] constexpr auto toggle(const enum_type e) const noexcept -> EnumMask
		{
			auto t = *this;
			t.toggle_with(e);
			return t;
		}

		constexpr auto clear() noexcept -> void
		{
			bits_ = default_value;
		}

		// ===================================================================
		// 修改
		// ===================================================================

		// |=
		constexpr auto unite_with(const EnumMask& other) noexcept -> EnumMask&
		{
			bits_ |= other.bits_;
			return *this;
		}

		// |
		[[nodiscard]] constexpr auto unite(const EnumMask& other) const noexcept -> EnumMask
		{
			auto t = *this;
			t.unite_with(other);
			return t;
		}

		// &=
		constexpr auto intersect_with(const EnumMask& other) noexcept -> EnumMask&
		{
			bits_ &= other.bits_;
			return *this;
		}

		// &=
		[[nodiscard]] constexpr auto intersect(const EnumMask& other) const noexcept -> EnumMask
		{
			auto t = *this;
			t.intersect_with(other);
			return t;
		}

		// 差集(~=)
		constexpr auto subtract_with(const EnumMask& other) noexcept -> EnumMask&
		{
			bits_ &= static_cast<value_type>(~other.bits_);
			return *this;
		}

		// 差集(~)
		[[nodiscard]] constexpr auto subtract(const EnumMask& other) const noexcept -> EnumMask
		{
			auto t = *this;
			t.subtract_with(other);
			return t;
		}

		// 对称差(^=)
		constexpr auto symmetric_difference_with(const EnumMask& other) noexcept -> EnumMask&
		{
			bits_ ^= other.bits_;
			return *this;
		}

		// 对称差(^)
		[[nodiscard]] constexpr auto symmetric_difference(const EnumMask& other) const noexcept -> EnumMask
		{
			auto t = *this;
			t.symmetric_difference_with(other);
			return t;
		}

		// 补集(所有未被置位的枚举)
		[[nodiscard]] constexpr auto complement() const noexcept -> EnumMask
		{
			return {static_cast<value_type>(~bits_ & all_enums_value)};
		}

		// ===================================================================
		// 迭代器
		// ===================================================================

		class Iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using iterator_concept = std::forward_iterator_tag;
			using value_type = enum_type;
			using difference_type = std::ptrdiff_t;
			using pointer = void;
			using reference = enum_type;

		private:
			EnumMask::value_type mask_;

		public:
			constexpr Iterator() noexcept
				: mask_{default_value} {}

			constexpr explicit Iterator(const EnumMask::value_type mask) noexcept
				: mask_{mask} {}

			[[nodiscard]] constexpr auto operator*() const noexcept -> enum_type
			{
				return static_cast<enum_type>(std::countr_zero(mask_));
			}

			constexpr auto operator++() noexcept -> Iterator&
			{
				mask_ &= static_cast<EnumMask::value_type>(mask_ - 1);
				return *this;
			}

			constexpr auto operator++(int) noexcept -> Iterator
			{
				auto tmp = *this;
				++*this;
				return tmp;
			}

			[[nodiscard]] constexpr auto operator==(const Iterator&) const noexcept -> bool = default;
		};

		[[nodiscard]] constexpr auto begin() const noexcept -> Iterator
		{
			return Iterator{bits_};
		}

		[[nodiscard]] constexpr auto end() const noexcept -> Iterator
		{
			return Iterator{};
		}

		[[nodiscard]] constexpr auto operator==(const EnumMask& other) const noexcept -> bool = default;
	};

	template<typename EnumType>
		requires std::is_enum_v<EnumType>
	[[nodiscard]] constexpr auto to_string(const EnumType e) noexcept -> std::string_view
	{
		return magic_enum::enum_name(e);
	}

	template<typename EnumType>
		requires std::is_enum_v<EnumType>
	[[nodiscard]] constexpr auto to_flags_string(const EnumType e) noexcept -> std::string
	{
		return magic_enum::enum_flags_name(e);
	}

	template<typename EnumType>
	[[nodiscard]] constexpr auto to_string(const EnumMask<EnumType>& mask) noexcept -> std::string
	{
		std::string name;
		for (const auto e: mask)
		{
			if (!name.empty())
			{
				name.append(1, '|');
			}

			name.append_range(utility::to_string(e));
		}

		return name;
	}

	template<typename EnumType>
		requires std::is_enum_v<EnumType>
	[[nodiscard]] constexpr auto from_string(const std::string_view e) noexcept -> std::optional<EnumType>
	{
		return magic_enum::enum_cast<EnumType>(e);
	}

	template<typename EnumType>
		requires std::is_enum_v<EnumType>
	[[nodiscard]] constexpr auto from_flags_string(const std::string_view e) noexcept -> std::optional<EnumType>
	{
		return magic_enum::enum_flags_cast<EnumType>(e);
	}

	template<typename EnumType>
	[[nodiscard]] constexpr auto from_mask_string(std::string_view mask) noexcept -> std::optional<EnumMask<EnumType>>
	{
		const auto count = magic_enum::enum_count<EnumType>();

		EnumMask<EnumType> result{};
		while (!mask.empty())
		{
			const auto d = mask.find('|');
			const auto s = (d == std::string_view::npos) ? mask : mask.substr(0, d);
			bool found = false;
			for (std::size_t i = 0; i < count; ++i)
			{
				if (const auto e = static_cast<EnumType>(i);
					s == magic_enum::enum_name(e))
				{
					result.add_with(e);
					found = true;
					break;
				}
			}

			if (!found)
			{
				return std::nullopt;
			}
			if (d == std::string_view::npos)
			{
				mask.remove_prefix(mask.size());
			}
			else
			{
				mask.remove_prefix(d + 1);
			}
		}

		return result;
	}
}

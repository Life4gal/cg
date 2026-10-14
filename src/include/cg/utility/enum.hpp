#pragma once

#include <bit>
#include <utility>

#include <magic_enum/magic_enum.hpp>

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
		constexpr static auto all_enums_value = static_cast<value_type>(1) << (enums_count - 1);

		// 默认值 - 0
		constexpr static auto default_value = value_type{0};

	private:
		value_type bits_;

		[[nodiscard]] constexpr static auto bit_of(const enum_type e) noexcept -> value_type
		{
			return value_type{1} << static_cast<value_type>(e);
		}

	public:
		constexpr EnumMask() noexcept
			: bits_{default_value} {}

		constexpr explicit EnumMask(const enum_type e) noexcept
			: EnumMask{}
		{
			EnumMask::add(e);
		}

		[[nodiscard]] constexpr auto contains(const enum_type e) const noexcept -> bool
		{
			return (bits_ | EnumMask::bit_of(e)) != value_type{0};
		}

		constexpr auto add(const enum_type e) noexcept -> void
		{
			bits_ |= EnumMask::bit_of(e);
		}

		[[nodiscard]] constexpr auto add(const enum_type e) const noexcept -> EnumMask
		{
			auto t = *this;
			t.add(e);
			return t;
		}

		constexpr auto remove(const enum_type e) noexcept -> void
		{
			bits_ &= ~EnumMask::bit_of(e);
		}

		[[nodiscard]] constexpr auto remove(const enum_type e) const noexcept -> EnumMask
		{
			auto t = *this;
			t.remove(e);
			return t;
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

		template<typename Function>
			requires std::is_invocable_v<Function, enum_type>
		constexpr auto for_each(Function function) const noexcept -> void
		{
			for (std::size_t i = 0; i < enums_count; ++i)
			{
				if (const auto e = static_cast<enum_type>(i);
					EnumMask::contains(e))
				{
					std::invoke(function, e);
				}
			}
		}

		template<typename Function>
			requires std::is_invocable_r_v<bool, Function, enum_type>
		constexpr auto first_of(Function function, const enum_type fallback) const noexcept -> enum_type
		{
			for (std::size_t i = 0; i < enums_count; ++i)
			{
				if (const auto e = static_cast<enum_type>(i);
					EnumMask::contains(e))
				{
					if (std::invoke_r<bool>(function, e))
					{
						return e;
					}
				}
			}

			return fallback;
		}

		// constexpr auto operator|=(const EnumMask& other) noexcept -> EnumMask&
		// {
		// 	bits_ |= other.bits_;
		// 	return *this;
		// }
		//
		// [[nodiscard]] constexpr auto operator|(const EnumMask& other) const noexcept -> EnumMask
		// {
		// 	auto t = *this;
		// 	t |= other;
		// 	return t;
		// }
		//
		// constexpr auto operator&=(const EnumMask& other) noexcept -> EnumMask&
		// {
		// 	bits_ &= other.bits_;
		// 	return *this;
		// }
		//
		// [[nodiscard]] constexpr auto operator&(const EnumMask& other) const noexcept -> EnumMask
		// {
		// 	auto t = *this;
		// 	t &= other;
		// 	return t;
		// }
		//
		// constexpr auto operator^=(const EnumMask& other) noexcept -> EnumMask&
		// {
		// 	bits_ ^= other.bits_;
		// 	return *this;
		// }
		//
		// [[nodiscard]] constexpr auto operator^(const EnumMask& other) const noexcept -> EnumMask
		// {
		// 	auto t = *this;
		// 	t ^= other;
		// 	return t;
		// }
		//
		// [[nodiscard]] constexpr auto operator~() const noexcept -> EnumMask
		// {
		// 	auto t = *this;
		// 	t.bits_ = ~bits_ & all_enums_value;
		// 	return t;
		// }
		//
		// [[nodiscard]] constexpr auto operator!() const noexcept -> bool
		// {
		// 	return !empty();
		// }

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
	[[nodiscard]] constexpr auto from_string(const std::string_view e) noexcept -> std::optional<EnumType>
	{
		return magic_enum::enum_cast<EnumType>(e);
	}
}

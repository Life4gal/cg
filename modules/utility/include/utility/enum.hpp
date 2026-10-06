#pragma once

#include <utility>

namespace cg::utility
{
	template<typename>
	struct is_flag : std::false_type {};

	// template<typename EnumType>
	// 	requires std::is_scoped_enum_v<EnumType>
	// struct is_flag<EnumType> : std::true_type {};

	template<typename EnumType>
	constexpr auto is_flag_v = is_flag<EnumType>::value;

	template<typename EnumType>
	concept flag_t = is_flag_v<EnumType>;
}

// =============================================================================
// operator|

// flag | value => flag
template<cg::utility::flag_t EnumType, std::integral ValueType>
[[nodiscard]] constexpr auto operator|(const EnumType lhs, const ValueType rhs) noexcept -> EnumType //
	requires requires { std::to_underlying(lhs) | rhs; }
{
	return static_cast<EnumType>(std::to_underlying(lhs) | rhs);
}

// value | flag => value
template<cg::utility::flag_t EnumType, std::integral ValueType>
[[nodiscard]] constexpr auto operator|(const ValueType lhs, const EnumType rhs) noexcept -> ValueType //
	requires requires { lhs | std::to_underlying(rhs); }
{
	return static_cast<ValueType>(lhs | std::to_underlying(rhs));
}

// flag | flag => bool
template<cg::utility::flag_t EnumType>
[[nodiscard]] constexpr auto operator|(const EnumType lhs, const EnumType rhs) noexcept -> bool //
	requires requires { std::to_underlying(lhs) | std::to_underlying(rhs); }
{
	return std::to_underlying(lhs) | std::to_underlying(rhs);
}

// flag |= value => flag
template<cg::utility::flag_t EnumType, std::integral ValueType>
constexpr auto operator|=(EnumType& lhs, const ValueType rhs) noexcept -> EnumType& //
	requires requires { lhs | rhs; }
{
	lhs = lhs | rhs;
	return lhs;
}

// value |= flag => value
template<cg::utility::flag_t EnumType, std::integral ValueType>
constexpr auto operator|=(ValueType& lhs, const EnumType rhs) noexcept -> ValueType& //
	requires requires { lhs | rhs; }
{
	lhs = lhs | rhs;
	return lhs;
}

// flag |= flag => flag
template<cg::utility::flag_t EnumType>
constexpr auto operator|=(EnumType& lhs, const EnumType rhs) noexcept -> EnumType& //
	requires requires { lhs | rhs; }
{
	lhs = lhs | rhs;
	return lhs;
}

// =============================================================================
// operator&

// flag & value => flag
template<cg::utility::flag_t EnumType, std::integral ValueType>
[[nodiscard]] constexpr auto operator&(const EnumType lhs, const ValueType rhs) noexcept -> EnumType //
	requires requires { std::to_underlying(lhs) & rhs; }
{
	return static_cast<EnumType>(std::to_underlying(lhs) & rhs);
}

// value & flag => value
template<cg::utility::flag_t EnumType, std::integral ValueType>
[[nodiscard]] constexpr auto operator&(const ValueType lhs, const EnumType rhs) noexcept -> ValueType //
	requires requires { lhs & std::to_underlying(rhs); }
{
	return static_cast<ValueType>(lhs & std::to_underlying(rhs));
}

// flag & flag => bool
template<cg::utility::flag_t EnumType>
[[nodiscard]] constexpr auto operator&(const EnumType lhs, const EnumType rhs) noexcept -> bool //
	requires requires { std::to_underlying(lhs) & std::to_underlying(rhs); }
{
	return std::to_underlying(lhs) & std::to_underlying(rhs);
}

// flag &= value => flag
template<cg::utility::flag_t EnumType, std::integral ValueType>
constexpr auto operator&=(EnumType& lhs, const ValueType rhs) noexcept -> EnumType& //
	requires requires { lhs & rhs; }
{
	lhs = lhs & rhs;
	return lhs;
}

// value &= flag => value
template<cg::utility::flag_t EnumType, std::integral ValueType>
constexpr auto operator&=(ValueType& lhs, const EnumType rhs) noexcept -> ValueType& //
	requires requires { lhs & rhs; }
{
	lhs = lhs & rhs;
	return lhs;
}

// flag &= flag => flag
template<cg::utility::flag_t EnumType>
constexpr auto operator&=(EnumType& lhs, const EnumType rhs) noexcept -> EnumType& //
	requires requires { lhs & rhs; }
{
	lhs = lhs & rhs;
	return lhs;
}

// =============================================================================
// operator^

// flag ^ value => flag
template<cg::utility::flag_t EnumType, std::integral ValueType>
[[nodiscard]] constexpr auto operator^(const EnumType lhs, const ValueType rhs) noexcept -> EnumType //
	requires requires { std::to_underlying(lhs) ^ rhs; }
{
	return static_cast<EnumType>(std::to_underlying(lhs) ^ rhs);
}

// value ^ flag => value
template<cg::utility::flag_t EnumType, std::integral ValueType>
[[nodiscard]] constexpr auto operator^(const ValueType lhs, const EnumType rhs) noexcept -> ValueType //
	requires requires { lhs ^ std::to_underlying(rhs); }
{
	return static_cast<ValueType>(lhs ^ std::to_underlying(rhs));
}

// flag ^ flag => bool
template<cg::utility::flag_t EnumType>
[[nodiscard]] constexpr auto operator^(const EnumType lhs, const EnumType rhs) noexcept -> bool //
	requires requires { std::to_underlying(lhs) ^ std::to_underlying(rhs); }
{
	return std::to_underlying(lhs) ^ std::to_underlying(rhs);
}

// flag ^= value => flag
template<cg::utility::flag_t EnumType, std::integral ValueType>
constexpr auto operator^=(EnumType& lhs, const ValueType rhs) noexcept -> EnumType& //
	requires requires { lhs ^ rhs; }
{
	lhs = lhs ^ rhs;
	return lhs;
}

// value ^= flag => value
template<cg::utility::flag_t EnumType, std::integral ValueType>
constexpr auto operator^=(ValueType& lhs, const EnumType rhs) noexcept -> ValueType& //
	requires requires { lhs ^ rhs; }
{
	lhs = lhs ^ rhs;
	return lhs;
}

// flag ^= flag => flag
template<cg::utility::flag_t EnumType>
constexpr auto operator^=(EnumType& lhs, const EnumType rhs) noexcept -> EnumType& //
	requires requires { lhs ^ rhs; }
{
	lhs = lhs ^ rhs;
	return lhs;
}

// =============================================================================
// operator~

template<cg::utility::flag_t EnumType>
[[nodiscard]] constexpr auto operator~(const EnumType e) noexcept -> EnumType //
	requires requires { ~std::to_underlying(e); }
{
	return static_cast<EnumType>(~std::to_underlying(e));
}

// =============================================================================
// operator!

template<cg::utility::flag_t EnumType>
[[nodiscard]] constexpr auto operator!(const EnumType e) noexcept -> bool //
	requires requires { !std::to_underlying(e); }
{
	return !std::to_underlying(e);
}

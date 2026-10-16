#pragma once

#include <variant>

enum class T : std::uint8_t;

namespace cg::utility
{
	namespace variant_detail
	{
		template<typename... Ts>
		constexpr auto variant_probe(const std::variant<Ts...>&) noexcept -> std::true_type;

		constexpr auto variant_probe(...) noexcept -> std::false_type;

		template<typename... Ts>
		constexpr auto variant_size_probe(const std::variant<Ts...>&) noexcept -> std::integral_constant<std::size_t, sizeof...(Ts)>;

		constexpr auto variant_size_probe(...) noexcept -> std::integral_constant<std::size_t, 0>;

		template<std::size_t Index, typename... Ts>
		constexpr auto variant_type_probe(const std::variant<Ts...>&) noexcept -> std::variant_alternative<Index, std::variant<Ts...>>;

		template<std::size_t Index>
		constexpr auto variant_type_probe(...) noexcept -> std::void_t<>;
	}

	template<typename T>
	struct is_variant_or_like : decltype(variant_detail::variant_probe(std::declval<T&>())) {};

	template<typename T>
	constexpr auto is_variant_or_like_v = is_variant_or_like<T>::value;

	template<typename T>
	concept variant_or_like_t = is_variant_or_like_v<T>;

	template<typename>
	struct variant_size;

	template<variant_or_like_t T>
	struct variant_size<T> : decltype(variant_detail::variant_size_probe(std::declval<T&>())) {};

	template<typename T>
	constexpr auto variant_size_v = variant_size<T>::value;

	template<std::size_t Index, typename>
	struct variant_alternative;

	template<std::size_t Index, variant_or_like_t T>
	struct variant_alternative<Index, T> : decltype(variant_detail::variant_type_probe<Index>(std::declval<T&>())) {};

	template<std::size_t Index, typename T>
	using variant_alternative_t = typename variant_alternative<Index, T>::type;

	// class Foo1 : public std::variant<int> {};
	//
	// class Foo2 : public std::variant<int, float> {};
	//
	// static_assert(!is_variant_or_like_v<int>);
	// static_assert(is_variant_or_like_v<std::variant<int>>);
	// static_assert(is_variant_or_like_v<std::variant<int, float>>);
	// static_assert(is_variant_or_like_v<Foo1>);
	// static_assert(is_variant_or_like_v<Foo2>);
	//
	// static_assert(variant_size_v<std::variant<int>> == 1);
	// static_assert(variant_size_v<std::variant<int, float>> == 2);
	// static_assert(variant_size_v<Foo1> == 1);
	// static_assert(variant_size_v<Foo2> == 2);
	//
	// static_assert(std::is_same_v<variant_alternative_t<0, std::variant<int, float>>, int>);
	// static_assert(std::is_same_v<variant_alternative_t<1, std::variant<int, float>>, float>);
	// static_assert(std::is_same_v<variant_alternative_t<0, Foo2>, int>);
	// static_assert(std::is_same_v<variant_alternative_t<1, Foo2>, float>);
}

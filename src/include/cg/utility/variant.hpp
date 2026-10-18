#pragma once

#include <variant>

namespace cg::utility
{
	namespace variant_detail
	{
		template<typename... Ts>
		constexpr auto variant_probe(const std::variant<Ts...>&) noexcept -> std::true_type;

		constexpr auto variant_probe(...) noexcept -> std::false_type;
	}

	template<typename T>
	struct is_variant_or_like : decltype(variant_detail::variant_probe(std::declval<T&>())) {};

	template<typename T>
	constexpr auto is_variant_or_like_v = is_variant_or_like<T>::value;

	template<typename T>
	concept variant_or_like_t = is_variant_or_like_v<T>;

	namespace variant_detail
	{
		template<typename... Ts>
		constexpr auto variant_size_probe(const std::variant<Ts...>&) noexcept -> std::integral_constant<std::size_t, sizeof...(Ts)>;

		constexpr auto variant_size_probe(...) noexcept -> std::integral_constant<std::size_t, 0>;

		template<std::size_t Index, typename... Ts>
		constexpr auto variant_type_probe(const std::variant<Ts...>&) noexcept -> std::variant_alternative<Index, std::variant<Ts...>>;

		template<std::size_t Index>
		constexpr auto variant_type_probe(...) noexcept -> std::void_t<>;

		template<typename... Ts>
		constexpr auto variant_identity_probe(const std::variant<Ts...>&) noexcept -> std::variant<Ts...>;

		constexpr auto variant_identity_probe(...) noexcept -> void;

		template<variant_or_like_t T>
		using variant_identity = decltype(variant_detail::variant_identity_probe(std::declval<T&>()));

		constexpr auto invalid_variant_index = 0xffff'ffff'ffff'ffffuz;

		template<typename T, typename U, typename All>
		struct variant_index_probe;

		template<typename T, typename... Current, typename... All>
		struct variant_index_probe<T, std::variant<T, Current...>, std::variant<All...>> : std::integral_constant<std::size_t, sizeof...(All) - sizeof...(Current) - 1> {};

		template<typename T, typename First, typename... Current, typename... All>
		struct variant_index_probe<T, std::variant<First, Current...>, std::variant<All...>> : variant_index_probe<T, std::variant<Current...>, std::variant<All...>> {};

		template<typename T, typename... All>
		struct variant_index_probe<T, std::variant<>, std::variant<All...>> : std::integral_constant<std::size_t, invalid_variant_index> {};

		template<typename T, variant_or_like_t V>
		struct variant_index : variant_index_probe<T, variant_identity<V>, variant_identity<V>> {};
	}

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

	template<typename T, variant_or_like_t V>
	constexpr auto variant_index_v = variant_detail::variant_index<T, V>::value;

	using variant_detail::invalid_variant_index;

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
	//
	// static_assert(variant_index_v<int, std::variant<int, float>> == 0);
	// static_assert(variant_index_v<float, std::variant<int, float>> == 1);
	// static_assert(variant_index_v<float, std::variant<>> == invalid_variant_index);
	// static_assert(variant_index_v<int, Foo2> == 0);
	// static_assert(variant_index_v<float, Foo2> == 1);
}

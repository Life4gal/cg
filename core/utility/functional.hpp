#pragma once

#include <type_traits>

namespace cg::utility
{
	template<typename... Ts>
	class Overloaded : Ts...
	{
	public:
		// const auto visitor = Overloaded
		// {
		//		[](const T1&) { ... },
		//		[](const T2&) { ... },
		//		[](const T3&) { ... },
		// };
		constexpr explicit Overloaded(Ts&&... ts) noexcept((std::is_nothrow_constructible_v<Ts, decltype(ts)> and ...))
			: Ts{std::forward<Ts>(ts)}... {}

		// const auto lambda1 = [](const T1&) { ... };
		// const auto lambda2 = [](const T2&) { ... };
		// const auto lambda3 = [](const T3&) { ... };
		//
		// const auto visitor = Overloaded{ lambda1, lambda2, lambda3 };
		constexpr explicit Overloaded(const Ts&... ts) noexcept((std::is_nothrow_constructible_v<Ts, decltype(ts)> and ...))
			: Ts{ts}... {}

		using Ts::operator()...;
	};
}

#pragma once

#include <algorithm>
#include <ranges>

namespace cg::utility
{
	template<typename T>
	struct hash
	{
		[[nodiscard]] static auto operator()(const T& obj) noexcept -> std::size_t
		{
			return std::hash<T>::operator()(obj);
		}
	};

	// string/string_view/vector<char>
	template<std::ranges::contiguous_range String>
		requires std::is_same_v<typename String::value_type, char>
	struct hash<String>
	{
		using is_transparent = int;

		[[nodiscard]] constexpr static auto operator()(const std::ranges::viewable_range auto string) noexcept -> std::size_t
		{
			constexpr auto offset_basis = std::uint64_t{0xcbf29ce484222325ull};
			constexpr auto prime = std::uint64_t{0x100000001b3ull};

			auto result = offset_basis;
			std::ranges::for_each(
				string,
				[&result](const auto v) noexcept -> void
				{
					result ^= v;
					result *= prime;
				}
			);

			return result;
		}

		// template<typename S>
		// [[nodiscard]] constexpr static auto operator()(S&& string) noexcept -> std::size_t
		// {
		// 	return hash::operator()(std::forward<S>(string) | std::views::all);
		// }
	};

	[[nodiscard]] constexpr auto hash_combine_2(const std::size_t hash1, const std::size_t hash2) noexcept -> std::size_t
	{
		return hash1 + 0x9e3779b97f681800ull + (hash2 << 6) + (hash2 >> 2);
	}

	template<typename First, typename Second, typename... Reset>
	[[nodiscard]] constexpr auto hash_combine(First&& first, Second&& second, Reset&&... reset) noexcept -> std::size_t //
		requires requires
		{
			hash<std::remove_cvref_t<First>>{};
			hash<std::remove_cvref_t<Second>>{};
			(hash<std::remove_cvref_t<Reset>>{}, ...);
		}
	{
		if constexpr (sizeof...(Reset) == 0)
		{
			const auto hash1 = hash<std::remove_cvref_t<First>>(std::forward<First>(first));
			const auto hash2 = hash<std::remove_cvref_t<Second>>(std::forward<Second>(second));
			return utility::hash_combine_2(hash1, hash2);
		}
		else
		{
			const auto hash1 = hash<std::remove_cvref_t<First>>(std::forward<First>(first));
			const auto hash2 = hash_combine(std::forward<Second>(second), std::forward<Reset>(reset)...);
			return utility::hash_combine_2(hash1, hash2);
		}
	}
}

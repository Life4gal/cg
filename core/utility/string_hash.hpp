#pragma once

#include <string>

namespace cg::utility
{
	class StringHash
	{
	public:
		using is_transparent = int;

		[[nodiscard]] static auto operator()(const std::string& string) noexcept -> std::size_t
		{
			return std::hash<std::string>::operator()(string);
		}

		[[nodiscard]] static auto operator()(const std::string_view string) noexcept -> std::size_t
		{
			return std::hash<std::string_view>::operator()(string);
		}

		[[nodiscard]] static auto operator()(const char* string) noexcept -> std::size_t
		{
			return operator()(std::string_view{string});
		}
	};
}

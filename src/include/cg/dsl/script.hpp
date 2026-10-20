#pragma once

#include <string>
#include <vector>

namespace cg::dsl
{
	class Diagnostic
	{
	public:
		// 定位路径,如 $.effects[0].cost[1]
		std::string path;
		std::string message;

		[[nodiscard]] auto operator==(const Diagnostic&) const -> bool = default;
	};

	class Diagnostics : public std::vector<Diagnostic> {};
}

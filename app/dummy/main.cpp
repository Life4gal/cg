#include <print>

#include <cg/domain/timing.hpp>
#include <magic_enum/magic_enum.hpp>

auto main() noexcept -> int
{
	std::println("Hello World!");

	std::println("Timing: ");
	for (constexpr auto names = magic_enum::enum_names<cg::domain::Timing>();
	     const auto name: names)
	{
		std::println("\t- {}", name);
	}
}

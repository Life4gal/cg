#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#ifdef _WIN32
#include <Windows.h>
#endif

auto main(const int argc, char** argv) noexcept -> int
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	return doctest::Context(argc, argv).run();
}

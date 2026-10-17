#include <print>

#include <cg/server/net_server.hpp>
#include <cg/domain/win.hpp>

// TODO: ixwebsocket -> mbedTLS -> bcrypt.lib
#ifdef _WIN32
#pragma comment(lib, "bcrypt.lib")
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace cg;

auto main() noexcept -> int
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	std::println("{}", std::filesystem::current_path().string());

	server::NetOptions options
	{
			.cards_path = "cards",
			.decks_path = "decks",
			.web_path = "web/dist",
			.pics_path = "pics",
			.assets_path = "web/public/assets",
			.host = "127.0.0.1",
			.port = 5678,
	};

	server::NetServer server{std::move(options)};

	if (!server.initialize())
	{
		return 1;
	}

	return server.run();
}

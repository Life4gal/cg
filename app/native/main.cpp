#include <print>

#include <cg/server/native_server.hpp>
#include <cg/server/codec.hpp>

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

	server::NativeOptions options
	{
			.room =
			{
					.cards_path = "cards",
					.decks_path = "decks",
			},
	};

	server::NativeServer server{std::move(options)};

	if (!server.initialize())
	{
		return 1;
	}

	const auto drain = [](server::NativeServer::NativeClient& client) noexcept -> void
	{
		const auto seat = client.seat();
		const auto name = seat == domain::Player::FIRST ? "FIRST" : "SECOND";

		while (auto message = client.try_pop())
		{
			std::println("[{}]{}", name, server::ValueCodec<server::message::Message>::encode(*message).dump());
		}
	};

	const auto result = server.run();

	drain(server.client(domain::Player::FIRST));
	drain(server.client(domain::Player::SECOND));

	return result;
}

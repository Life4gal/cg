#include <core/engine/duel.hpp>

#include <filesystem>

#include <core/host/python.hpp>

#include <doctest/doctest.h>

namespace
{
	using namespace cg;

	[[nodiscard]] auto script_root() noexcept -> std::string
	{
		return (std::filesystem::current_path().parent_path() / "scripts").string();
	}
}

TEST_CASE("Python宿主")
{
	auto host = std::make_unique<host::PythonHost>();
	REQUIRE(host->initialize(script_root()));

	engine::Duel duel{};
	duel.bind_host(std::move(host));

	REQUIRE(duel.register_prototype(domain::CardCode{1}));

	duel.set_player_info(domain::Player::FIRST, 8000, 5, 1);
	duel.set_player_info(domain::Player::SECOND, 8000, 5, 1);

	duel.start();
}

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
	// 创建python宿主
	auto host = std::make_unique<host::PythonHost>();
	REQUIRE(host->initialize(script_root()));
	// 创建决斗并绑定
	engine::Duel duel{};
	duel.bind_host(std::move(host));

	// 注册卡牌
	REQUIRE(
		duel.register_card(
			domain::CardCode{1},
			domain::Player::FIRST,
			domain::Zone::Monster{.position = domain::Zone::MonsterPosition::MAIN_1, .form = domain::Zone::MonsterForm::FACE_UP_ATTACK}
		)
	);
	REQUIRE(
		duel.register_card(
			domain::CardCode{1},
			domain::Player::FIRST,
			domain::Zone::Monster{.position = domain::Zone::MonsterPosition::MAIN_2, .form = domain::Zone::MonsterForm::FACE_UP_ATTACK}
		)
	);
	REQUIRE(
		duel.register_card(
			domain::CardCode{1},
			domain::Player::SECOND,
			domain::Zone::Monster{.position = domain::Zone::MonsterPosition::MAIN_1, .form = domain::Zone::MonsterForm::FACE_UP_ATTACK}
		)
	);

	for (domain::Zone::size_type i = 0; i < 40; ++i)
	{
		REQUIRE(
			duel.register_card(
				domain::CardCode{1},
				domain::Player::FIRST,
				domain::Zone::Deck{.index = i}
			)
		);

		REQUIRE(
			duel.register_card(
				domain::CardCode{1},
				domain::Player::SECOND,
				domain::Zone::Deck{.index = i}
			)
		);
	}

	// 设置双方信息
	duel.set_player_info(domain::Player::FIRST, 8000, 5, 1);
	duel.set_player_info(domain::Player::SECOND, 8000, 5, 1);

	// 开始决斗
	duel.start();
}

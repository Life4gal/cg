#include <cg/driver/local_session.hpp>

#include <memory>
#include <vector>

#include <doctest/doctest.h>

namespace
{
	using namespace cg;

	[[nodiscard]] auto make_vanilla_config() -> flow::DuelConfig
	{
		flow::DuelDeck deck{};

		for (const auto code: {domain::CardCode{97000131}, domain::CardCode{97000132}, domain::CardCode{97000133}})
		{
			for (std::size_t count = 0; count < 3; ++count)
			{
				deck.main_deck.push_back(code);
			}
		}

		flow::DuelConfig config{};
		config.decks[std::to_underlying(domain::Player::FIRST)] = deck;
		config.decks[std::to_underlying(domain::Player::SECOND)] = deck;
		config.seed = 20261025;
		config.first_player = domain::Player::FIRST;
		config.opening_hand = 5;

		// 测试用最小卡组: 只验证本地会话流程
		config.deck_rules.main_min = 1;
		config.deck_rules.main_max = 60;
		config.deck_rules.extra_max = 0;
		config.deck_rules.side_max = 0;
		config.deck_rules.max_copies = 3;

		return config;
	}
}

TEST_CASE("driver::LocalDuelSession runs pass/phase/normal-summon through two controllers")
{
	auto first = std::make_unique<driver::ScriptedController>();
	auto second = std::make_unique<driver::ScriptedController>();

	auto* const first_controller = first.get();
	auto* const second_controller = second.get();

	driver::LocalDuelSession session{std::move(first), std::move(second)};

	const auto loaded = session.load_scripts(CG_TEST_CARDS_DIR);
	if (!loaded)
	{
		for (const auto& error: session.duel().take_script_errors())
		{
			MESSAGE(error);
		}
	}
	REQUIRE(loaded);

	const auto config = make_vanilla_config();
	REQUIRE(session.initialize(config));

	const auto initial_view = session.view(domain::Player::FIRST);
	REQUIRE(initial_view.self_side.hand.size() == 5);
	CHECK(initial_view.opponent_side.hand_count == 5);
	CHECK(initial_view.phase == domain::Phase::DRAW);
	CHECK(initial_view.first_turn);

	const auto monster = initial_view.self_side.hand.front();
	const auto* monster_instance = session.duel().game().find_instance(monster);
	REQUIRE(monster_instance != nullptr);
	const auto monster_code = initial_view.card_codes.find(monster.raw());
	REQUIRE(monster_code != initial_view.card_codes.end());
	CHECK(monster_code->second == monster_instance->origin_code());

	// FIRST: 过 -> (STANDBY 后)过 -> (MAIN_1)通常召唤 -> 过
	// SECOND: 过 -> 推进阶段 -> 过 -> 推进阶段 -> 过
	first_controller->set_actions(
		{
				driver::PassAction{},
				driver::PassAction{},
				driver::NormalSummonAction{.card = monster, .column = domain::FieldColumn::C1},
				driver::PassAction{},
		}
	);

	second_controller->set_actions(
		{
				driver::PassAction{},
				driver::AdvancePhaseAction{},
				driver::PassAction{},
				driver::AdvancePhaseAction{},
				driver::PassAction{},
		}
	);

	const auto result = session.run();

	CHECK(result.status == driver::LocalDuelSession::Status::NO_ACTION);
	CHECK(session.duel().game().zone_of(monster) == domain::Zone::MONSTER);
	CHECK(session.duel().turn().phase() == domain::Phase::MAIN_1);
}

TEST_CASE("driver::LocalDuelSession reports an invalid action instead of silently passing")
{
	auto first = std::make_unique<driver::ScriptedController>(
		std::vector<driver::LocalAction>{driver::AdvancePhaseAction{}}
	);
	auto second = std::make_unique<driver::ScriptedController>();

	driver::LocalDuelSession session{std::move(first), std::move(second)};

	const auto loaded = session.load_scripts(CG_TEST_CARDS_DIR);
	if (!loaded)
	{
		for (const auto& error: session.duel().take_script_errors())
		{
			MESSAGE(error);
		}
	}
	REQUIRE(loaded);
	REQUIRE(session.initialize(make_vanilla_config()));

	const auto result = session.run();

	CHECK(result.status == driver::LocalDuelSession::Status::INVALID_ACTION);
}

TEST_CASE("driver::LocalDuelSession applies an Extra Deck fusion action")
{
	constexpr domain::CardCode blue_eyes_code{89631139};
	constexpr domain::CardCode blue_eyes_ultimate_code{23995346};

	auto first = std::make_unique<driver::ScriptedController>();
	auto second = std::make_unique<driver::ScriptedController>();
	auto* const first_controller = first.get();
	auto* const second_controller = second.get();

	driver::LocalDuelSession session{std::move(first), std::move(second)};

	const auto loaded = session.load_scripts(CG_CARDS_DIR);
	if (!loaded)
	{
		for (const auto& error: session.duel().take_script_errors())
		{
			MESSAGE(error);
		}
	}
	REQUIRE(loaded);

	flow::DuelDeck deck{};
	deck.main_deck = {blue_eyes_code, blue_eyes_code, blue_eyes_code};
	deck.extra_deck = {blue_eyes_ultimate_code};

	flow::DuelConfig config{};
	config.decks[std::to_underlying(domain::Player::FIRST)] = deck;
	config.decks[std::to_underlying(domain::Player::SECOND)] = deck;
	config.opening_hand = 0;
	config.deck_rules.main_min = 1;
	config.deck_rules.main_max = 60;
	config.deck_rules.extra_max = 1;
	config.deck_rules.side_max = 0;
	config.deck_rules.max_copies = 3;
	REQUIRE(session.initialize(config));

	auto& game = session.duel().game();
	const auto materials = game.collect_zone_card(domain::Player::FIRST, domain::Zone::DECK);
	REQUIRE(materials.size() == 3);
	for (std::size_t index = 0; index < materials.size(); ++index)
	{
		game.place(
			*game.find_instance_mutable(materials[index]),
			domain::MonsterPlacement{
					.column = static_cast<domain::FieldColumn>(std::to_underlying(domain::FieldColumn::C1) + index),
					.position = domain::Position::FACE_UP_ATTACK,
			}
		);
	}

	const auto extra_cards = game.collect_zone_card(domain::Player::FIRST, domain::Zone::EXTRA_DECK);
	REQUIRE(extra_cards.size() == 1);
	const auto fusion = extra_cards.front();

	const auto settle = [&]() -> void
	{
		while (!session.duel().chain().empty())
		{
			session.duel().pass_priority();
		}
	};
	const auto advance_phase = [&]() -> bool
	{
		settle();
		session.duel().pass_priority();
		session.duel().pass_priority();
		return session.duel().change_phase();
	};

	CHECK(advance_phase()); // STANDBY
	CHECK(advance_phase()); // MAIN_1
	REQUIRE(session.duel().turn().phase() == domain::Phase::MAIN_1);

	first_controller->set_actions(
		{
				driver::ExtraSummonAction{
						.kind = domain::ExtraDeckSummonType::FUSION,
						.card = fusion,
						.placement = domain::ExtraMonsterPlacement{
								.column = domain::FieldColumn::EX1,
								.position = domain::Position::FACE_UP_ATTACK,
						},
						.materials = materials,
				},
				driver::PassAction{},
				driver::PassAction{},
				driver::PassAction{},
		}
	);
	second_controller->set_actions(
		{
				driver::PassAction{},
				driver::PassAction{},
				driver::PassAction{},
				driver::PassAction{},
		}
	);

	const auto result = session.run();

	CHECK(result.status == driver::LocalDuelSession::Status::NO_ACTION);
	CHECK(game.zone_of(fusion) == domain::Zone::EXTRA_MONSTER);
	CHECK(game.zone_of(materials[0]) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(materials[1]) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(materials[2]) == domain::Zone::GRAVEYARD);
	CHECK(game.find_instance(fusion)->summon_record().method == domain::SummonMethod::FUSION);
}

TEST_CASE("driver::LocalDuelSession reports surrender")
{
	auto first = std::make_unique<driver::ScriptedController>(
		std::vector<driver::LocalAction>{driver::SurrenderAction{}}
	);
	auto second = std::make_unique<driver::ScriptedController>();

	driver::LocalDuelSession session{std::move(first), std::move(second)};

	const auto loaded = session.load_scripts(CG_TEST_CARDS_DIR);
	if (!loaded)
	{
		for (const auto& error: session.duel().take_script_errors())
		{
			MESSAGE(error);
		}
	}
	REQUIRE(loaded);
	REQUIRE(session.initialize(make_vanilla_config()));

	const auto result = session.run();

	CHECK(result.status == driver::LocalDuelSession::Status::MATCH_OVER);
	REQUIRE(result.winner.has_value());
	CHECK(*result.winner == domain::Player::SECOND);
}

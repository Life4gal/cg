#include <cg/flow/duel.hpp>

#include <array>
#include <filesystem>
#include <vector>

#include <cg/driver/local_test_selector.hpp>
#include <cg/server/match_config.hpp>

#include <doctest/doctest.h>

namespace
{
	using namespace cg;

	// 现有 cards/ 中可组成合法 bootstrap 卡组的真实卡
	constexpr std::array main_codes
	{
			domain::CardCode{12580477},
			domain::CardCode{18144506},
			domain::CardCode{33508719},
			domain::CardCode{41420027},
			domain::CardCode{47529357},
			domain::CardCode{50913601},
			domain::CardCode{53582587},
			domain::CardCode{55144522},
			domain::CardCode{73628505},
			domain::CardCode{76103675},
			domain::CardCode{79759861},
			domain::CardCode{84257639},
			domain::CardCode{89631139},
			domain::CardCode{97017120},
	};

	constexpr std::array extra_codes
	{
			domain::CardCode{23995346},
	};

	[[nodiscard]] auto make_bootstrap_deck() -> flow::DuelDeck
	{
		flow::DuelDeck deck{};

		for (const auto code: main_codes)
		{
			for (std::size_t count = 0; count < 3; ++count)
			{
				deck.main_deck.push_back(code);
			}
		}
		deck.main_deck.resize(40);

		for (const auto code: extra_codes)
		{
			for (std::size_t count = 0; count < 3; ++count)
			{
				deck.extra_deck.push_back(code);
			}
		}

		return deck;
	}

	[[nodiscard]] auto deck_codes(const flow::Duel& duel, const domain::Player player) -> std::vector<domain::CardCode>
	{
		std::vector<domain::CardCode> codes{};
		for (const auto card: duel.game().collect_zone_card(player, domain::Zone::DECK))
		{
			if (const auto* instance = duel.game().find_instance(card);
				instance != nullptr)
			{
				codes.push_back(instance->origin_code());
			}
		}
		return codes;
	}

	driver::LocalTestSelector g_selector{};
}

TEST_CASE("server::MatchConfig loads and initializes a local duel")
{
	const auto config = server::MatchConfig::load(std::filesystem::path{CG_TEST_CONFIG_DIR} / "local_duel.json");
	if (!config.has_value())
	{
		MESSAGE(config.error());
	}
	REQUIRE(config.has_value());

	flow::Duel duel{g_selector};

	REQUIRE(duel.load_scripts(config->cards_path));
	REQUIRE(duel.initialize(config->duel));

	CHECK(duel.game().definitions().size() >= 15);
	CHECK(duel.game().count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == config->duel.opening_hand);
	CHECK(duel.game().count_zone_card(domain::Player::SECOND, domain::Zone::HAND) == config->duel.opening_hand);
	CHECK(duel.game().life_points(domain::Player::FIRST) == config->duel.life_points);
}

TEST_CASE("flow::Duel loads real cards and initializes from DuelConfig")
{
	flow::Duel duel{g_selector};

	const auto loaded = duel.load_scripts(CG_CARDS_DIR);
	if (!loaded)
	{
		for (const auto& error: duel.take_script_errors())
		{
			MESSAGE(error);
		}
	}
	REQUIRE(loaded);

	const auto deck = make_bootstrap_deck();
	REQUIRE(deck.main_deck.size() == 40);
	REQUIRE_FALSE(deck.extra_deck.empty());

	flow::DuelConfig config{};
	config.decks[std::to_underlying(domain::Player::FIRST)] = deck;
	config.decks[std::to_underlying(domain::Player::SECOND)] = deck;
	config.seed = 20261025;
	config.first_player = domain::Player::FIRST;

	REQUIRE(duel.initialize(config));

	const auto& game = duel.game();
	CHECK(game.definitions().size() >= 15);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == config.opening_hand);
	CHECK(game.count_zone_card(domain::Player::SECOND, domain::Zone::HAND) == config.opening_hand);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::DECK) == deck.main_deck.size() - config.opening_hand);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::EXTRA_DECK) == deck.extra_deck.size());
	CHECK(game.life_points(domain::Player::FIRST) == config.life_points);
	CHECK(game.life_points(domain::Player::SECOND) == config.life_points);
	CHECK(duel.turn().turn_player() == config.first_player);
	CHECK(duel.turn().phase() == domain::Phase::DRAW);
}

TEST_CASE("flow::Duel honors first_turn_draw from DuelConfig")
{
	const auto deck = make_bootstrap_deck();

	flow::Duel fixed{g_selector};
	REQUIRE(fixed.load_scripts(CG_CARDS_DIR));
	auto fixed_config = flow::DuelConfig{};
	fixed_config.decks[std::to_underlying(domain::Player::FIRST)] = deck;
	fixed_config.decks[std::to_underlying(domain::Player::SECOND)] = deck;
	fixed_config.seed = 7;
	REQUIRE(fixed.initialize(fixed_config));
	CHECK(fixed.game().count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == 5);
	CHECK_FALSE(fixed.draw_for_turn());
	CHECK(fixed.game().count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == 5);

	flow::Duel enabled{g_selector};
	REQUIRE(enabled.load_scripts(CG_CARDS_DIR));
	auto enabled_config = fixed_config;
	enabled_config.first_turn_draw = true;
	REQUIRE(enabled.initialize(enabled_config));
	CHECK(enabled.game().count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == 5);
	CHECK(enabled.draw_for_turn());
	CHECK(enabled.game().count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == 6);
}

TEST_CASE("flow::Duel initialization is deterministic for the same seed")
{
	flow::Duel first{g_selector};
	flow::Duel second{g_selector};
	REQUIRE(first.load_scripts(CG_CARDS_DIR));
	REQUIRE(second.load_scripts(CG_CARDS_DIR));

	const auto deck = make_bootstrap_deck();

	flow::DuelConfig config{};
	config.decks[std::to_underlying(domain::Player::FIRST)] = deck;
	config.decks[std::to_underlying(domain::Player::SECOND)] = deck;
	config.seed = 42;

	REQUIRE(first.initialize(config));
	REQUIRE(second.initialize(config));

	CHECK(deck_codes(first, domain::Player::FIRST) == deck_codes(second, domain::Player::FIRST));
	CHECK(deck_codes(first, domain::Player::SECOND) == deck_codes(second, domain::Player::SECOND));
}

TEST_CASE("flow::Duel validates side deck cards independently")
{
	flow::Duel duel{g_selector};
	REQUIRE(duel.load_scripts(CG_CARDS_DIR));
	REQUIRE(duel.load_scripts(CG_TEST_CARDS_DIR));

	const auto deck = make_bootstrap_deck();

	SUBCASE("side deck may contain cards not present in main or extra")
	{
		auto config = flow::DuelConfig{};
		config.decks[std::to_underlying(domain::Player::FIRST)] = deck;
		config.decks[std::to_underlying(domain::Player::SECOND)] = deck;
		config.decks[std::to_underlying(domain::Player::FIRST)].side_deck.push_back(domain::CardCode{97000131});

		CHECK(duel.initialize(config));
	}

	SUBCASE("side deck copies count against main and extra copies")
	{
		auto config = flow::DuelConfig{};
		config.decks[std::to_underlying(domain::Player::FIRST)] = deck;
		config.decks[std::to_underlying(domain::Player::SECOND)] = deck;
		config.decks[std::to_underlying(domain::Player::FIRST)].side_deck.push_back(deck.main_deck.front());

		CHECK_FALSE(duel.initialize(config));
	}
}

TEST_CASE("flow::Duel rejects an opening hand larger than the main deck before mutating state")
{
	flow::Duel duel{g_selector};
	REQUIRE(duel.load_scripts(CG_CARDS_DIR));

	const auto deck = make_bootstrap_deck();

	flow::DuelConfig config{};
	config.decks[std::to_underlying(domain::Player::FIRST)] = deck;
	config.decks[std::to_underlying(domain::Player::SECOND)] = deck;
	config.opening_hand = 41;

	CHECK_FALSE(duel.initialize(config));
	CHECK(duel.game().instances().empty());
}

TEST_CASE("flow::Duel rejects illegal DuelConfig decks")
{
	flow::Duel duel{g_selector};
	REQUIRE(duel.load_scripts(CG_CARDS_DIR));

	const auto deck = make_bootstrap_deck();

	SUBCASE("main deck too small")
	{
		auto config = flow::DuelConfig{};
		config.decks[std::to_underlying(domain::Player::FIRST)] = deck;
		config.decks[std::to_underlying(domain::Player::SECOND)] = deck;
		config.decks[std::to_underlying(domain::Player::FIRST)].main_deck.pop_back();

		CHECK_FALSE(duel.initialize(config));
	}

	SUBCASE("unknown card code")
	{
		auto config = flow::DuelConfig{};
		config.decks[std::to_underlying(domain::Player::FIRST)] = deck;
		config.decks[std::to_underlying(domain::Player::SECOND)] = deck;
		config.decks[std::to_underlying(domain::Player::FIRST)].main_deck.front() = domain::CardCode{1};

		CHECK_FALSE(duel.initialize(config));
	}

	SUBCASE("non-extra card in extra deck")
	{
		auto config = flow::DuelConfig{};
		config.decks[std::to_underlying(domain::Player::FIRST)] = deck;
		config.decks[std::to_underlying(domain::Player::SECOND)] = deck;
		config.decks[std::to_underlying(domain::Player::FIRST)].extra_deck.push_back(deck.main_deck.front());

		CHECK_FALSE(duel.initialize(config));
	}
}

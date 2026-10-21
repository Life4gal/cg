#include <cg/flow/duel.hpp>

#include <doctest/doctest.h>

namespace
{
	using namespace cg;

	[[nodiscard]] auto make_monster(const domain::CardCode code) -> state::CardDefinition
	{
		state::CardDefinition definition{};
		definition.code = code;
		definition.kind = {domain::CardType::MONSTER};
		definition.subtypes = {domain::CardSubtype::NORMAL};
		definition.monster = state::CardDefinition::Monster
		{
				.attribute = domain::Attribute::EARTH,
				.race = domain::Race::DRAGON,
				.level = 4,
				.attack = 1000,
				.defense = 1000,
				.pendulum_scale = std::nullopt,
				.link_marker = std::nullopt,
		};

		return definition;
	}
}

TEST_CASE("flow::Duel performs core actions")
{
	constexpr domain::CardCode monster_code{91000001};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(monster_code, make_monster(monster_code));

	const auto deck_card = game.create_instance(monster_code, domain::Player::FIRST);
	const auto hand_card = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(game.place(deck_card, domain::DeckPlacement{.index = 0}));
	REQUIRE(game.place(hand_card, domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE}));

	// 抽卡阶段
	CHECK(duel.turn().phase() == domain::Phase::DRAW);
	CHECK(duel.draw_for_turn());
	CHECK(game.count_hand_zones_card(domain::Player::FIRST) == 2);
	CHECK_FALSE(duel.draw_for_turn());

	// 进入主要阶段
	CHECK(duel.change_phase());
	CHECK(duel.change_phase());
	CHECK(duel.turn().phase() == domain::Phase::MAIN_1);

	// 通常召唤
	CHECK(duel.normal_summon(hand_card, domain::FieldColumn::C1));
	CHECK(game.zone_of(hand_card) == domain::Zone::MONSTER);

	// 一回合一次
	const auto second = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(game.place(second, domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE}));
	CHECK_FALSE(duel.normal_summon(second, domain::FieldColumn::C2));
	CHECK_FALSE(duel.set_monster(second, domain::FieldColumn::C2));

	// 第一回合:MAIN_1 → END → 交换回合
	CHECK(duel.change_phase());
	CHECK(duel.turn().phase() == domain::Phase::END);
	CHECK(duel.end_turn());
	CHECK(duel.turn().turn_player() == domain::Player::SECOND);

	// 对方回合:抽卡 → 主要阶段 → 盖放
	const auto opponent_deck = game.create_instance(monster_code, domain::Player::SECOND);
	REQUIRE(game.place(opponent_deck, domain::DeckPlacement{.index = 0}));
	CHECK(duel.draw_for_turn());
	CHECK(duel.change_phase());
	CHECK(duel.change_phase());
	CHECK(duel.turn().phase() == domain::Phase::MAIN_1);

	const auto opponent_hand = game.create_instance(monster_code, domain::Player::SECOND);
	REQUIRE(game.place(opponent_hand, domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE}));
	CHECK(duel.set_monster(opponent_hand, domain::FieldColumn::C1));

	const auto placement = game.placement(opponent_hand);
	REQUIRE(placement.has_value());
	const auto* monster_placement = std::get_if<domain::MonsterPlacement>(&*placement);
	REQUIRE(monster_placement != nullptr);
	CHECK(monster_placement->position == domain::Position::FACE_DOWN_DEFENSE);
}

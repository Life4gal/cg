#include <cg/flow/duel.hpp>

#include <doctest/doctest.h>

namespace
{
	using namespace cg;

	[[nodiscard]] auto make_monster(
		const domain::CardCode code,
		const std::underlying_type_t<domain::AttackDefense> attack = 1000,
		const std::underlying_type_t<domain::AttackDefense> defense = 1000
	) -> state::CardDefinition
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
				.attack = attack,
				.defense = defense,
				.pendulum_scale = std::nullopt,
				.link_marker = std::nullopt,
		};

		return definition;
	}

	// 先攻空过;返回时轮到后攻的战斗阶段
	auto enter_second_player_battle(flow::Duel& duel) -> void
	{
		duel.change_phase(); // STANDBY
		duel.change_phase(); // MAIN_1
		duel.change_phase(); // END
		duel.end_turn();

		duel.change_phase(); // STANDBY
		duel.change_phase(); // MAIN_1
		duel.change_phase(); // BATTLE
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

TEST_CASE("flow::Duel resolves an attack-position battle")
{
	constexpr domain::CardCode attacker_code{91000010};
	constexpr domain::CardCode defender_code{91000011};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(attacker_code, make_monster(attacker_code, 1000, 1000));
	game.register_definition(defender_code, make_monster(defender_code, 800, 1500));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto defender = game.create_instance(defender_code, domain::Player::FIRST);
	REQUIRE(game.place(defender, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK}));

	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(game.place(attacker, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK}));

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	CHECK(duel.declare_attack(attacker, defender));

	CHECK(game.zone_of(defender) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 7800);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);
	CHECK(duel.winner() == std::nullopt);

	// 同一怪兽一回合只能攻击一次
	CHECK_FALSE(duel.declare_attack(attacker, std::nullopt));
}

TEST_CASE("flow::Duel resolves a defense-position battle")
{
	constexpr domain::CardCode attacker_code{91000012};
	constexpr domain::CardCode defender_code{91000013};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(attacker_code, make_monster(attacker_code, 1000, 1000));
	game.register_definition(defender_code, make_monster(defender_code, 800, 1500));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto defender = game.create_instance(defender_code, domain::Player::FIRST);
	REQUIRE(game.place(defender, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_DEFENSE}));

	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(game.place(attacker, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK}));

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	CHECK(duel.declare_attack(attacker, defender));

	// 守备力更高:攻击方不破坏,受到差值伤害
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	CHECK(game.zone_of(defender) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::SECOND) == 7500);
	CHECK(game.life_points(domain::Player::FIRST) == 8000);
}

TEST_CASE("flow::Duel direct attack and winner")
{
	constexpr domain::CardCode attacker_code{91000014};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(attacker_code, make_monster(attacker_code, 1000, 1000));

	game.set_life_points(domain::Player::FIRST, 1000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(game.place(attacker, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK}));

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	CHECK(duel.declare_attack(attacker, std::nullopt));

	CHECK(game.life_points(domain::Player::FIRST) == 0);
	CHECK(duel.winner() == domain::Player::SECOND);
}

TEST_CASE("flow::Duel flips a face-down defender before damage calculation")
{
	constexpr domain::CardCode attacker_code{91000015};
	constexpr domain::CardCode defender_code{91000016};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(attacker_code, make_monster(attacker_code, 1000, 1000));
	game.register_definition(defender_code, make_monster(defender_code, 800, 800));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto defender = game.create_instance(defender_code, domain::Player::FIRST);
	REQUIRE(game.place(defender, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_DOWN_DEFENSE}));

	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(game.place(attacker, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK}));

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	CHECK(duel.declare_attack(attacker, defender));

	// 里侧守备被翻面后因守备力较低被破坏,且不造成战斗伤害
	CHECK(game.zone_of(defender) == domain::Zone::GRAVEYARD);
	CHECK(game.life_points(domain::Player::FIRST) == 8000);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);
}

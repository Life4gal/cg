#include <cg/state/game.hpp>

#include <doctest/doctest.h>

namespace
{
	using namespace cg;

	[[nodiscard]] auto make_definition(const domain::CardCode code) -> state::CardDefinition
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

TEST_CASE("state::Game moves cards into the requested zones")
{
	constexpr domain::CardCode monster_code{90000002};

	state::Game game{};
	game.register_definition(monster_code, make_definition(monster_code));

	const auto own = game.create_instance(monster_code, domain::Player::FIRST);
	const auto opponent = game.create_instance(monster_code, domain::Player::SECOND);

	REQUIRE(game.place(own, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK}));
	REQUIRE(game.place(opponent, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK}));

	// collect_zones_card_ex 只返回指定玩家的区域
	CHECK(game.collect_zones_card_ex(domain::Player::FIRST, {domain::Zone::MONSTER}).size() == 1);
	CHECK(game.collect_zones_card_ex(domain::Player::SECOND, {domain::Zone::MONSTER}).size() == 1);

	// 额外卡组
	REQUIRE(game.move_to_extra_deck(own, {domain::Reason::EFFECT}));
	CHECK(game.zone_of(own) == domain::Zone::EXTRA_DECK);

	// 除外区
	REQUIRE(game.move_to_banished(opponent, {domain::Reason::EFFECT}));
	CHECK(game.zone_of(opponent) == domain::Zone::BANISHED);
	CHECK(game.count_banished_zones_card(domain::Player::SECOND) == 1);
}

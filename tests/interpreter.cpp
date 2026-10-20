#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>

#include <cg/dsl/json/reader.hpp>
#include <cg/dsl/types.hpp>
#include <cg/dsl/vocabulary.hpp>

#include <cg/execution/interpreter.hpp>

#include <cg/state/game.hpp>

#include <nlohmann/json.hpp>
#include <doctest/doctest.h>

namespace
{
	using namespace cg;

	[[nodiscard]] auto make_definition(const domain::CardCode code, const domain::CardType type) -> state::CardDefinition
	{
		auto definition = state::CardDefinition{};
		definition.code = code;
		definition.kind = {type};
		definition.subtypes = {domain::CardSubtype::NORMAL};

		if (type == domain::CardType::MONSTER)
		{
			definition.monster = state::CardDefinition::Monster
			{
					.attribute = domain::Attribute::EARTH,
					.race = domain::Race::DRAGON,
					.level = 4,
					.attack = 1000,
					.defense = 1000,
			};
		}
		else
		{
			definition.spell = state::CardDefinition::Spell{};
		}

		return definition;
	}
}

TEST_CASE("execution::Interpreter resolves Raigeki")
{
	constexpr auto raigeki_code = domain::CardCode{12580477};
	constexpr auto monster_code = domain::CardCode{90000001};

	auto game = state::Game{};
	game.card_definitions.emplace(raigeki_code, make_definition(raigeki_code, domain::CardType::SPELL));
	game.card_definitions.emplace(monster_code, make_definition(monster_code, domain::CardType::MONSTER));

	constexpr state::CardInstanceId raigeki{1};
	constexpr state::CardInstanceId own_monster{2};
	constexpr state::CardInstanceId opponent_monster{3};

	game.card_instances.emplace_back(raigeki, domain::Player::FIRST, game.card_definitions.at(raigeki_code));
	game.card_instances.emplace_back(own_monster, domain::Player::FIRST, game.card_definitions.at(monster_code));
	game.card_instances.emplace_back(opponent_monster, domain::Player::SECOND, game.card_definitions.at(monster_code));

	REQUIRE(game.place(own_monster, domain::Placement{domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK}}));
	REQUIRE(game.place(opponent_monster, domain::Placement{domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK}}));

	const dsl::TypeRegistry types{};
	const dsl::Vocabulary vocabulary{types};
	dsl::json::Reader reader{types, vocabulary};

	std::ifstream stream{std::filesystem::path{CG_CARDS_DIR} / "12580477.json", std::ios::binary};
	REQUIRE(stream.good());

	std::stringstream buffer{};
	buffer << stream.rdbuf();

	const auto script = reader.read_card_script(nlohmann::json::parse(buffer.str()), "12580477.json");
	REQUIRE(script.has_value());
	REQUIRE(script->effects.size() == 1);

	const auto* activated = std::get_if<dsl::ActivatedEffect>(&script->effects[0].body);
	REQUIRE(activated != nullptr);

	execution::Interpreter interpreter{vocabulary};
	execution::Context context{.game = game, .controller = domain::Player::FIRST, .source = raigeki, .bindings = {}};

	CHECK(interpreter.execute(*activated, context));

	CHECK(game.zone_of(opponent_monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(own_monster) == domain::Zone::MONSTER);
	CHECK(game.count_graveyard_zones_card(domain::Player::SECOND) == 1);
	CHECK(game.count_graveyard_zones_card(domain::Player::FIRST) == 0);
}

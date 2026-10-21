#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>

#include <cg/dsl/json/reader.hpp>
#include <cg/dsl/types.hpp>
#include <cg/dsl/vocabulary.hpp>
#include <cg/state/game.hpp>
#include <cg/execution/interpreter.hpp>
#include <cg/execution/script_registry.hpp>

#include <nlohmann/json.hpp>
#include <doctest/doctest.h>

namespace
{
	using namespace cg;

	const dsl::TypeRegistry g_types{};
	const dsl::Vocabulary g_vocabulary{g_types};
	dsl::json::Reader g_reader{g_types, g_vocabulary};
	execution::ScriptRegistry g_scripts{};

	[[nodiscard]] auto make_definition(const domain::CardCode code, const domain::CardType type) -> state::CardDefinition
	{
		state::CardDefinition definition{};
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
					.pendulum_scale = std::nullopt,
					.link_marker = std::nullopt,
			};
		}
		else
		{
			definition.spell = state::CardDefinition::Spell{};
		}

		return definition;
	}

	[[nodiscard]] auto read_card_script(const std::string_view file) -> std::optional<dsl::CardScript>
	{
		const std::ifstream stream{std::filesystem::path{CG_CARDS_DIR} / file, std::ios::binary};
		if (!stream.good())
		{
			return std::nullopt;
		}

		std::stringstream buffer{};
		buffer << stream.rdbuf();

		std::ignore = g_reader.take_diagnostics();
		return g_reader.read_card_script(nlohmann::json::parse(buffer.str()), file);
	}
}

TEST_CASE("execution::Interpreter resolves Raigeki")
{
	constexpr domain::CardCode raigeki_code{12580477};
	constexpr domain::CardCode monster_code{90000001};

	state::Game game{};
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

	const auto script = read_card_script("12580477.json");
	REQUIRE(script.has_value());
	REQUIRE(script->effects.size() == 1);

	const auto* activated = std::get_if<dsl::ActivatedEffect>(&script->effects[0].body);
	REQUIRE(activated != nullptr);

	execution::Interpreter interpreter{g_vocabulary};
	execution::Context context{game, domain::Player::FIRST, raigeki};

	CHECK(interpreter.execute(*activated, context));

	CHECK(game.zone_of(opponent_monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(own_monster) == domain::Zone::MONSTER);
	CHECK(game.count_graveyard_zones_card(domain::Player::SECOND) == 1);
	CHECK(game.count_graveyard_zones_card(domain::Player::FIRST) == 0);
}

TEST_CASE("execution::Interpreter resolves Tribute to the Doomed")
{
	constexpr domain::CardCode spell_code{79759861};
	constexpr domain::CardCode monster_code{90000003};

	state::Game game{};
	game.card_definitions.emplace(spell_code, make_definition(spell_code, domain::CardType::SPELL));
	game.card_definitions.emplace(monster_code, make_definition(monster_code, domain::CardType::MONSTER));

	constexpr state::CardInstanceId spell{1};
	constexpr state::CardInstanceId hand_card{2};
	constexpr state::CardInstanceId opponent_monster{3};

	game.card_instances.emplace_back(spell, domain::Player::FIRST, game.card_definitions.at(spell_code));
	game.card_instances.emplace_back(hand_card, domain::Player::FIRST, game.card_definitions.at(monster_code));
	game.card_instances.emplace_back(opponent_monster, domain::Player::SECOND, game.card_definitions.at(monster_code));

	REQUIRE(game.place(hand_card, domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE}));
	REQUIRE(game.place(opponent_monster, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK}));

	const auto script = read_card_script("79759861.json");
	REQUIRE(script.has_value());
	REQUIRE(script->effects.size() == 1);

	const auto* activated = std::get_if<dsl::ActivatedEffect>(&script->effects[0].body);
	REQUIRE(activated != nullptr);

	execution::Interpreter interpreter{g_vocabulary};
	execution::Context context{game, domain::Player::FIRST};

	CHECK(interpreter.execute(*activated, context));

	// 费用:舍弃手牌
	CHECK(game.zone_of(hand_card) == domain::Zone::GRAVEYARD);
	CHECK(game.count_hand_zones_card(domain::Player::FIRST) == 0);
	CHECK(game.count_graveyard_zones_card(domain::Player::FIRST) == 1);

	// 对象与处理:破坏对方怪兽
	CHECK(game.zone_of(opponent_monster) == domain::Zone::GRAVEYARD);
	CHECK(game.count_graveyard_zones_card(domain::Player::SECOND) == 1);
}

TEST_CASE("execution::Interpreter evaluates boolean operators")
{
	const auto run = [](const std::string_view condition) -> bool
	{
		const auto text = std::format(
			R"({{"identity":{{"kind":"SPELL","subtypes":["NORMAL"]}},"effects":[{{"type":"activated","activation":{{"mode":"activation"}},"condition":{},"effect":[]}}]}})",
			condition
		);

		state::Game game{};

		const auto script = g_reader.read_card_script(nlohmann::json::parse(text), "<test>");
		REQUIRE(script.has_value());
		REQUIRE(script->effects.size() == 1);

		const auto* activated = std::get_if<dsl::ActivatedEffect>(&script->effects[0].body);
		REQUIRE(activated != nullptr);

		execution::Interpreter interpreter{g_vocabulary};
		execution::Context context{game, domain::Player::FIRST};

		return interpreter.execute(*activated, context);
	};

	CHECK(run(R"({"not": false})"));
	CHECK_FALSE(run(R"({"not": true})"));
	CHECK(run(R"({"and": [true, {"not": false}]})"));
	CHECK_FALSE(run(R"({"and": [true, false]})"));
	CHECK(run(R"({"or": [false, true]})"));
	CHECK_FALSE(run(R"({"or": [false, false]})"));
}

TEST_CASE("execution::Interpreter searches a subtype from the deck")
{
	constexpr domain::CardCode terraforming_code{73628505};
	constexpr domain::CardCode monster_code{90000011};
	constexpr domain::CardCode field_spell_code{90000012};

	state::Game game{};
	game.card_definitions.emplace(terraforming_code, make_definition(terraforming_code, domain::CardType::SPELL));
	game.card_definitions.emplace(monster_code, make_definition(monster_code, domain::CardType::MONSTER));

	state::CardDefinition field_spell{};
	field_spell.code = field_spell_code;
	field_spell.kind = {domain::CardType::SPELL};
	field_spell.subtypes = {domain::CardSubtype::FIELD};
	field_spell.spell = state::CardDefinition::Spell{};
	game.card_definitions.emplace(field_spell_code, field_spell);

	constexpr state::CardInstanceId terraforming{1};
	constexpr state::CardInstanceId monster_in_deck{2};
	constexpr state::CardInstanceId field_spell_in_deck{3};

	game.card_instances.emplace_back(terraforming, domain::Player::FIRST, game.card_definitions.at(terraforming_code));
	game.card_instances.emplace_back(monster_in_deck, domain::Player::FIRST, game.card_definitions.at(monster_code));
	game.card_instances.emplace_back(field_spell_in_deck, domain::Player::FIRST, game.card_definitions.at(field_spell_code));

	REQUIRE(game.place(monster_in_deck, domain::DeckPlacement{.index = 0}));
	REQUIRE(game.place(field_spell_in_deck, domain::DeckPlacement{.index = 1}));

	const auto script = read_card_script("73628505.json");
	REQUIRE(script.has_value());
	REQUIRE(script->effects.size() == 1);

	const auto* activated = std::get_if<dsl::ActivatedEffect>(&script->effects[0].body);
	REQUIRE(activated != nullptr);

	execution::Interpreter interpreter{g_vocabulary};
	execution::Context context{game, domain::Player::FIRST, terraforming};

	CHECK(interpreter.execute(*activated, context));

	CHECK(game.zone_of(field_spell_in_deck) == domain::Zone::HAND);
	CHECK(game.zone_of(monster_in_deck) == domain::Zone::DECK);
	CHECK(game.count_hand_zones_card(domain::Player::FIRST) == 1);
}

TEST_CASE("execution::Interpreter draws cards")
{
	constexpr domain::CardCode pot_code{55144522};
	constexpr domain::CardCode monster_code{90000013};

	state::Game game{};
	game.card_definitions.emplace(pot_code, make_definition(pot_code, domain::CardType::SPELL));
	game.card_definitions.emplace(monster_code, make_definition(monster_code, domain::CardType::MONSTER));

	constexpr state::CardInstanceId pot{1};
	constexpr state::CardInstanceId deck_a{2};
	constexpr state::CardInstanceId deck_b{3};

	game.card_instances.emplace_back(pot, domain::Player::FIRST, game.card_definitions.at(pot_code));
	game.card_instances.emplace_back(deck_a, domain::Player::FIRST, game.card_definitions.at(monster_code));
	game.card_instances.emplace_back(deck_b, domain::Player::FIRST, game.card_definitions.at(monster_code));

	REQUIRE(game.place(deck_a, domain::DeckPlacement{.index = 0}));
	REQUIRE(game.place(deck_b, domain::DeckPlacement{.index = 1}));

	const auto script = read_card_script("55144522.json");
	REQUIRE(script.has_value());
	REQUIRE(script->effects.size() == 1);

	const auto* activated = std::get_if<dsl::ActivatedEffect>(&script->effects[0].body);
	REQUIRE(activated != nullptr);

	execution::Interpreter interpreter{g_vocabulary};
	execution::Context context{game, domain::Player::FIRST, pot};

	CHECK(interpreter.execute(*activated, context));

	CHECK(game.count_hand_zones_card(domain::Player::FIRST) == 2);
	CHECK(game.is_deck_zones_empty(domain::Player::FIRST));
}

TEST_CASE("execution::Interpreter changes life points")
{
	constexpr domain::CardCode sparks_code{76103675};
	constexpr domain::CardCode dian_keto_code{84257639};

	state::Game game{};
	game.card_definitions.emplace(sparks_code, make_definition(sparks_code, domain::CardType::SPELL));
	game.card_definitions.emplace(dian_keto_code, make_definition(dian_keto_code, domain::CardType::SPELL));

	constexpr state::CardInstanceId sparks{1};
	constexpr state::CardInstanceId dian_keto{2};

	game.card_instances.emplace_back(sparks, domain::Player::FIRST, game.card_definitions.at(sparks_code));
	game.card_instances.emplace_back(dian_keto, domain::Player::FIRST, game.card_definitions.at(dian_keto_code));

	game.change_life_points(domain::Player::FIRST, 8000);
	game.change_life_points(domain::Player::SECOND, 8000);

	execution::Interpreter interpreter{g_vocabulary};

	{
		const auto script = read_card_script("76103675.json");
		REQUIRE(script.has_value());
		REQUIRE(script->effects.size() == 1);

		const auto* activated = std::get_if<dsl::ActivatedEffect>(&script->effects[0].body);
		REQUIRE(activated != nullptr);

		execution::Context context{game, domain::Player::FIRST, sparks};
		CHECK(interpreter.execute(*activated, context));
	}

	CHECK(game.life_points(domain::Player::SECOND) == 7800);

	{
		const auto script = read_card_script("84257639.json");
		REQUIRE(script.has_value());
		REQUIRE(script->effects.size() == 1);

		const auto* activated = std::get_if<dsl::ActivatedEffect>(&script->effects[0].body);
		REQUIRE(activated != nullptr);

		execution::Context context{game, domain::Player::FIRST, dian_keto};
		CHECK(interpreter.execute(*activated, context));
	}

	CHECK(game.life_points(domain::Player::FIRST) == 9000);
}

TEST_CASE("execution::Interpreter matches a FLIP trigger and resolves")
{
	constexpr domain::CardCode jar_code{33508719};
	constexpr domain::CardCode monster_code{90000020};

	state::Game game{};
	game.card_definitions.emplace(jar_code, make_definition(jar_code, domain::CardType::MONSTER));
	game.card_definitions.emplace(monster_code, make_definition(monster_code, domain::CardType::MONSTER));

	constexpr state::CardInstanceId jar{1};
	game.card_instances.emplace_back(jar, domain::Player::FIRST, game.card_definitions.at(jar_code));
	REQUIRE(game.place(jar, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_DOWN_DEFENSE}));

	const auto add_to_hand = [&](const state::CardInstanceId card, const domain::Player player) -> void
	{
		game.card_instances.emplace_back(card, player, game.card_definitions.at(monster_code));
		REQUIRE(game.place(card, domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE}));
	};
	const auto add_to_deck = [&](const state::CardInstanceId card, const domain::Player player) -> void
	{
		game.card_instances.emplace_back(card, player, game.card_definitions.at(monster_code));
		REQUIRE(game.place(card, domain::DeckPlacement{.index = 0}));
	};

	add_to_hand(state::CardInstanceId{10}, domain::Player::FIRST);
	add_to_hand(state::CardInstanceId{11}, domain::Player::FIRST);
	add_to_hand(state::CardInstanceId{12}, domain::Player::SECOND);
	add_to_hand(state::CardInstanceId{13}, domain::Player::SECOND);

	for (state::CardInstanceId::value_type i = 0; i < 5; ++i)
	{
		add_to_deck(state::CardInstanceId{static_cast<state::CardInstanceId::value_type>(20 + i)}, domain::Player::FIRST);
		add_to_deck(state::CardInstanceId{static_cast<state::CardInstanceId::value_type>(30 + i)}, domain::Player::SECOND);
	}

	const auto script = read_card_script("33508719.json");
	REQUIRE(script.has_value());
	REQUIRE(script->effects.size() == 1);

	const auto* activated = std::get_if<dsl::ActivatedEffect>(&script->effects[0].body);
	REQUIRE(activated != nullptr);

	const auto* trigger = std::get_if<dsl::Trigger>(&activated->activation);
	REQUIRE(trigger != nullptr);

	execution::Interpreter interpreter{g_vocabulary};
	execution::Context context{game, domain::Player::FIRST, jar};
	context.trigger_source = dsl::TriggerSource{domain::EventCode::FLIP};
	context.event_card = jar;

	CHECK(interpreter.matches(*trigger, context));
	CHECK(interpreter.execute(*activated, context));

	CHECK(game.count_hand_zones_card(domain::Player::FIRST) == 5);
	CHECK(game.count_hand_zones_card(domain::Player::SECOND) == 5);
	CHECK(game.count_graveyard_zones_card(domain::Player::FIRST) == 2);
	CHECK(game.count_graveyard_zones_card(domain::Player::SECOND) == 2);
	CHECK(game.is_deck_zones_empty(domain::Player::FIRST));
	CHECK(game.is_deck_zones_empty(domain::Player::SECOND));
}

TEST_CASE("execution::Interpreter collects triggered effects in SEGOC order")
{
	constexpr domain::CardCode jar_code{33508719};
	constexpr domain::CardCode optional_code{90000030};

	state::Game game{};
	game.card_definitions.emplace(jar_code, make_definition(jar_code, domain::CardType::MONSTER));
	game.card_definitions.emplace(optional_code, make_definition(optional_code, domain::CardType::MONSTER));

	// 对手的壶实例ID更小,用于证明排序不是简单按实例顺序
	constexpr state::CardInstanceId opponent_jar{1};
	constexpr state::CardInstanceId turn_jar{2};
	constexpr state::CardInstanceId optional_card{3};

	game.card_instances.emplace_back(opponent_jar, domain::Player::SECOND, game.card_definitions.at(jar_code));
	game.card_instances.emplace_back(turn_jar, domain::Player::FIRST, game.card_definitions.at(jar_code));
	game.card_instances.emplace_back(optional_card, domain::Player::FIRST, game.card_definitions.at(optional_code));

	REQUIRE(game.place(opponent_jar, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_DOWN_DEFENSE}));
	REQUIRE(game.place(turn_jar, domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_DOWN_DEFENSE}));
	REQUIRE(game.place(optional_card, domain::MonsterPlacement{.column = domain::FieldColumn::C3, .position = domain::Position::FACE_UP_ATTACK}));

	const auto jar = read_card_script("33508719.json");
	REQUIRE(jar.has_value());
	g_scripts.add(jar_code, *jar);

	// 合成一个同一事件(FLIP)的任意触发,用于验证"必发优先"
	dsl::ActivatedEffect optional_activated{};
	optional_activated.activation = dsl::ActivationKind
	{
			dsl::Trigger
			{
					.spec = dsl::TriggerSpec
					{
							.sources =
							{
									{dsl::TriggerSource{domain::EventCode::FLIP}},
							},
							.condition = std::nullopt,
							.timing = dsl::TriggerTiming::WHEN,
					},
					.optional = true,
			}
	};

	dsl::CardScript optional_script{};
	optional_script.effects.push_back(
		dsl::Effect
		{
				.id = dsl::EffectId{1},
				.body = dsl::EffectBody{std::move(optional_activated)},
		}
	);
	g_scripts.add(optional_code, std::move(optional_script));

	execution::Interpreter interpreter{g_vocabulary};
	execution::Context context{game, domain::Player::FIRST};

	context.trigger_source = dsl::TriggerSource{domain::EventCode::FLIP};

	const auto triggered = interpreter.collect_triggered(g_scripts, context, domain::Player::FIRST);
	REQUIRE(triggered.size() == 3);

	CHECK(triggered[0].card == turn_jar);
	CHECK(triggered[1].card == opponent_jar);
	CHECK(triggered[2].card == optional_card);
}

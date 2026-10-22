#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>

#include <cg/dsl/json/reader.hpp>
#include <cg/dsl/types.hpp>
#include <cg/dsl/vocabulary.hpp>
#include <cg/state/game.hpp>
#include <cg/execution/chain.hpp>
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
	game.register_definition(raigeki_code, make_definition(raigeki_code, domain::CardType::SPELL));
	game.register_definition(monster_code, make_definition(monster_code, domain::CardType::MONSTER));

	const auto raigeki = game.create_instance(raigeki_code, domain::Player::FIRST);
	const auto own_monster = game.create_instance(monster_code, domain::Player::FIRST);
	const auto opponent_monster = game.create_instance(monster_code, domain::Player::SECOND);

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
	game.register_definition(spell_code, make_definition(spell_code, domain::CardType::SPELL));
	game.register_definition(monster_code, make_definition(monster_code, domain::CardType::MONSTER));

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	const auto hand_card = game.create_instance(monster_code, domain::Player::FIRST);
	const auto opponent_monster = game.create_instance(monster_code, domain::Player::SECOND);

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
	game.register_definition(terraforming_code, make_definition(terraforming_code, domain::CardType::SPELL));
	game.register_definition(monster_code, make_definition(monster_code, domain::CardType::MONSTER));

	state::CardDefinition field_spell{};
	field_spell.code = field_spell_code;
	field_spell.kind = {domain::CardType::SPELL};
	field_spell.subtypes = {domain::CardSubtype::FIELD};
	field_spell.spell = state::CardDefinition::Spell{};
	game.register_definition(field_spell_code, field_spell);

	const auto terraforming = game.create_instance(terraforming_code, domain::Player::FIRST);
	const auto monster_in_deck = game.create_instance(monster_code, domain::Player::FIRST);
	const auto field_spell_in_deck = game.create_instance(field_spell_code, domain::Player::FIRST);

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
	game.register_definition(pot_code, make_definition(pot_code, domain::CardType::SPELL));
	game.register_definition(monster_code, make_definition(monster_code, domain::CardType::MONSTER));

	const auto pot = game.create_instance(pot_code, domain::Player::FIRST);
	const auto deck_a = game.create_instance(monster_code, domain::Player::FIRST);
	const auto deck_b = game.create_instance(monster_code, domain::Player::FIRST);

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
	game.register_definition(sparks_code, make_definition(sparks_code, domain::CardType::SPELL));
	game.register_definition(dian_keto_code, make_definition(dian_keto_code, domain::CardType::SPELL));

	const auto sparks = game.create_instance(sparks_code, domain::Player::FIRST);
	const auto dian_keto = game.create_instance(dian_keto_code, domain::Player::FIRST);

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
	game.register_definition(jar_code, make_definition(jar_code, domain::CardType::MONSTER));
	game.register_definition(monster_code, make_definition(monster_code, domain::CardType::MONSTER));

	const auto jar = game.create_instance(jar_code, domain::Player::FIRST);
	REQUIRE(game.place(jar, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_DOWN_DEFENSE}));

	const auto add_to_hand = [&](const domain::Player player) -> void
	{
		const auto id = game.create_instance(monster_code, player);
		REQUIRE(game.place(id, domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE}));
	};
	const auto add_to_deck = [&](const domain::Player player) -> void
	{
		const auto id = game.create_instance(monster_code, player);
		REQUIRE(game.place(id, domain::DeckPlacement{.index = 0}));
	};

	add_to_hand(domain::Player::FIRST);
	add_to_hand(domain::Player::FIRST);
	add_to_hand(domain::Player::SECOND);
	add_to_hand(domain::Player::SECOND);

	for (state::CardInstanceId::value_type i = 0; i < 5; ++i)
	{
		add_to_deck(domain::Player::FIRST);
		add_to_deck(domain::Player::SECOND);
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
	game.register_definition(jar_code, make_definition(jar_code, domain::CardType::MONSTER));
	game.register_definition(optional_code, make_definition(optional_code, domain::CardType::MONSTER));

	// 对手的壶实例ID更小,用于证明排序不是简单按实例顺序
	const auto opponent_jar = game.create_instance(jar_code, domain::Player::SECOND);
	const auto turn_jar = game.create_instance(jar_code, domain::Player::FIRST);
	const auto optional_card = game.create_instance(optional_code, domain::Player::FIRST);

	REQUIRE(game.place(opponent_jar, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_DOWN_DEFENSE}));
	REQUIRE(game.place(turn_jar, domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_DOWN_DEFENSE}));
	REQUIRE(game.place(optional_card, domain::MonsterPlacement{.column = domain::FieldColumn::C3, .position = domain::Position::FACE_UP_ATTACK}));

	const auto jar = read_card_script("33508719.json");
	REQUIRE(jar.has_value());

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

	execution::ScriptRegistry scripts{};
	scripts.add(jar_code, *jar);
	scripts.add(optional_code, std::move(optional_script));

	execution::Interpreter interpreter{g_vocabulary};
	execution::Context context{game, domain::Player::FIRST};

	context.trigger_source = dsl::TriggerSource{domain::EventCode::FLIP};

	const auto triggered = interpreter.collect_triggered(scripts, context, domain::Player::FIRST);
	REQUIRE(triggered.size() == 3);

	CHECK(triggered[0].instance.get().id() == turn_jar);
	CHECK(triggered[1].instance.get().id() == opponent_jar);
	CHECK(triggered[2].instance.get().id() == optional_card);
}

TEST_CASE("execution::Interpreter builds and resolves a chain")
{
	constexpr domain::CardCode rat_code{97017120};
	constexpr domain::CardCode monster_code{90000040};

	state::Game game{};
	game.register_definition(rat_code, make_definition(rat_code, domain::CardType::MONSTER));
	game.register_definition(monster_code, make_definition(monster_code, domain::CardType::MONSTER));

	// 两只巨大老鼠在墓地(触发条件:在墓地)
	const auto first_rat = game.create_instance(rat_code, domain::Player::FIRST);
	const auto second_rat = game.create_instance(rat_code, domain::Player::SECOND);

	REQUIRE(game.place(first_rat, domain::GraveyardPlacement{.index = 0}));
	REQUIRE(game.place(second_rat, domain::GraveyardPlacement{.index = 0}));

	// 双方卡组各一张可被检索的怪兽
	const auto first_deck = game.create_instance(monster_code, domain::Player::FIRST);
	const auto second_deck = game.create_instance(monster_code, domain::Player::SECOND);

	REQUIRE(game.place(first_deck, domain::DeckPlacement{.index = 0}));
	REQUIRE(game.place(second_deck, domain::DeckPlacement{.index = 0}));

	const auto rat = read_card_script("97017120.json");
	REQUIRE(rat.has_value());

	execution::ScriptRegistry scripts{};
	scripts.add(rat_code, *rat);

	execution::Interpreter interpreter{g_vocabulary};
	execution::Context context{game, domain::Player::FIRST};

	context.trigger_source = dsl::TriggerSource{domain::EventCode::BATTLE_DESTROYED};

	const auto triggered = interpreter.collect_triggered(scripts, context, domain::Player::FIRST);
	REQUIRE(triggered.size() == 2);

	auto chain = interpreter.build_chain(triggered, context);
	REQUIRE(chain.size() == 2);

	CHECK(interpreter.resolve(chain));

	CHECK(game.is_deck_zones_empty(domain::Player::FIRST));
	CHECK(game.is_deck_zones_empty(domain::Player::SECOND));
	CHECK(game.count_monster_zones_card(domain::Player::FIRST) == 1);
	CHECK(game.count_monster_zones_card(domain::Player::SECOND) == 1);
	CHECK(game.count_graveyard_zones_card(domain::Player::FIRST) == 1);
	CHECK(game.count_graveyard_zones_card(domain::Player::SECOND) == 1);
}

TEST_CASE("execution::Interpreter applies static numeric modifiers")
{
	const auto make_monster = [](const domain::CardCode code, const domain::Race race) -> state::CardDefinition
	{
		auto definition = make_definition(code, domain::CardType::MONSTER);
		definition.monster->race = race;

		return definition;
	};

	constexpr domain::CardCode mountain_code{50913601};
	constexpr domain::CardCode dragon_code{90000050};
	constexpr domain::CardCode warrior_code{90000051};

	state::Game game{};
	game.register_definition(mountain_code, make_definition(mountain_code, domain::CardType::SPELL));
	game.register_definition(dragon_code, make_monster(dragon_code, domain::Race::DRAGON));
	game.register_definition(warrior_code, make_monster(warrior_code, domain::Race::WARRIOR));

	const auto mountain = game.create_instance(mountain_code, domain::Player::FIRST);
	const auto dragon = game.create_instance(dragon_code, domain::Player::FIRST);
	const auto warrior = game.create_instance(warrior_code, domain::Player::FIRST);

	REQUIRE(game.place(mountain, domain::FieldSpellPlacement{.position = domain::Position::FACE_UP_ATTACK}));
	REQUIRE(game.place(dragon, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK}));
	REQUIRE(game.place(warrior, domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK}));

	const auto mountain_script = read_card_script("50913601.json");
	REQUIRE(mountain_script.has_value());

	execution::ScriptRegistry scripts{};
	scripts.add(mountain_code, *mountain_script);

	execution::Interpreter interpreter{g_vocabulary};
	execution::Context context{game, domain::Player::FIRST};

	CHECK(interpreter.static_numeric_bonus(scripts, dragon, domain::NumericProperty::ATTACK, context) == 200);
	CHECK(interpreter.static_numeric_bonus(scripts, dragon, domain::NumericProperty::DEFENSE, context) == 200);
	CHECK(interpreter.static_numeric_bonus(scripts, warrior, domain::NumericProperty::ATTACK, context) == 0);
}

TEST_CASE("execution::ScriptRegistry loads the card corpus")
{
	execution::ScriptRegistry scripts{};

	CHECK(scripts.load_directory(std::filesystem::path{CG_CARDS_DIR}, g_types, g_vocabulary));
	CHECK(scripts.take_errors().empty());
	CHECK(scripts.size() > 0);

	CHECK(scripts.find(domain::CardCode{12580477}) != nullptr);
	CHECK(scripts.find(domain::CardCode{33508719}) != nullptr);
	CHECK(scripts.find(domain::CardCode{50913601}) != nullptr);
}

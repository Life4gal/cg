#include <cg/flow/duel.hpp>

#include <fstream>

#include <doctest/doctest.h>

namespace
{
	using namespace cg;

	[[nodiscard]] auto make_monster(
		const domain::CardCode code,
		const std::underlying_type_t<domain::AttackDefense> attack = 1000,
		const std::underlying_type_t<domain::AttackDefense> defense = 1000,
		const std::underlying_type_t<domain::LevelRank> level = 4,
		const domain::Race race = domain::Race::DRAGON
	) -> state::CardDefinition
	{
		state::CardDefinition definition{};
		definition.code = code;
		definition.kind = {domain::CardType::MONSTER};
		definition.subtypes = {domain::CardSubtype::NORMAL};
		definition.monster = state::CardDefinition::Monster
		{
				.attribute = domain::Attribute::EARTH,
				.race = race,
				.level = level,
				.attack = attack,
				.defense = defense,
				.pendulum_scale = std::nullopt,
				.link_marker = std::nullopt,
		};

		return definition;
	}

	[[nodiscard]] auto make_spell(const domain::CardCode code, const std::initializer_list<domain::CardSubtype> subtypes) -> state::CardDefinition
	{
		state::CardDefinition definition{};
		definition.code = code;
		definition.kind = {domain::CardType::SPELL};
		definition.subtypes = subtypes;
		definition.spell = state::CardDefinition::Spell{};

		return definition;
	}

	[[nodiscard]] auto make_trap(const domain::CardCode code, const std::initializer_list<domain::CardSubtype> subtypes) -> state::CardDefinition
	{
		state::CardDefinition definition{};
		definition.code = code;
		definition.kind = {domain::CardType::TRAP};
		definition.subtypes = subtypes;
		definition.trap = state::CardDefinition::Trap{};

		return definition;
	}

	// 双方放弃优先权后推进阶段
	auto advance_phase(flow::Duel& duel) -> bool
	{
		duel.pass_priority();
		duel.pass_priority();
		return duel.change_phase();
	}

	// 先攻空过;返回时轮到后攻的战斗阶段
	auto enter_second_player_battle(flow::Duel& duel) -> void
	{
		advance_phase(duel); // STANDBY
		advance_phase(duel); // MAIN_1
		advance_phase(duel); // END
		duel.end_turn();

		advance_phase(duel); // STANDBY
		advance_phase(duel); // MAIN_1
		advance_phase(duel); // BATTLE
	}
}

TEST_CASE("flow::Duel performs core actions")
{
	constexpr domain::CardCode monster_code{91000001};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(monster_code, make_monster(monster_code));

	const auto deck_card = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(deck_card.valid());
	const auto hand_card = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(hand_card.valid());

	game.place(*game.find_instance_mutable(deck_card), domain::DeckPlacement{.index = 0});
	game.place(*game.find_instance_mutable(hand_card), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	// 抽卡阶段
	CHECK(duel.turn().phase() == domain::Phase::DRAW);
	CHECK(duel.draw_for_turn());
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == 2);
	CHECK_FALSE(duel.draw_for_turn());

	// 进入主要阶段
	CHECK(advance_phase(duel));
	CHECK(advance_phase(duel));
	CHECK(duel.turn().phase() == domain::Phase::MAIN_1);

	// 通常召唤
	CHECK(duel.normal_summon(hand_card, domain::FieldColumn::C1));
	CHECK(game.zone_of(hand_card) == domain::Zone::MONSTER);

	// 一回合一次
	const auto second = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(second.valid());

	game.place(*game.find_instance_mutable(second), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	CHECK_FALSE(duel.normal_summon(second, domain::FieldColumn::C2));
	CHECK_FALSE(duel.set_monster(second, domain::FieldColumn::C2));

	// 第一回合:MAIN_1 → END → 交换回合
	CHECK(advance_phase(duel));
	CHECK(duel.turn().phase() == domain::Phase::END);
	CHECK(duel.end_turn());
	CHECK(duel.turn().turn_player() == domain::Player::SECOND);

	// 对方回合:抽卡 → 主要阶段 → 盖放
	const auto opponent_deck = game.create_instance(monster_code, domain::Player::SECOND);
	REQUIRE(opponent_deck.valid());

	game.place(*game.find_instance_mutable(opponent_deck), domain::DeckPlacement{.index = 0});
	CHECK(duel.draw_for_turn());
	CHECK(advance_phase(duel));
	CHECK(advance_phase(duel));
	CHECK(duel.turn().phase() == domain::Phase::MAIN_1);

	const auto opponent_hand = game.create_instance(monster_code, domain::Player::SECOND);
	REQUIRE(opponent_hand.valid());

	game.place(*game.find_instance_mutable(opponent_hand), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	CHECK(duel.set_monster(opponent_hand, domain::FieldColumn::C1));

	const auto placement = game.find_instance(opponent_hand)->placement();
	const auto* monster_placement = std::get_if<domain::MonsterPlacement>(&placement);
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
	REQUIRE(defender.valid());
	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(attacker.valid());

	game.place(*game.find_instance_mutable(defender), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});
	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	CHECK(duel.declare_attack(attacker, defender));

	CHECK(game.zone_of(defender) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 7800);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);
	CHECK(duel.winner() == std::nullopt);

	// 战斗/伤害步骤状态机推进到结束
	CHECK(duel.battle().battle_step() == domain::BattleStep::END);
	CHECK_FALSE(duel.battle().is_in_damage_step());

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
	REQUIRE(defender.valid());
	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(attacker.valid());

	game.place(*game.find_instance_mutable(defender), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_DEFENSE});
	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

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
	REQUIRE(attacker.valid());

	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

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
	REQUIRE(defender.valid());
	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(attacker.valid());

	game.place(*game.find_instance_mutable(defender), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	CHECK(duel.declare_attack(attacker, defender));

	// 里侧守备被翻面后因守备力较低被破坏,且不造成战斗伤害
	CHECK(game.zone_of(defender) == domain::Zone::GRAVEYARD);
	CHECK(game.life_points(domain::Player::FIRST) == 8000);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);
}

TEST_CASE("flow::Duel tribute summons a level 5 monster")
{
	constexpr domain::CardCode high_code{91000020};
	constexpr domain::CardCode low_code{91000021};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(high_code, make_monster(high_code, 2000, 2000, 5));
	game.register_definition(low_code, make_monster(low_code, 1000, 1000));

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	const auto high = game.create_instance(high_code, domain::Player::FIRST);
	REQUIRE(high.valid());
	const auto tribute = game.create_instance(low_code, domain::Player::FIRST);
	REQUIRE(tribute.valid());

	game.place(*game.find_instance_mutable(high), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(tribute), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	// 等级 5 不能直接通常召唤
	CHECK_FALSE(duel.normal_summon(high, domain::FieldColumn::C2));

	const std::array tributes{tribute};
	CHECK(duel.tribute_summon(high, domain::FieldColumn::C2, tributes));

	CHECK(game.zone_of(tribute) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(high) == domain::Zone::MONSTER);
}

TEST_CASE("flow::Duel requires two tributes for a level 7 monster")
{
	constexpr domain::CardCode high_code{91000022};
	constexpr domain::CardCode low_code{91000023};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(high_code, make_monster(high_code, 2500, 2500, 7));
	game.register_definition(low_code, make_monster(low_code, 1000, 1000));

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	const auto high = game.create_instance(high_code, domain::Player::FIRST);
	REQUIRE(high.valid());
	const auto first = game.create_instance(low_code, domain::Player::FIRST);
	REQUIRE(first.valid());
	const auto second = game.create_instance(low_code, domain::Player::FIRST);
	REQUIRE(second.valid());

	game.place(*game.find_instance_mutable(high), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(first), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});
	game.place(*game.find_instance_mutable(second), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	const std::array one{first};
	CHECK_FALSE(duel.tribute_summon(high, domain::FieldColumn::C3, one));

	const std::array two{first, second};
	CHECK(duel.tribute_summon(high, domain::FieldColumn::C3, two));

	CHECK(game.zone_of(first) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(second) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(high) == domain::Zone::MONSTER);
}

TEST_CASE("flow::Duel loses when drawing from an empty deck")
{
	flow::Duel duel{};

	CHECK_FALSE(duel.draw_for_turn());
	CHECK(duel.winner() == domain::Player::SECOND);
}

TEST_CASE("flow::Duel requires both players to pass before advancing phase")
{
	flow::Duel duel{};

	CHECK(duel.turn().phase() == domain::Phase::DRAW);

	// 未放弃优先权时不能推进
	CHECK_FALSE(duel.change_phase());

	// 回合玩家放弃 → 对手窗口
	CHECK_FALSE(duel.pass_priority());
	CHECK_FALSE(duel.change_phase());

	// 对手放弃 → 双方通过
	CHECK(duel.pass_priority());
	CHECK(duel.change_phase());
	CHECK(duel.turn().phase() == domain::Phase::STANDBY);

	// 新阶段优先权重置
	CHECK_FALSE(duel.priority().passed());
	CHECK(duel.priority().holder() == flow::Priority::Holder::TURN_PLAYER);
}

TEST_CASE("flow::Duel triggers effects from scripts")
{
	constexpr domain::CardCode jar_code{33508719};
	constexpr domain::CardCode monster_code{92000001};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(jar_code, make_monster(jar_code));
	game.register_definition(monster_code, make_monster(monster_code));

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	const auto jar = game.create_instance(jar_code, domain::Player::FIRST);
	REQUIRE(jar.valid());
	game.place(*game.find_instance_mutable(jar), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto add_to_hand = [&](const domain::Player player) -> void
	{
		const auto card = game.create_instance(monster_code, player);
		REQUIRE(card.valid());
		game.place(*game.find_instance_mutable(card), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	};
	const auto add_to_deck = [&](const domain::Player player) -> void
	{
		const auto card = game.create_instance(monster_code, player);
		REQUIRE(card.valid());
		game.place(*game.find_instance_mutable(card), domain::DeckPlacement{.index = 0});
	};

	for (auto i = 0; i < 2; ++i)
	{
		add_to_hand(domain::Player::FIRST);
		add_to_hand(domain::Player::SECOND);
	}

	for (auto i = 0; i < 5; ++i)
	{
		add_to_deck(domain::Player::FIRST);
		add_to_deck(domain::Player::SECOND);
	}

	CHECK(duel.trigger_event(dsl::TriggerSource{domain::EventCode::FLIP}, jar));

	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == 5);
	CHECK(game.count_zone_card(domain::Player::SECOND, domain::Zone::HAND) == 5);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::GRAVEYARD) == 2);
	CHECK(game.count_zone_card(domain::Player::SECOND, domain::Zone::GRAVEYARD) == 2);

	// 不匹配的事件不触发
	CHECK_FALSE(duel.trigger_event(dsl::TriggerSource{domain::EventCode::BATTLE_DESTROYED}, jar));
}

TEST_CASE("flow::Duel applies static modifiers in battle")
{
	constexpr domain::CardCode mountain_code{50913601};
	constexpr domain::CardCode dragon_code{93000001};
	constexpr domain::CardCode warrior_code{93000002};

	flow::Duel duel{};
	auto& game = duel.game();

	state::CardDefinition mountain{};
	mountain.code = mountain_code;
	mountain.kind = {domain::CardType::SPELL};
	mountain.subtypes = {domain::CardSubtype::FIELD};
	mountain.spell = state::CardDefinition::Spell{};
	game.register_definition(mountain_code, mountain);

	game.register_definition(dragon_code, make_monster(dragon_code, 1000, 1000, 4, domain::Race::DRAGON));
	game.register_definition(warrior_code, make_monster(warrior_code, 1000, 1000, 4, domain::Race::WARRIOR));

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto mountain_card = game.create_instance(mountain_code, domain::Player::FIRST);
	REQUIRE(mountain_card.valid());
	game.place(*game.find_instance_mutable(mountain_card), domain::FieldSpellPlacement{.position = domain::Position::FACE_UP_ATTACK});

	const auto defender = game.create_instance(warrior_code, domain::Player::FIRST);
	REQUIRE(defender.valid());
	game.place(*game.find_instance_mutable(defender), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto attacker = game.create_instance(dragon_code, domain::Player::SECOND);
	REQUIRE(attacker.valid());
	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	// 印刷值保持 1000,战场计算使用有效值(含 Mountain)
	CHECK(game.find_instance(attacker)->origin_attack() == 1000);
	CHECK(game.find_instance(attacker)->origin_defense() == 1000);

	// 龙族 1000 + Mountain 200 = 1200 > 战士 1000
	CHECK(duel.declare_attack(attacker, defender));

	CHECK(game.zone_of(defender) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 7800);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);
}

TEST_CASE("flow::CardStateResolver feeds effective stats to scripts")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_effective_stats";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"99500006.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"WIND","race":"DRAGON","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"ignition"},"condition":{"ge":[{"call":"attack","args":{"card":{"call":"this_card","args":{}}}},1200]},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})"
	);

	constexpr domain::CardCode mountain_code{50913601};
	constexpr domain::CardCode monster_code{99500006};

	const auto run = [&](const bool with_mountain) -> std::pair<bool, std::underlying_type_t<domain::AttackDefense>>
	{
		flow::Duel duel{};
		auto& game = duel.game();

		game.register_definition(mountain_code, make_spell(mountain_code, {domain::CardSubtype::FIELD}));
		game.register_definition(monster_code, make_monster(monster_code, 1000, 1000, 4, domain::Race::DRAGON));

		REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));
		REQUIRE(duel.load_scripts(directory));

		game.set_life_points(domain::Player::FIRST, 8000);
		game.set_life_points(domain::Player::SECOND, 8000);

		if (with_mountain)
		{
			const auto mountain = game.create_instance(mountain_code, domain::Player::FIRST);
			REQUIRE(mountain.valid());
			game.place(*game.find_instance_mutable(mountain), domain::FieldSpellPlacement{.position = domain::Position::FACE_UP_ATTACK});
		}

		const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
		REQUIRE(monster.valid());
		game.place(*game.find_instance_mutable(monster), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

		advance_phase(duel); // STANDBY
		advance_phase(duel); // MAIN_1
		REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

		// 印刷值仍是 1000;条件里的 attack 应读取有效值
		const auto activated = duel.activate_effect(monster, 0);
		return {activated, game.life_points(domain::Player::SECOND)};
	};

	const auto [with_mountain, lp_with] = run(true);
	CHECK(with_mountain);
	CHECK(lp_with == 7500);

	const auto [without_mountain, lp_without] = run(false);
	CHECK_FALSE(without_mountain);
	CHECK(lp_without == 8000);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::CardStateResolver applies static discrete properties")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_static_properties";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"99500007.json",
		R"({"identity":{"kind":"SPELL","subtypes":["FIELD"]},"effects":[{"type":"static","apply_to":{"call":"is_card_type","args":{"card":"$","card_type":"MONSTER"}},"modifiers":[{"call":"add_attribute","args":{"attribute":"DARK"}},{"call":"change_level","args":{"delta":1}},{"call":"add_card_subtype","args":{"subtype":"TUNER"}}]}]})"
	);
	write(
		"99500008.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"WIND","race":"DRAGON","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"ignition"},"condition":{"and":[{"call":"is_attribute","args":{"card":{"call":"this_card","args":{}},"attribute":"WIND"}},{"call":"is_attribute","args":{"card":{"call":"this_card","args":{}},"attribute":"DARK"}},{"ge":[{"call":"level","args":{"card":{"call":"this_card","args":{}}}},5]},{"call":"is_card_subtype","args":{"card":{"call":"this_card","args":{}},"subtype":"TUNER"}}]},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})"
	);

	constexpr domain::CardCode field_code{99500007};
	constexpr domain::CardCode monster_code{99500008};

	const auto run = [&](const bool with_field) -> std::pair<bool, std::underlying_type_t<domain::AttackDefense>>
	{
		flow::Duel duel{};
		auto& game = duel.game();

		game.register_definition(field_code, make_spell(field_code, {domain::CardSubtype::FIELD}));

		state::CardDefinition monster{};
		monster.code = monster_code;
		monster.kind = {domain::CardType::MONSTER};
		monster.subtypes = {domain::CardSubtype::EFFECT};
		monster.monster = state::CardDefinition::Monster
		{
				.attribute = domain::Attribute::WIND,
				.race = domain::Race::DRAGON,
				.level = 4,
				.attack = 1000,
				.defense = 1000,
				.pendulum_scale = std::nullopt,
				.link_marker = std::nullopt,
		};
		game.register_definition(monster_code, monster);

		REQUIRE(duel.load_scripts(directory));

		game.set_life_points(domain::Player::FIRST, 8000);
		game.set_life_points(domain::Player::SECOND, 8000);

		if (with_field)
		{
			const auto field = game.create_instance(field_code, domain::Player::FIRST);
			REQUIRE(field.valid());
			game.place(*game.find_instance_mutable(field), domain::FieldSpellPlacement{.position = domain::Position::FACE_UP_ATTACK});
		}

		const auto card = game.create_instance(monster_code, domain::Player::FIRST);
		REQUIRE(card.valid());
		game.place(*game.find_instance_mutable(card), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

		const auto* instance = game.find_instance(card);
		REQUIRE(instance != nullptr);
		CHECK(instance->origin_attribute() == domain::Attribute::WIND);
		CHECK(instance->origin_level() == 4);
		CHECK_FALSE(instance->origin_is_card_subtype(domain::CardSubtype::TUNER));

		advance_phase(duel); // STANDBY
		advance_phase(duel); // MAIN_1
		REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

		const auto activated = duel.activate_effect(card, 0);
		return {activated, game.life_points(domain::Player::SECOND)};
	};

	const auto [with_field, lp_with] = run(true);
	CHECK(with_field);
	CHECK(lp_with == 7500);

	const auto [without_field, lp_without] = run(false);
	CHECK_FALSE(without_field);
	CHECK(lp_without == 8000);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel performs Synchro and Xyz summons")
{
	constexpr domain::CardCode tuner_code{99600001};
	constexpr domain::CardCode non_tuner_code{99600002};
	constexpr domain::CardCode synchro_code{99600003};
	constexpr domain::CardCode xyz_code{99600004};
	constexpr domain::CardCode material_a_code{99600005};
	constexpr domain::CardCode material_b_code{99600006};

	flow::Duel duel{};
	auto& game = duel.game();

	auto tuner = make_monster(tuner_code, 1000, 1000, 2);
	tuner.subtypes = {domain::CardSubtype::EFFECT, domain::CardSubtype::TUNER};
	auto non_tuner = make_monster(non_tuner_code, 1000, 1000, 6);
	auto synchro = make_monster(synchro_code, 2500, 2000, 8);
	synchro.subtypes = {domain::CardSubtype::SYNCHRO};
	auto xyz = make_monster(xyz_code, 2000, 2000, 4);
	xyz.subtypes = {domain::CardSubtype::XYZ};
	auto material_a = make_monster(material_a_code, 1000, 1000, 4);
	auto material_b = make_monster(material_b_code, 1000, 1000, 4);

	game.register_definition(tuner_code, tuner);
	game.register_definition(non_tuner_code, non_tuner);
	game.register_definition(synchro_code, synchro);
	game.register_definition(xyz_code, xyz);
	game.register_definition(material_a_code, material_a);
	game.register_definition(material_b_code, material_b);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto tuner_card = game.create_instance(tuner_code, domain::Player::FIRST);
	REQUIRE(tuner_card.valid());
	game.place(*game.find_instance_mutable(tuner_card), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});
	const auto non_tuner_card = game.create_instance(non_tuner_code, domain::Player::FIRST);
	REQUIRE(non_tuner_card.valid());
	game.place(*game.find_instance_mutable(non_tuner_card), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});
	const auto material_a_card = game.create_instance(material_a_code, domain::Player::FIRST);
	REQUIRE(material_a_card.valid());
	game.place(*game.find_instance_mutable(material_a_card), domain::MonsterPlacement{.column = domain::FieldColumn::C3, .position = domain::Position::FACE_UP_ATTACK});
	const auto material_b_card = game.create_instance(material_b_code, domain::Player::FIRST);
	REQUIRE(material_b_card.valid());
	game.place(*game.find_instance_mutable(material_b_card), domain::MonsterPlacement{.column = domain::FieldColumn::C4, .position = domain::Position::FACE_UP_ATTACK});

	const auto synchro_card = game.create_instance(synchro_code, domain::Player::FIRST);
	REQUIRE(synchro_card.valid());
	game.place(*game.find_instance_mutable(synchro_card), domain::ExtraDeckPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	const auto xyz_card = game.create_instance(xyz_code, domain::Player::FIRST);
	REQUIRE(xyz_card.valid());
	game.place(*game.find_instance_mutable(xyz_card), domain::ExtraDeckPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	const std::array synchro_materials{tuner_card, non_tuner_card};
	CHECK(duel.synchro_summon(synchro_card, domain::MonsterPlacement{.column = domain::FieldColumn::C5, .position = domain::Position::FACE_UP_ATTACK}, synchro_materials));
	CHECK(game.zone_of(synchro_card) == domain::Zone::MONSTER);
	CHECK(game.zone_of(tuner_card) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(non_tuner_card) == domain::Zone::GRAVEYARD);
	CHECK(game.find_instance(synchro_card)->summon_record().method == domain::SummonMethod::SYNCHRO);

	const std::array xyz_materials{material_a_card, material_b_card};
	CHECK(duel.xyz_summon(xyz_card, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, xyz_materials));
	CHECK(game.zone_of(xyz_card) == domain::Zone::EXTRA_MONSTER);
	CHECK(game.zone_of(material_a_card) == domain::Zone::NOT_EXIST);
	CHECK(game.zone_of(material_b_card) == domain::Zone::NOT_EXIST);
	CHECK(game.find_instance(xyz_card)->overlays().size() == 2);
	CHECK(game.find_instance(xyz_card)->summon_record().method == domain::SummonMethod::XYZ);
}

TEST_CASE("flow::Duel keeps battle-protected monsters")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_protection";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	// 全场怪物获得战斗破坏抗性
	write(
		"95000001.json",
		R"({"identity":{"kind":"SPELL","subtypes":["FIELD"]},"effects":[{"type":"static","apply_to":true,"modifiers":[{"call":"add_protection","args":{"protection":"BATTLE"}}]}]})"
	);

	constexpr domain::CardCode field_code{95000001};
	constexpr domain::CardCode defender_code{95000002};
	constexpr domain::CardCode attacker_code{95000003};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(field_code, make_spell(field_code, {domain::CardSubtype::FIELD}));
	game.register_definition(defender_code, make_monster(defender_code, 800, 800));
	game.register_definition(attacker_code, make_monster(attacker_code, 1000, 1000));

	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto field = game.create_instance(field_code, domain::Player::FIRST);
	REQUIRE(field.valid());
	game.place(*game.find_instance_mutable(field), domain::FieldSpellPlacement{.position = domain::Position::FACE_UP_ATTACK});

	const auto defender = game.create_instance(defender_code, domain::Player::FIRST);
	REQUIRE(defender.valid());
	game.place(*game.find_instance_mutable(defender), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(attacker.valid());
	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	// 攻击力差仍造成战斗伤害,但被保护的怪物不会被战斗破坏
	CHECK(duel.declare_attack(attacker, defender));

	CHECK(game.zone_of(defender) == domain::Zone::MONSTER);
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 7800);
}

TEST_CASE("flow::Duel fires battle destruction triggers automatically")
{
	constexpr domain::CardCode rat_code{97017120};
	constexpr domain::CardCode attacker_code{96000001};
	constexpr domain::CardCode recruit_code{96000002};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(rat_code, make_monster(rat_code, 1400, 1450, 4, domain::Race::BEAST));
	game.register_definition(attacker_code, make_monster(attacker_code, 1600, 1000));
	game.register_definition(recruit_code, make_monster(recruit_code, 1000, 1000));

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto rat = game.create_instance(rat_code, domain::Player::FIRST);
	REQUIRE(rat.valid());
	game.place(*game.find_instance_mutable(rat), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(attacker.valid());
	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto recruit = game.create_instance(recruit_code, domain::Player::FIRST);
	REQUIRE(recruit.valid());
	game.place(*game.find_instance_mutable(recruit), domain::DeckPlacement{.index = 0});

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	CHECK(duel.declare_attack(attacker, rat));

	// 战破后自动触发 Giant Rat:从卡组特召 EARTH/攻击力 1500 以下
	CHECK(game.zone_of(rat) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	CHECK(game.zone_of(recruit) == domain::Zone::MONSTER);
}

TEST_CASE("flow::Duel processes destruction events from effects")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_event_queue";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"97000011.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"ignition"},"effect":[{"call":"destroy","args":{"cards":{"call":"find_cards","args":{"players":["OPPONENT"],"zones":["MONSTER"]}},"reason":"EFFECT"}}]}]})"
	);
	write(
		"97000012.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"DESTROYED"},"timing":"WHEN","optional":false,"condition":{"call":"is_in_zone","args":{"card":{"call":"this_card","args":{}},"zones":["GRAVEYARD"]}}},"effect":[{"call":"draw","args":{"players":["SELF"],"count":1}}]}]})"
	);

	constexpr domain::CardCode igniter_code{97000011};
	constexpr domain::CardCode victim_code{97000012};
	constexpr domain::CardCode draw_code{97000013};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(igniter_code, make_monster(igniter_code));
	game.register_definition(victim_code, make_monster(victim_code));
	game.register_definition(draw_code, make_monster(draw_code));

	REQUIRE(duel.load_scripts(directory));

	const auto igniter = game.create_instance(igniter_code, domain::Player::FIRST);
	REQUIRE(igniter.valid());
	game.place(*game.find_instance_mutable(igniter), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto victim = game.create_instance(victim_code, domain::Player::SECOND);
	REQUIRE(victim.valid());
	game.place(*game.find_instance_mutable(victim), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto draw_card = game.create_instance(draw_code, domain::Player::SECOND);
	REQUIRE(draw_card.valid());
	game.place(*game.find_instance_mutable(draw_card), domain::DeckPlacement{.index = 0});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	// 效果破坏 → 结算后统一开 DESTROYED 时点
	CHECK(duel.activate_effect(igniter, 0));

	CHECK(game.zone_of(victim) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(igniter) == domain::Zone::MONSTER);
	CHECK(game.count_zone_card(domain::Player::SECOND, domain::Zone::HAND) == 1);
	CHECK(game.count_zone_card(domain::Player::SECOND, domain::Zone::DECK) == 0);
}

TEST_CASE("flow::Duel shares usage counters within a group")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_usage_group";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"99500001.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"ignition"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]},{"type":"activated","usage":{"kind":"TURN_PER_NAME"},"mutex":0,"activation":{"mode":"ignition"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":300}}]},{"type":"activated","usage":{"kind":"TURN_PER_NAME"},"mutex":0,"activation":{"mode":"ignition"},"effect":[{"call":"recover_life","args":{"players":["SELF"],"amount":500}}]}]})"
	);

	constexpr domain::CardCode card_code{99500001};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(card_code, make_monster(card_code));
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto card = game.create_instance(card_code, domain::Player::FIRST);
	REQUIRE(card.valid());
	game.place(*game.find_instance_mutable(card), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	// 三个效果互斥:发动其一后,其余两个也被次数限制拒绝
	CHECK(duel.activate_effect(card, 0));
	CHECK(duel.activate_effect(card, 1) == false);
	CHECK(duel.activate_effect(card, 2) == false);

	CHECK(game.life_points(domain::Player::SECOND) == 7500);
	CHECK(game.life_points(domain::Player::FIRST) == 8000);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel applies mutex independently of usage counts")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_usage_combined";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"99500002.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"ignition"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]},{"type":"activated","usage":{"kind":"TURN_PER_CARD","count":2},"mutex":0,"activation":{"mode":"ignition"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":100}}]}]})"
	);

	constexpr domain::CardCode card_code{99500002};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(card_code, make_monster(card_code));
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto card = game.create_instance(card_code, domain::Player::FIRST);
	REQUIRE(card.valid());
	game.place(*game.find_instance_mutable(card), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	// 效果0:无usage限制;先选它后,效果1被互斥锁住
	CHECK(duel.activate_effect(card, 0));
	CHECK(duel.activate_effect(card, 0));
	CHECK(duel.activate_effect(card, 1) == false);
	CHECK(game.life_points(domain::Player::SECOND) == 7000);

	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	// 对方回合空过
	for (auto i = 0; i < 5; ++i)
	{
		CHECK(advance_phase(duel));
	}
	REQUIRE(duel.turn().phase() == domain::Phase::END);
	CHECK(duel.end_turn());

	// 回到先攻:互斥选择重置,这次选效果1;它自己的次数上限是2
	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(duel.activate_effect(card, 1));
	CHECK(duel.activate_effect(card, 1));
	CHECK(duel.activate_effect(card, 1) == false);
	CHECK(duel.activate_effect(card, 0) == false);
	CHECK(game.life_points(domain::Player::SECOND) == 6800);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel rejects invalid mutex links")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_usage_duplicate";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"99500003.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","usage":{"kind":"TURN_PER_NAME"},"mutex":1,"activation":{"mode":"ignition"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]},{"type":"activated","usage":{"kind":"TURN_PER_CARD"},"activation":{"mode":"ignition"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":100}}]}]})"
	);

	constexpr domain::CardCode card_code{99500003};

	flow::Duel duel{};
	duel.game().register_definition(card_code, make_monster(card_code));

	// mutex 只能指向更早效果 → 加载失败
	CHECK_FALSE(duel.load_scripts(directory));

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel enforces triggered effect usage limits")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_usage";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"99000021.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"ignition"},"effect":[{"call":"destroy","args":{"cards":{"call":"find_cards","args":{"players":["OPPONENT"],"zones":["MONSTER"]}},"reason":"EFFECT"}}]}]})"
	);
	write(
		"99000022.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","usage":{"kind":"TURN_PER_NAME"},"activation":{"mode":"trigger","source":{"event":"DESTROYED"},"timing":"WHEN","optional":false,"condition":{"call":"is_in_zone","args":{"card":{"call":"this_card","args":{}},"zones":["GRAVEYARD"]}}},"effect":[{"call":"draw","args":{"players":["SELF"],"count":1}}]}]})"
	);

	constexpr domain::CardCode destroyer_code{99000021};
	constexpr domain::CardCode victim_code{99000022};
	constexpr domain::CardCode draw_code{99000023};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(destroyer_code, make_monster(destroyer_code));
	game.register_definition(victim_code, make_monster(victim_code));
	game.register_definition(draw_code, make_monster(draw_code));

	REQUIRE(duel.load_scripts(directory));

	const auto destroyer = game.create_instance(destroyer_code, domain::Player::FIRST);
	REQUIRE(destroyer.valid());
	game.place(*game.find_instance_mutable(destroyer), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto victim_a = game.create_instance(victim_code, domain::Player::SECOND);
	REQUIRE(victim_a.valid());
	game.place(*game.find_instance_mutable(victim_a), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto victim_b = game.create_instance(victim_code, domain::Player::SECOND);
	REQUIRE(victim_b.valid());
	game.place(*game.find_instance_mutable(victim_b), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	for (auto i = 0; i < 2; ++i)
	{
		const auto card = game.create_instance(draw_code, domain::Player::SECOND);
		REQUIRE(card.valid());
		game.place(*game.find_instance_mutable(card), domain::DeckPlacement{.index = 0});
	}

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	// 一次破坏两张同名卡,但「同名卡一回合一次」只允许抽1
	CHECK(duel.activate_effect(destroyer, 0));

	CHECK(game.zone_of(victim_a) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(victim_b) == domain::Zone::GRAVEYARD);
	CHECK(game.count_zone_card(domain::Player::SECOND, domain::Zone::HAND) == 1);
	CHECK(game.count_zone_card(domain::Player::SECOND, domain::Zone::DECK) == 1);
}

TEST_CASE("flow::Duel activates traps with an event timing")
{
	constexpr domain::CardCode torrential_code{53582587};
	constexpr domain::CardCode monster_code{98000001};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(torrential_code, make_trap(torrential_code, {domain::CardSubtype::NORMAL}));
	game.register_definition(monster_code, make_monster(monster_code));

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto torrential = game.create_instance(torrential_code, domain::Player::SECOND);
	REQUIRE(torrential.valid());
	game.place(*game.find_instance_mutable(torrential), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(monster.valid());
	game.place(*game.find_instance_mutable(monster), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	// 先攻空过
	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	advance_phase(duel); // END
	REQUIRE(duel.end_turn());

	// 后攻盖放 Torrential
	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.set_spell_trap(torrential, domain::FieldColumn::C1));
	advance_phase(duel); // BATTLE
	advance_phase(duel); // MAIN_2
	advance_phase(duel); // END
	REQUIRE(duel.end_turn());

	// 先攻召唤 → 召唤成功窗口
	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.normal_summon(monster, domain::FieldColumn::C1));

	// 先攻放弃优先权 → 后攻可发动带 WHEN 时点的陷阱
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.activate_card(torrential));

	CHECK(game.zone_of(monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(torrential) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel activates ignition and quick effects")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_activation";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"94000001.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"ignition"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})"
	);
	write(
		"94000002.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"quick"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})"
	);

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(domain::CardCode{94000001}, make_monster(domain::CardCode{94000001}));
	game.register_definition(domain::CardCode{94000002}, make_monster(domain::CardCode{94000002}));

	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto ignition_card = game.create_instance(domain::CardCode{94000001}, domain::Player::FIRST);
	REQUIRE(ignition_card.valid());
	const auto quick_card = game.create_instance(domain::CardCode{94000002}, domain::Player::SECOND);
	REQUIRE(quick_card.valid());
	game.place(*game.find_instance_mutable(ignition_card), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});
	game.place(*game.find_instance_mutable(quick_card), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	// DRAW → STANDBY → MAIN_1
	CHECK(advance_phase(duel));
	CHECK(advance_phase(duel));
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	// 启动效果:回合玩家、主要阶段
	CHECK(duel.activate_effect(ignition_card, 0));
	CHECK(game.life_points(domain::Player::SECOND) == 7500);

	// 对手的诱发即时:优先权仍在回合玩家 → 失败
	CHECK_FALSE(duel.activate_effect(quick_card, 0));
	CHECK(game.life_points(domain::Player::FIRST) == 8000);

	// 回合玩家放弃 → 对手获得优先权 → 成功
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.activate_effect(quick_card, 0));
	CHECK(game.life_points(domain::Player::FIRST) == 7500);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel enforces per-turn usage restrictions")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_usage";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	{
		std::ofstream stream{directory / "95000001.json", std::ios::binary};
		stream <<
				R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","usage":{"kind":"TURN_PER_NAME","verb":"USE"},"activation":{"mode":"ignition"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})";
	}

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(domain::CardCode{95000001}, make_monster(domain::CardCode{95000001}));
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto card = game.create_instance(domain::CardCode{95000001}, domain::Player::FIRST);
	REQUIRE(card.valid());
	game.place(*game.find_instance_mutable(card), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	// 本回合第一次成功,第二次被次数限制拒绝
	CHECK(duel.activate_effect(card, 0));
	CHECK(duel.activate_effect(card, 0) == false);
	CHECK(game.life_points(domain::Player::SECOND) == 7500);

	// 结束本回合
	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	// 对方回合空过
	for (auto i = 0; i < 5; ++i)
	{
		CHECK(advance_phase(duel));
	}
	REQUIRE(duel.turn().phase() == domain::Phase::END);
	CHECK(duel.end_turn());

	// 回到先攻回合:限制已重置
	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(duel.activate_effect(card, 0));
	CHECK(game.life_points(domain::Player::SECOND) == 7000);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel activates spell cards")
{
	constexpr domain::CardCode raigeki_code{12580477};
	constexpr domain::CardCode mountain_code{50913601};
	constexpr domain::CardCode monster_code{96000001};

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(raigeki_code, make_spell(raigeki_code, {domain::CardSubtype::NORMAL}));
	game.register_definition(mountain_code, make_spell(mountain_code, {domain::CardSubtype::FIELD}));
	game.register_definition(monster_code, make_monster(monster_code));

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	const auto raigeki = game.create_instance(raigeki_code, domain::Player::FIRST);
	REQUIRE(raigeki.valid());
	const auto mountain = game.create_instance(mountain_code, domain::Player::FIRST);
	REQUIRE(mountain.valid());
	const auto monster = game.create_instance(monster_code, domain::Player::SECOND);
	REQUIRE(monster.valid());

	game.place(*game.find_instance_mutable(raigeki), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(mountain), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(monster), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	// 抽卡阶段不能发动魔法
	CHECK_FALSE(duel.activate_card(raigeki));

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1

	// 通常魔法:结算后送墓
	CHECK(duel.activate_card(raigeki));
	CHECK(game.zone_of(monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(raigeki) == domain::Zone::GRAVEYARD);

	// 场地魔法:留场
	CHECK(duel.activate_card(mountain));
	CHECK(game.zone_of(mountain) == domain::Zone::FIELD_SPELL);

	// 非魔法卡不可用此接口
	CHECK_FALSE(duel.activate_card(monster));
}

TEST_CASE("flow::Duel sets and activates a trap")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_trap";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	{
		std::ofstream stream{directory / "97000001.json", std::ios::binary};
		stream <<
				R"({"identity":{"kind":"TRAP","subtypes":["NORMAL"]},"effects":[{"type":"activated","activation":{"mode":"activation"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})";
	}

	flow::Duel duel{};
	auto& game = duel.game();

	game.register_definition(domain::CardCode{97000001}, make_trap(domain::CardCode{97000001}, {domain::CardSubtype::NORMAL}));
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto trap = game.create_instance(domain::CardCode{97000001}, domain::Player::FIRST);
	REQUIRE(trap.valid());
	game.place(*game.find_instance_mutable(trap), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1

	// 盖放
	CHECK(duel.set_spell_trap(trap, domain::FieldColumn::C1));
	CHECK(game.zone_of(trap) == domain::Zone::SPELL_TRAP);

	// 盖放当回合不能发动
	CHECK_FALSE(duel.activate_card(trap));

	// 结束回合
	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	// 对方回合:放弃优先权 → 陷阱控制者获得窗口
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.activate_card(trap));

	CHECK(game.life_points(domain::Player::SECOND) == 7500);
	CHECK(game.zone_of(trap) == domain::Zone::GRAVEYARD);

	std::filesystem::remove_all(directory);
}

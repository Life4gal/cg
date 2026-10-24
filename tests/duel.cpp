#include <cg/flow/duel.hpp>

#include <algorithm>
#include <fstream>
#include <numeric>

#include <cg/driver/local_test_selector.hpp>

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

	// 把待结算的连锁全部结算完(测试用)
	auto settle(flow::Duel& duel) -> void
	{
		while (!duel.chain().empty())
		{
			duel.pass_priority();
		}
	}

	// 双方放弃优先权后推进阶段
	// 推进并结算当前攻击,直到伤害步骤结束时点关闭
	auto resolve_attack(flow::Duel& duel) -> void
	{
		while (duel.pending_attack())
		{
			duel.pass_priority();
		}
	}

	// 推进并结算当前召唤,直到成功/无效时点打开
	auto resolve_summon(flow::Duel& duel) -> void
	{
		while (duel.pending_summon())
		{
			duel.pass_priority();
		}
	}

	// 推进到指定伤害步骤(测试无触发/连锁的战斗)
	auto advance_to_damage_step(flow::Duel& duel, const domain::DamageStep step) -> void
	{
		while (
			duel.pending_attack() &&
			(duel.battle().battle_step() != domain::BattleStep::DAMAGE || duel.battle().damage_step() != step)
		)
		{
			duel.pass_priority();
		}
	}

	auto advance_phase(flow::Duel& duel) -> bool
	{
		settle(duel);
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

	driver::LocalTestSelector g_selector{};
}

TEST_CASE("flow::Duel performs core actions")
{
	constexpr domain::CardCode monster_code{91000001};

	flow::Duel duel{g_selector};
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
	resolve_summon(duel);
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

	flow::Duel duel{g_selector};
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
	resolve_attack(duel);

	CHECK(game.zone_of(defender) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 7800);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);
	CHECK(duel.winner() == std::nullopt);

	// 战斗/伤害步骤状态机推进到结束
	CHECK_FALSE(duel.pending_attack());
	CHECK(duel.battle().battle_step() == domain::BattleStep::BATTLE);
	CHECK_FALSE(duel.battle().is_in_damage_step());

	// 同一怪兽一回合只能攻击一次
	CHECK_FALSE(duel.declare_attack(attacker, std::nullopt));
}

TEST_CASE("flow::Duel delays battle resolution until the attack declaration window closes")
{
	constexpr domain::CardCode attacker_code{91000030};
	constexpr domain::CardCode defender_code{91000031};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(attacker_code, make_monster(attacker_code, 1000, 1000));
	game.register_definition(defender_code, make_monster(defender_code, 800, 1000));

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

	// 攻击宣言时点打开:尚未进行伤害计算,双方都可以响应
	CHECK(duel.pending_attack());
	CHECK(duel.battle().battle_step() == domain::BattleStep::BATTLE);
	CHECK_FALSE(duel.battle().is_in_damage_step());
	CHECK(game.zone_of(defender) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 8000);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);

	resolve_attack(duel);

	CHECK_FALSE(duel.pending_attack());
	CHECK(game.zone_of(defender) == domain::Zone::GRAVEYARD);
	CHECK(game.life_points(domain::Player::FIRST) == 7800);
}

TEST_CASE("flow::Duel resolves attack-declaration triggers before damage calculation")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_attack_trigger";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	std::ofstream stream{directory / "97000021.json", std::ios::binary};
	stream <<
			R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"EARTH","race":"BEAST","level":4,"attack":800,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"ATTACK_DECLARED"},"timing":"WHEN","optional":false},"effect":[{"call":"destroy","args":{"cards":{"call":"find_cards","args":{"players":["OPPONENT"],"zones":["MONSTER","EXTRA_MONSTER"]}},"reason":"EFFECT"}}]}]})";
	stream.close();

	constexpr domain::CardCode watcher_code{97000021};
	constexpr domain::CardCode attacker_code{97000022};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(watcher_code, make_monster(watcher_code, 800, 1000));
	game.register_definition(attacker_code, make_monster(attacker_code, 1000, 1000));

	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto watcher = game.create_instance(watcher_code, domain::Player::FIRST);
	REQUIRE(watcher.valid());
	game.place(*game.find_instance_mutable(watcher), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(attacker.valid());
	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	CHECK(duel.declare_attack(attacker, watcher));

	// 攻击宣言触发已入链,但效果尚未结算
	REQUIRE(duel.chain().size() == 1);
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	resolve_attack(duel);

	// 攻击者被触发效果破坏,攻击中止,不进行伤害计算
	CHECK(game.zone_of(attacker) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(watcher) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 8000);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel sends battle-destroyed monsters at the end of the damage step")
{
	constexpr domain::CardCode attacker_code{91000040};
	constexpr domain::CardCode defender_code{91000041};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(attacker_code, make_monster(attacker_code, 1000, 1000));
	game.register_definition(defender_code, make_monster(defender_code, 800, 1000));

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

	// 攻击宣言 → 伤害步骤开始
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.pass_priority());
	CHECK(duel.battle().battle_step() == domain::BattleStep::DAMAGE);
	CHECK(duel.battle().damage_step() == domain::DamageStep::START);

	// 伤害步骤开始 → 伤害计算前
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.pass_priority());
	CHECK(duel.battle().damage_step() == domain::DamageStep::BEFORE_CALCULATION);

	// 伤害计算前 → 伤害计算时
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.pass_priority());
	CHECK(duel.battle().damage_step() == domain::DamageStep::PERFORM_CALCULATION);

	// 伤害计算时 → 伤害计算后:伤害已确定,但破坏尚未送墓
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.pass_priority());
	CHECK(duel.battle().damage_step() == domain::DamageStep::AFTER_CALCULATION);
	CHECK(game.zone_of(defender) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 7800);

	// 伤害计算后 → 伤害步骤结束:此时才送墓
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.pass_priority());
	CHECK(duel.battle().damage_step() == domain::DamageStep::END);
	CHECK(game.zone_of(defender) == domain::Zone::GRAVEYARD);
	CHECK(duel.pending_attack());

	// 结束时点关闭,攻击状态清除
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.pass_priority());
	CHECK_FALSE(duel.pending_attack());
	CHECK(duel.battle().battle_step() == domain::BattleStep::BATTLE);
}

TEST_CASE("flow::Duel allows another monster to attack after the first attack ends")
{
	constexpr domain::CardCode attacker_code{91000050};
	constexpr domain::CardCode defender_code{91000051};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(attacker_code, make_monster(attacker_code, 1000, 1000));
	game.register_definition(defender_code, make_monster(defender_code, 800, 1000));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto defender_a = game.create_instance(defender_code, domain::Player::FIRST);
	REQUIRE(defender_a.valid());
	const auto defender_b = game.create_instance(defender_code, domain::Player::FIRST);
	REQUIRE(defender_b.valid());
	const auto attacker_a = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(attacker_a.valid());
	const auto attacker_b = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(attacker_b.valid());

	game.place(*game.find_instance_mutable(defender_a), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});
	game.place(*game.find_instance_mutable(defender_b), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});
	game.place(*game.find_instance_mutable(attacker_a), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});
	game.place(*game.find_instance_mutable(attacker_b), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	CHECK(duel.declare_attack(attacker_a, defender_a));
	resolve_attack(duel);
	CHECK(game.zone_of(defender_a) == domain::Zone::GRAVEYARD);

	// 第一次攻击结束后回到战斗步骤,另一只怪兽可以继续攻击
	CHECK(duel.battle().battle_step() == domain::BattleStep::BATTLE);
	CHECK(duel.declare_attack(attacker_b, defender_b));
	resolve_attack(duel);

	CHECK(game.zone_of(defender_b) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(attacker_a) == domain::Zone::MONSTER);
	CHECK(game.zone_of(attacker_b) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 7600);
}

TEST_CASE("flow::Duel enforces damage-step activation categories")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_damage_step_properties";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	std::ofstream stream{directory / "97000041.json", std::ios::binary};
	stream <<
			R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"quick"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]},{"type":"activated","activation":{"mode":"quick"},"properties":["ATK_DEF_CHANGE"],"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]},{"type":"activated","activation":{"mode":"quick"},"properties":["DAMAGE_CALCULATION"],"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]},{"type":"activated","activation":{"mode":"quick"},"properties":["ACTIVATION_NEGATE"],"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})";
	stream.close();

	constexpr domain::CardCode quick_code{97000041};
	constexpr domain::CardCode attacker_code{97000042};
	constexpr domain::CardCode defender_code{97000043};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(quick_code, make_monster(quick_code, 1000, 1000));
	game.register_definition(attacker_code, make_monster(attacker_code, 1000, 1000));
	game.register_definition(defender_code, make_monster(defender_code, 1000, 1000));

	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto quick = game.create_instance(quick_code, domain::Player::FIRST);
	REQUIRE(quick.valid());
	game.place(*game.find_instance_mutable(quick), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	const auto defender = game.create_instance(defender_code, domain::Player::FIRST);
	REQUIRE(defender.valid());
	game.place(*game.find_instance_mutable(defender), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(attacker.valid());
	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	CHECK(duel.declare_attack(attacker, defender));
	advance_to_damage_step(duel, domain::DamageStep::BEFORE_CALCULATION);
	REQUIRE(duel.battle().battle_step() == domain::BattleStep::DAMAGE);
	REQUIRE(duel.battle().damage_step() == domain::DamageStep::BEFORE_CALCULATION);

	// 让非回合玩家(FIRST)获得优先权
	CHECK_FALSE(duel.pass_priority());

	// 无属性:伤害步骤内不可发动
	CHECK_FALSE(duel.activate_effect(quick, 0));
	CHECK(duel.chain().empty());

	// ATK/DEF 变化:伤害计算前可发动
	CHECK(duel.activate_effect(quick, 1));
	CHECK(duel.chain().size() == 1);
	settle(duel);
	CHECK(game.life_points(domain::Player::SECOND) == 7500);

	// 明确「伤害计算时」:伤害计算前不可发动
	CHECK_FALSE(duel.pass_priority());
	CHECK_FALSE(duel.activate_effect(quick, 2));

	// 进入伤害计算时
	advance_to_damage_step(duel, domain::DamageStep::PERFORM_CALCULATION);
	REQUIRE(duel.battle().damage_step() == domain::DamageStep::PERFORM_CALCULATION);

	CHECK_FALSE(duel.pass_priority());

	// ATK/DEF 变化在伤害计算时起不可发动
	CHECK_FALSE(duel.activate_effect(quick, 1));

	// 明确「伤害计算时」在此时可发动
	CHECK(duel.activate_effect(quick, 2));
	settle(duel);
	CHECK(game.life_points(domain::Player::SECOND) == 7000);

	// 无效「发动」类:伤害计算后仍可发动
	advance_to_damage_step(duel, domain::DamageStep::AFTER_CALCULATION);
	REQUIRE(duel.battle().damage_step() == domain::DamageStep::AFTER_CALCULATION);
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.activate_effect(quick, 3));
	settle(duel);
	CHECK(game.life_points(domain::Player::SECOND) == 6500);

	resolve_attack(duel);

	CHECK(game.zone_of(attacker) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(defender) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(quick) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 8000);
	CHECK(game.life_points(domain::Player::SECOND) == 6500);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel allows Counter Traps in the damage step")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_damage_step_counter";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	std::ofstream stream{directory / "97000051.json", std::ios::binary};
	stream <<
			R"({"identity":{"kind":"TRAP","subtypes":["COUNTER"]},"effects":[{"type":"activated","activation":{"mode":"activation"},"effect":[]}]})";
	stream.close();

	constexpr domain::CardCode counter_code{97000051};
	constexpr domain::CardCode attacker_code{97000052};
	constexpr domain::CardCode defender_code{97000053};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(counter_code, make_trap(counter_code, {domain::CardSubtype::COUNTER}));
	game.register_definition(attacker_code, make_monster(attacker_code, 1000, 1000));
	game.register_definition(defender_code, make_monster(defender_code, 1000, 1000));

	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto counter = game.create_instance(counter_code, domain::Player::FIRST);
	REQUIRE(counter.valid());
	game.place(*game.find_instance_mutable(counter), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto defender = game.create_instance(defender_code, domain::Player::FIRST);
	REQUIRE(defender.valid());
	game.place(*game.find_instance_mutable(defender), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(attacker.valid());
	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	// 第一回合盖放,第二回合发动
	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(duel.set_spell_trap(counter, domain::FieldColumn::C1));
	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(advance_phase(duel)); // BATTLE
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	CHECK(duel.declare_attack(attacker, defender));
	advance_to_damage_step(duel, domain::DamageStep::BEFORE_CALCULATION);
	REQUIRE(duel.battle().damage_step() == domain::DamageStep::BEFORE_CALCULATION);

	CHECK_FALSE(duel.pass_priority());

	// Counter Trap 在伤害步骤内可发动
	CHECK(duel.activate_card(counter));
	CHECK(duel.chain().size() == 1);
	settle(duel);
	CHECK(game.zone_of(counter) == domain::Zone::GRAVEYARD);

	resolve_attack(duel);
	CHECK(game.zone_of(attacker) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(defender) == domain::Zone::GRAVEYARD);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel emits BATTLE_DAMAGE with player and amount")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_battle_damage";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	std::ofstream attacker_stream{directory / "97000061.json", std::ios::binary};
	attacker_stream <<
			R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"BATTLE_DAMAGE"},"timing":"WHEN","optional":false,"condition":{"and":[{"eq":[{"call":"event_player","args":{}},1]},{"ge":[{"call":"event_amount","args":{}},200]}]}},"effect":[{"call":"recover_life","args":{"players":["SELF"],"amount":300}}]}]})";
	attacker_stream.close();

	std::ofstream watcher_stream{directory / "97000062.json", std::ios::binary};
	watcher_stream <<
			R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"BATTLE_DAMAGE"},"timing":"WHEN","optional":false,"condition":{"and":[{"eq":[{"call":"event_player","args":{}},0]},{"ge":[{"call":"event_amount","args":{}},200]}]}},"effect":[{"call":"recover_life","args":{"players":["SELF"],"amount":500}}]}]})";
	watcher_stream.close();

	constexpr domain::CardCode attacker_code{97000061};
	constexpr domain::CardCode watcher_code{97000062};
	constexpr domain::CardCode defender_code{97000063};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(attacker_code, make_monster(attacker_code, 1000, 1000));
	game.register_definition(watcher_code, make_monster(watcher_code, 1000, 1000));
	game.register_definition(defender_code, make_monster(defender_code, 800, 800));

	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto watcher = game.create_instance(watcher_code, domain::Player::FIRST);
	REQUIRE(watcher.valid());
	game.place(*game.find_instance_mutable(watcher), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	const auto defender = game.create_instance(defender_code, domain::Player::FIRST);
	REQUIRE(defender.valid());
	game.place(*game.find_instance_mutable(defender), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(attacker.valid());
	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	CHECK(duel.declare_attack(attacker, defender));
	resolve_attack(duel);

	// FIRST 受到 200 战斗伤害,其触发回复 500;SECOND 的触发回复 300
	CHECK(game.life_points(domain::Player::FIRST) == 8300);
	CHECK(game.life_points(domain::Player::SECOND) == 8300);
	CHECK(game.zone_of(defender) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	CHECK(game.zone_of(watcher) == domain::Zone::MONSTER);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel emits BATTLE_DAMAGE when the attacker's controller takes damage")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_battle_damage_controller";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	std::ofstream attacker_stream{directory / "97000071.json", std::ios::binary};
	attacker_stream <<
			R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":800,"defense":800}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"BATTLE_DAMAGE"},"timing":"WHEN","optional":false,"condition":{"and":[{"eq":[{"call":"event_player","args":{}},0]},{"ge":[{"call":"event_amount","args":{}},200]}]}},"effect":[{"call":"recover_life","args":{"players":["SELF"],"amount":300}}]}]})";
	attacker_stream.close();

	std::ofstream watcher_stream{directory / "97000072.json", std::ios::binary};
	watcher_stream <<
			R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"BATTLE_DAMAGE"},"timing":"WHEN","optional":false,"condition":{"and":[{"eq":[{"call":"event_player","args":{}},1]},{"ge":[{"call":"event_amount","args":{}},200]}]}},"effect":[{"call":"recover_life","args":{"players":["SELF"],"amount":500}}]}]})";
	watcher_stream.close();

	constexpr domain::CardCode attacker_code{97000071};
	constexpr domain::CardCode watcher_code{97000072};
	constexpr domain::CardCode defender_code{97000073};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(attacker_code, make_monster(attacker_code, 800, 800));
	game.register_definition(watcher_code, make_monster(watcher_code, 1000, 1000));
	game.register_definition(defender_code, make_monster(defender_code, 1000, 1000));

	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto watcher = game.create_instance(watcher_code, domain::Player::FIRST);
	REQUIRE(watcher.valid());
	game.place(*game.find_instance_mutable(watcher), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	const auto defender = game.create_instance(defender_code, domain::Player::FIRST);
	REQUIRE(defender.valid());
	game.place(*game.find_instance_mutable(defender), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_DEFENSE});

	const auto attacker = game.create_instance(attacker_code, domain::Player::SECOND);
	REQUIRE(attacker.valid());
	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	enter_second_player_battle(duel);
	REQUIRE(duel.turn().phase() == domain::Phase::BATTLE);

	CHECK(duel.declare_attack(attacker, defender));
	resolve_attack(duel);

	// SECOND 受到 200 战斗伤害,其触发回复 300;FIRST 的触发回复 500
	CHECK(game.life_points(domain::Player::FIRST) == 8500);
	CHECK(game.life_points(domain::Player::SECOND) == 8100);
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	CHECK(game.zone_of(defender) == domain::Zone::MONSTER);
	CHECK(game.zone_of(watcher) == domain::Zone::MONSTER);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel resolves a defense-position battle")
{
	constexpr domain::CardCode attacker_code{91000012};
	constexpr domain::CardCode defender_code{91000013};

	flow::Duel duel{g_selector};
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
	resolve_attack(duel);

	// 守备力更高:攻击方不破坏,受到差值伤害
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	CHECK(game.zone_of(defender) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::SECOND) == 7500);
	CHECK(game.life_points(domain::Player::FIRST) == 8000);
}

TEST_CASE("flow::Duel direct attack and winner")
{
	constexpr domain::CardCode attacker_code{91000014};

	flow::Duel duel{g_selector};
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
	resolve_attack(duel);

	CHECK(game.life_points(domain::Player::FIRST) == 0);
	CHECK(duel.winner() == domain::Player::SECOND);
}

TEST_CASE("flow::Duel flips a face-down defender before damage calculation")
{
	constexpr domain::CardCode attacker_code{91000015};
	constexpr domain::CardCode defender_code{91000016};

	flow::Duel duel{g_selector};
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
	resolve_attack(duel);

	// 里侧守备被翻面后因守备力较低被破坏,且不造成战斗伤害
	CHECK(game.zone_of(defender) == domain::Zone::GRAVEYARD);
	CHECK(game.life_points(domain::Player::FIRST) == 8000);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);
}

TEST_CASE("flow::Duel tribute summons a level 5 monster")
{
	constexpr domain::CardCode high_code{91000020};
	constexpr domain::CardCode low_code{91000021};

	flow::Duel duel{g_selector};
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
	const auto low_hand = game.create_instance(low_code, domain::Player::FIRST);
	REQUIRE(low_hand.valid());

	game.place(*game.find_instance_mutable(high), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(low_hand), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(tribute), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	// 等级 5 不能直接通常召唤
	CHECK_FALSE(duel.normal_summon(high, domain::FieldColumn::C2));

	// 等级 4 不是上级召唤,不能以 0 祭品走 tribute_summon
	CHECK_FALSE(duel.tribute_summon(low_hand, domain::FieldColumn::C3, std::span<const state::CardInstanceId>{}));

	const std::array tributes{tribute};
	CHECK(duel.tribute_summon(high, domain::FieldColumn::C2, tributes));
	resolve_summon(duel);

	CHECK(game.zone_of(tribute) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(high) == domain::Zone::MONSTER);
}

TEST_CASE("flow::Duel requires two tributes for a level 7 monster")
{
	constexpr domain::CardCode high_code{91000022};
	constexpr domain::CardCode low_code{91000023};

	flow::Duel duel{g_selector};
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
	resolve_summon(duel);

	CHECK(game.zone_of(first) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(second) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(high) == domain::Zone::MONSTER);
}

TEST_CASE("flow::Duel loses when drawing from an empty deck")
{
	flow::Duel duel{g_selector};

	CHECK_FALSE(duel.draw_for_turn());
	CHECK(duel.winner() == domain::Player::SECOND);
}

TEST_CASE("flow::Duel requires both players to pass before advancing phase")
{
	flow::Duel duel{g_selector};

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

	flow::Duel duel{g_selector};
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
	settle(duel);

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

	flow::Duel duel{g_selector};
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
	resolve_attack(duel);

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
		flow::Duel duel{g_selector};
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
		settle(duel);

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
		R"({"identity":{"kind":"SPELL","subtypes":["FIELD"]},"effects":[{"type":"static","apply_to":{"call":"is_card_type","args":{"card":{"call":"$"},"card_type":"MONSTER"}},"modifiers":[{"call":"add_attribute","args":{"attribute":"DARK"}},{"call":"change_level","args":{"delta":1}},{"call":"add_card_subtype","args":{"subtype":"TUNER"}}]}]})"
	);
	write(
		"99500008.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"WIND","race":"DRAGON","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"ignition"},"condition":{"and":[{"call":"is_attribute","args":{"card":{"call":"this_card","args":{}},"attribute":"WIND"}},{"call":"is_attribute","args":{"card":{"call":"this_card","args":{}},"attribute":"DARK"}},{"ge":[{"call":"level","args":{"card":{"call":"this_card","args":{}}}},5]},{"call":"is_card_subtype","args":{"card":{"call":"this_card","args":{}},"subtype":"TUNER"}}]},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})"
	);

	constexpr domain::CardCode field_code{99500007};
	constexpr domain::CardCode monster_code{99500008};

	const auto run = [&](const bool with_field) -> std::pair<bool, std::underlying_type_t<domain::AttackDefense>>
	{
		flow::Duel duel{g_selector};
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
		settle(duel);

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

TEST_CASE("flow::Duel validates Synchro and Xyz summon procedures")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_synchro_xyz";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	// 合法同调:调整 + 通常怪兽,等级合计 8
	write(
		"99600003.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["SYNCHRO"],"monster":{"attribute":"EARTH","race":"DRAGON","level":8,"attack":2500,"defense":2000}},"summons":[{"method":"synchro","tuner":{"call":"is_card_subtype","args":{"card":{"call":"$"},"subtype":"TUNER"}},"non_tuner":{"call":"is_card_subtype","args":{"card":{"call":"$"},"subtype":"NORMAL"}}}]})"
	);
	// 受限同调:要求 non_tuner 为 EFFECT;测试素材中的通常怪兽不满足
	write(
		"99600007.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["SYNCHRO"],"monster":{"attribute":"EARTH","race":"DRAGON","level":8,"attack":2500,"defense":2000}},"summons":[{"method":"synchro","tuner":{"call":"is_card_subtype","args":{"card":{"call":"$"},"subtype":"TUNER"}},"non_tuner":{"call":"is_card_subtype","args":{"card":{"call":"$"},"subtype":"EFFECT"}}}]})"
	);
	// 合法超量:2 只怪兽,同等级 4
	write(
		"99600004.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["XYZ"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000}},"summons":[{"method":"xyz","material":{"call":"is_card_type","args":{"card":{"call":"$"},"card_type":"MONSTER"}},"count":2}]})"
	);
	// 受限超量:要求素材为 TUNER;测试素材不满足
	write(
		"99600008.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["XYZ"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000}},"summons":[{"method":"xyz","material":{"call":"is_card_subtype","args":{"card":{"call":"$"},"subtype":"TUNER"}},"count":2}]})"
	);

	constexpr domain::CardCode tuner_code{99600001};
	constexpr domain::CardCode non_tuner_code{99600002};
	constexpr domain::CardCode synchro_code{99600003};
	constexpr domain::CardCode xyz_code{99600004};
	constexpr domain::CardCode material_a_code{99600005};
	constexpr domain::CardCode material_b_code{99600006};
	constexpr domain::CardCode synchro_restricted_code{99600007};
	constexpr domain::CardCode xyz_restricted_code{99600008};

	flow::Duel duel{g_selector};
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
	auto synchro_restricted = make_monster(synchro_restricted_code, 2500, 2000, 8);
	synchro_restricted.subtypes = {domain::CardSubtype::SYNCHRO};
	auto xyz_restricted = make_monster(xyz_restricted_code, 2000, 2000, 4);
	xyz_restricted.subtypes = {domain::CardSubtype::XYZ};

	game.register_definition(tuner_code, tuner);
	game.register_definition(non_tuner_code, non_tuner);
	game.register_definition(synchro_code, synchro);
	game.register_definition(xyz_code, xyz);
	game.register_definition(material_a_code, material_a);
	game.register_definition(material_b_code, material_b);
	game.register_definition(synchro_restricted_code, synchro_restricted);
	game.register_definition(xyz_restricted_code, xyz_restricted);

	REQUIRE(duel.load_scripts(directory));

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
	const auto synchro_restricted_card = game.create_instance(synchro_restricted_code, domain::Player::FIRST);
	REQUIRE(synchro_restricted_card.valid());
	game.place(*game.find_instance_mutable(synchro_restricted_card), domain::ExtraDeckPlacement{.index = 2, .position = domain::Position::FACE_DOWN_DEFENSE});
	const auto xyz_restricted_card = game.create_instance(xyz_restricted_code, domain::Player::FIRST);
	REQUIRE(xyz_restricted_card.valid());
	game.place(*game.find_instance_mutable(xyz_restricted_card), domain::ExtraDeckPlacement{.index = 3, .position = domain::Position::FACE_DOWN_DEFENSE});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	const std::array synchro_materials{tuner_card, non_tuner_card};
	const std::array xyz_materials{material_a_card, material_b_card};

	// 硬编码规则会接受这些素材,但脚本 procedure 的素材谓词不满足
	CHECK_FALSE(duel.synchro_summon(synchro_restricted_card, domain::MonsterPlacement{.column = domain::FieldColumn::C5, .position = domain::Position::FACE_UP_ATTACK}, synchro_materials));

	// 没有 Tuner 的素材组合
	const std::array invalid_synchro_materials{non_tuner_card, material_a_card};
	CHECK_FALSE(duel.synchro_summon(synchro_card, domain::MonsterPlacement{.column = domain::FieldColumn::C5, .position = domain::Position::FACE_UP_ATTACK}, invalid_synchro_materials));

	CHECK(duel.synchro_summon(synchro_card, domain::MonsterPlacement{.column = domain::FieldColumn::C5, .position = domain::Position::FACE_UP_ATTACK}, synchro_materials));
	resolve_summon(duel);
	CHECK(game.zone_of(synchro_card) == domain::Zone::MONSTER);
	CHECK(game.zone_of(tuner_card) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(non_tuner_card) == domain::Zone::GRAVEYARD);
	CHECK(game.find_instance(synchro_card)->summon_record().method == domain::SummonMethod::SYNCHRO);

	CHECK_FALSE(duel.xyz_summon(xyz_restricted_card, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, xyz_materials));

	// 素材数量不足(procedure count = 2)
	const std::array one_material{material_a_card};
	CHECK_FALSE(duel.xyz_summon(xyz_card, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, one_material));

	CHECK(duel.xyz_summon(xyz_card, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, xyz_materials));
	resolve_summon(duel);
	CHECK(game.zone_of(xyz_card) == domain::Zone::EXTRA_MONSTER);
	CHECK(game.zone_of(material_a_card) == domain::Zone::NOT_EXIST);
	CHECK(game.zone_of(material_b_card) == domain::Zone::NOT_EXIST);
	CHECK(game.find_instance(xyz_card)->overlays().size() == 2);
	CHECK(game.find_instance(xyz_card)->summon_record().method == domain::SummonMethod::XYZ);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel resolves Fusion, Link, Ritual, and SelfSpecial summons")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_extra_summon_paths";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"99700010.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["FUSION"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000}},"summons":[{"method":"fusion","materials":[{"card":99700011},{"card":99700012}]}]})"
	);
	write(
		"99700020.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["LINK"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000}},"summons":[{"method":"link","material":{"call":"is_card_type","args":{"card":{"call":"$"},"card_type":"MONSTER"}},"count":2}]})"
	);
	write(
		"99700030.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["RITUAL"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000}},"summons":[{"method":"ritual","material":{"call":"is_card_type","args":{"card":{"call":"$"},"card_type":"MONSTER"}},"level":4}]})"
	);
	write(
		"99700040.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["SPECIAL_SUMMON"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000}},"summons":[{"method":"special","condition":true,"cost":[],"source_zones":["HAND"]}]})"
	);

	constexpr domain::CardCode fusion_code{99700010};
	constexpr domain::CardCode fusion_material_a{99700011};
	constexpr domain::CardCode fusion_material_b{99700012};
	constexpr domain::CardCode link_code{99700020};
	constexpr domain::CardCode link_material_a{99700021};
	constexpr domain::CardCode link_material_b{99700022};
	constexpr domain::CardCode ritual_code{99700030};
	constexpr domain::CardCode ritual_material_a{99700031};
	constexpr domain::CardCode ritual_material_b{99700032};
	constexpr domain::CardCode self_code{99700040};

	const auto prepare = [&](flow::Duel& duel) -> void
	{
		auto& game = duel.game();

		auto fusion = make_monster(fusion_code, 2000, 2000, 4);
		fusion.subtypes = {domain::CardSubtype::FUSION};
		auto link = make_monster(link_code, 2000, 2000, 4);
		link.subtypes = {domain::CardSubtype::LINK};
		auto ritual = make_monster(ritual_code, 2000, 2000, 4);
		ritual.subtypes = {domain::CardSubtype::RITUAL};
		auto self = make_monster(self_code, 2000, 2000, 4);
		self.subtypes = {domain::CardSubtype::SPECIAL_SUMMON};

		game.register_definition(fusion_code, fusion);
		game.register_definition(fusion_material_a, make_monster(fusion_material_a, 1000, 1000, 2));
		game.register_definition(fusion_material_b, make_monster(fusion_material_b, 1000, 1000, 2));
		game.register_definition(link_code, link);
		game.register_definition(link_material_a, make_monster(link_material_a, 1000, 1000, 2));
		game.register_definition(link_material_b, make_monster(link_material_b, 1000, 1000, 2));
		game.register_definition(ritual_code, ritual);
		game.register_definition(ritual_material_a, make_monster(ritual_material_a, 1000, 1000, 2));
		game.register_definition(ritual_material_b, make_monster(ritual_material_b, 1000, 1000, 2));
		game.register_definition(self_code, self);

		REQUIRE(duel.load_scripts(directory));
		game.set_life_points(domain::Player::FIRST, 8000);
		game.set_life_points(domain::Player::SECOND, 8000);
	};

	SUBCASE("fusion")
	{
		flow::Duel duel{g_selector};
		prepare(duel);
		auto& game = duel.game();

		const auto fusion = game.create_instance(fusion_code, domain::Player::FIRST);
		REQUIRE(fusion.valid());
		game.place(*game.find_instance_mutable(fusion), domain::ExtraDeckPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

		const auto material_a = game.create_instance(fusion_material_a, domain::Player::FIRST);
		REQUIRE(material_a.valid());
		game.place(*game.find_instance_mutable(material_a), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});
		const auto material_b = game.create_instance(fusion_material_b, domain::Player::FIRST);
		REQUIRE(material_b.valid());
		game.place(*game.find_instance_mutable(material_b), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

		advance_phase(duel);
		advance_phase(duel);

		const std::array one_material{material_a};
		CHECK_FALSE(duel.fusion_summon(fusion, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, one_material));

		const std::array materials{material_a, material_b};
		CHECK(duel.fusion_summon(fusion, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, materials));
		resolve_summon(duel);

		CHECK(game.zone_of(fusion) == domain::Zone::EXTRA_MONSTER);
		CHECK(game.zone_of(material_a) == domain::Zone::GRAVEYARD);
		CHECK(game.zone_of(material_b) == domain::Zone::GRAVEYARD);
		CHECK(game.find_instance(fusion)->summon_record().method == domain::SummonMethod::FUSION);
	}

	SUBCASE("link")
	{
		flow::Duel duel{g_selector};
		prepare(duel);
		auto& game = duel.game();

		const auto link = game.create_instance(link_code, domain::Player::FIRST);
		REQUIRE(link.valid());
		game.place(*game.find_instance_mutable(link), domain::ExtraDeckPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

		const auto material_a = game.create_instance(link_material_a, domain::Player::FIRST);
		REQUIRE(material_a.valid());
		game.place(*game.find_instance_mutable(material_a), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});
		const auto material_b = game.create_instance(link_material_b, domain::Player::FIRST);
		REQUIRE(material_b.valid());
		game.place(*game.find_instance_mutable(material_b), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

		advance_phase(duel);
		advance_phase(duel);

		const std::array one_material{material_a};
		CHECK_FALSE(duel.link_summon(link, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, one_material));

		const std::array materials{material_a, material_b};
		CHECK(duel.link_summon(link, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, materials));
		resolve_summon(duel);

		CHECK(game.zone_of(link) == domain::Zone::EXTRA_MONSTER);
		CHECK(game.zone_of(material_a) == domain::Zone::GRAVEYARD);
		CHECK(game.zone_of(material_b) == domain::Zone::GRAVEYARD);
		CHECK(game.find_instance(link)->summon_record().method == domain::SummonMethod::LINK);
	}

	SUBCASE("ritual")
	{
		flow::Duel duel{g_selector};
		prepare(duel);
		auto& game = duel.game();

		const auto ritual = game.create_instance(ritual_code, domain::Player::FIRST);
		REQUIRE(ritual.valid());
		game.place(*game.find_instance_mutable(ritual), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

		const auto material_a = game.create_instance(ritual_material_a, domain::Player::FIRST);
		REQUIRE(material_a.valid());
		game.place(*game.find_instance_mutable(material_a), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});
		const auto material_b = game.create_instance(ritual_material_b, domain::Player::FIRST);
		REQUIRE(material_b.valid());
		game.place(*game.find_instance_mutable(material_b), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

		advance_phase(duel);
		advance_phase(duel);

		const std::array one_material{material_a};
		CHECK_FALSE(duel.ritual_summon(ritual, domain::MonsterPlacement{.column = domain::FieldColumn::C3, .position = domain::Position::FACE_UP_ATTACK}, one_material));

		const std::array materials{material_a, material_b};
		CHECK(duel.ritual_summon(ritual, domain::MonsterPlacement{.column = domain::FieldColumn::C3, .position = domain::Position::FACE_UP_ATTACK}, materials));
		resolve_summon(duel);

		CHECK(game.zone_of(ritual) == domain::Zone::MONSTER);
		CHECK(game.zone_of(material_a) == domain::Zone::GRAVEYARD);
		CHECK(game.zone_of(material_b) == domain::Zone::GRAVEYARD);
		CHECK(game.find_instance(ritual)->summon_record().method == domain::SummonMethod::RITUAL);
	}

	SUBCASE("self special")
	{
		flow::Duel duel{g_selector};
		prepare(duel);
		auto& game = duel.game();

		const auto self = game.create_instance(self_code, domain::Player::FIRST);
		REQUIRE(self.valid());
		game.place(*game.find_instance_mutable(self), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

		advance_phase(duel);
		advance_phase(duel);

		CHECK(duel.self_special_summon(self, domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK}));
		resolve_summon(duel);

		CHECK(game.zone_of(self) == domain::Zone::MONSTER);
		CHECK(game.find_instance(self)->summon_record().method == domain::SummonMethod::SPECIAL);
	}

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel: Solemn Judgment negates a Normal Summon")
{
	constexpr domain::CardCode solemn_code{41420027};
	constexpr domain::CardCode monster_code{97000092};
	constexpr domain::CardCode second_code{97000093};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(solemn_code, make_trap(solemn_code, {domain::CardSubtype::COUNTER}));
	game.register_definition(monster_code, make_monster(monster_code));
	game.register_definition(second_code, make_monster(second_code));

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto solemn = game.create_instance(solemn_code, domain::Player::FIRST);
	REQUIRE(solemn.valid());
	game.place(*game.find_instance_mutable(solemn), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto monster = game.create_instance(monster_code, domain::Player::SECOND);
	REQUIRE(monster.valid());
	game.place(*game.find_instance_mutable(monster), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto second = game.create_instance(second_code, domain::Player::SECOND);
	REQUIRE(second.valid());
	game.place(*game.find_instance_mutable(second), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(duel.set_spell_trap(solemn, domain::FieldColumn::C1));
	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1

	CHECK(duel.normal_summon(monster, domain::FieldColumn::C1));
	CHECK(duel.pending_summon());
	CHECK_FALSE(duel.pass_priority());

	CHECK(duel.activate_card(solemn));
	CHECK(game.life_points(domain::Player::FIRST) == 4000);
	resolve_summon(duel);

	CHECK_FALSE(duel.pending_summon());
	CHECK(game.zone_of(monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(solemn) == domain::Zone::GRAVEYARD);
	CHECK(game.life_points(domain::Player::FIRST) == 4000);

	// 被无效的通常召唤仍消耗本回合的通招
	CHECK_FALSE(duel.normal_summon(second, domain::FieldColumn::C1));
	CHECK(game.zone_of(second) == domain::Zone::HAND);
}

TEST_CASE("flow::Duel: Solemn Judgment negates an Xyz Summon")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_solemn_xyz";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"97000095.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["XYZ"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000}},"summons":[{"method":"xyz","material":{"call":"is_card_type","args":{"card":{"call":"$"},"card_type":"MONSTER"}},"count":2}]})"
	);

	constexpr domain::CardCode solemn_code{41420027};
	constexpr domain::CardCode xyz_code{97000095};
	constexpr domain::CardCode material_a_code{97000096};
	constexpr domain::CardCode material_b_code{97000097};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(solemn_code, make_trap(solemn_code, {domain::CardSubtype::COUNTER}));
	auto xyz_definition = make_monster(xyz_code, 2000, 2000, 4);
	xyz_definition.subtypes = {domain::CardSubtype::XYZ};
	game.register_definition(xyz_code, xyz_definition);
	game.register_definition(material_a_code, make_monster(material_a_code, 1000, 1000, 4));
	game.register_definition(material_b_code, make_monster(material_b_code, 1000, 1000, 4));

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto solemn = game.create_instance(solemn_code, domain::Player::FIRST);
	REQUIRE(solemn.valid());
	game.place(*game.find_instance_mutable(solemn), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto xyz_card = game.create_instance(xyz_code, domain::Player::SECOND);
	REQUIRE(xyz_card.valid());
	game.place(*game.find_instance_mutable(xyz_card), domain::ExtraDeckPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto material_a = game.create_instance(material_a_code, domain::Player::SECOND);
	REQUIRE(material_a.valid());
	game.place(*game.find_instance_mutable(material_a), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});
	const auto material_b = game.create_instance(material_b_code, domain::Player::SECOND);
	REQUIRE(material_b.valid());
	game.place(*game.find_instance_mutable(material_b), domain::MonsterPlacement{.column = domain::FieldColumn::C3, .position = domain::Position::FACE_UP_ATTACK});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(duel.set_spell_trap(solemn, domain::FieldColumn::C1));
	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1

	const std::array materials{material_a, material_b};
	CHECK(duel.xyz_summon(xyz_card, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, materials));
	CHECK(duel.pending_summon());

	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.activate_card(solemn));
	CHECK(game.life_points(domain::Player::FIRST) == 4000);
	resolve_summon(duel);

	CHECK_FALSE(duel.pending_summon());
	CHECK(game.zone_of(xyz_card) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(material_a) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(material_b) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(solemn) == domain::Zone::GRAVEYARD);
	CHECK(game.find_instance(xyz_card)->overlays().empty());

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel: Solemn Judgment negates and destroys a Spell activation")
{
	constexpr domain::CardCode solemn_code{41420027};
	constexpr domain::CardCode raigeki_code{12580477};
	constexpr domain::CardCode monster_code{97000092};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(solemn_code, make_trap(solemn_code, {domain::CardSubtype::COUNTER}));
	game.register_definition(raigeki_code, make_spell(raigeki_code, {domain::CardSubtype::NORMAL}));
	game.register_definition(monster_code, make_monster(monster_code));

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto solemn = game.create_instance(solemn_code, domain::Player::FIRST);
	REQUIRE(solemn.valid());
	game.place(*game.find_instance_mutable(solemn), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(monster.valid());
	game.place(*game.find_instance_mutable(monster), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto raigeki = game.create_instance(raigeki_code, domain::Player::SECOND);
	REQUIRE(raigeki.valid());
	game.place(*game.find_instance_mutable(raigeki), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(duel.set_spell_trap(solemn, domain::FieldColumn::C1));
	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1

	CHECK(duel.activate_card(raigeki));
	CHECK(duel.chain().size() == 1);
	CHECK(game.zone_of(raigeki) == domain::Zone::SPELL_TRAP);

	CHECK(duel.activate_card(solemn));
	CHECK(duel.chain().size() == 2);
	CHECK(game.life_points(domain::Player::FIRST) == 4000);

	settle(duel);

	CHECK(game.zone_of(raigeki) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(solemn) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(monster) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 4000);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);
}

TEST_CASE("flow::Duel: Solemn Judgment negates and destroys a monster effect")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_solemn_monster_effect";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"97000099.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"quick"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})"
	);

	constexpr domain::CardCode solemn_code{41420027};
	constexpr domain::CardCode monster_code{97000099};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(solemn_code, make_trap(solemn_code, {domain::CardSubtype::COUNTER}));
	game.register_definition(monster_code, make_monster(monster_code));

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto solemn = game.create_instance(solemn_code, domain::Player::FIRST);
	REQUIRE(solemn.valid());
	game.place(*game.find_instance_mutable(solemn), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto monster = game.create_instance(monster_code, domain::Player::SECOND);
	REQUIRE(monster.valid());
	game.place(*game.find_instance_mutable(monster), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(duel.set_spell_trap(solemn, domain::FieldColumn::C1));
	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1

	CHECK(duel.activate_effect(monster, 0));
	CHECK(duel.chain().size() == 1);

	CHECK(duel.activate_card(solemn));
	CHECK(duel.chain().size() == 2);
	CHECK(game.life_points(domain::Player::FIRST) == 4000);

	settle(duel);

	// 无效发动并破坏来源怪兽;不是仅仅无效效果
	CHECK(game.zone_of(monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(solemn) == domain::Zone::GRAVEYARD);
	CHECK(game.life_points(domain::Player::FIRST) == 4000);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);

	std::filesystem::remove_all(directory);
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

	flow::Duel duel{g_selector};
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
	resolve_attack(duel);

	CHECK(game.zone_of(defender) == domain::Zone::MONSTER);
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 7800);
}

TEST_CASE("flow::Duel lets the driver choose battle destruction triggers")
{
	constexpr domain::CardCode rat_code{97017120};
	constexpr domain::CardCode attacker_code{96000001};
	constexpr domain::CardCode recruit_code{96000002};

	// 全选触发(含选发),用于验证驱动选择后 Giant Rat 会特召
	driver::LocalTestSelector auto_selector
	{
			nullptr,
			[](const execution::TriggerDecision& decision) -> std::vector<std::size_t>
			{
				std::vector<std::size_t> indices(decision.candidates.size());
				std::ranges::iota(indices, std::size_t{0});
				return indices;
			}
	};

	flow::Duel duel{auto_selector};
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
	resolve_attack(duel);

	// 战破后 Giant Rat 的选发触发由驱动选择并结算
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

	flow::Duel duel{g_selector};
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
	settle(duel);

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

	flow::Duel duel{g_selector};
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
	settle(duel);
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

	flow::Duel duel{g_selector};
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
	settle(duel);
	CHECK(duel.activate_effect(card, 0));
	settle(duel);
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
	settle(duel);
	CHECK(duel.activate_effect(card, 1));
	settle(duel);
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

	flow::Duel duel{g_selector};
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

	flow::Duel duel{g_selector};
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
	settle(duel);

	CHECK(game.zone_of(victim_a) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(victim_b) == domain::Zone::GRAVEYARD);
	CHECK(game.count_zone_card(domain::Player::SECOND, domain::Zone::HAND) == 1);
	CHECK(game.count_zone_card(domain::Player::SECOND, domain::Zone::DECK) == 1);
}

TEST_CASE("flow::Duel activates traps with an event timing")
{
	constexpr domain::CardCode torrential_code{53582587};
	constexpr domain::CardCode monster_code{98000001};

	flow::Duel duel{g_selector};
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
	resolve_summon(duel);

	// 先攻放弃优先权 → 后攻可发动带 WHEN 时点的陷阱
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.activate_card(torrential));
	settle(duel);

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

	flow::Duel duel{g_selector};
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

	// 启动效果:回合玩家、主要阶段 → 入连锁但尚未结算
	CHECK(duel.activate_effect(ignition_card, 0));
	CHECK(duel.chain().size() == 1);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);

	// 发动后优先权交给对方 → 对方的诱发即时可以连锁
	CHECK(duel.activate_effect(quick_card, 0));
	CHECK(duel.chain().size() == 2);

	// 回合玩家放弃 → 逆序结算:先 quick 再 ignition
	CHECK(duel.pass_priority());
	CHECK(duel.chain().empty());
	CHECK(game.life_points(domain::Player::FIRST) == 7500);
	CHECK(game.life_points(domain::Player::SECOND) == 7500);

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

	flow::Duel duel{g_selector};
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
	settle(duel);
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
	settle(duel);
	CHECK(game.life_points(domain::Player::SECOND) == 7000);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel activates spell cards")
{
	constexpr domain::CardCode raigeki_code{12580477};
	constexpr domain::CardCode mountain_code{50913601};
	constexpr domain::CardCode monster_code{96000001};

	flow::Duel duel{g_selector};
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
	settle(duel);
	CHECK(game.zone_of(monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(raigeki) == domain::Zone::GRAVEYARD);

	// 场地魔法:留场
	CHECK(duel.activate_card(mountain));
	settle(duel);
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

	flow::Duel duel{g_selector};
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
	settle(duel);

	CHECK(game.life_points(domain::Player::SECOND) == 7500);
	CHECK(game.zone_of(trap) == domain::Zone::GRAVEYARD);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel resolves a chain in reverse order and negates effects")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_chain_negate";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"94100001.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"quick"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})"
	);
	write(
		"94100002.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"quick"},"effect":[{"call":"negate_effect","args":{}}]}]})"
	);

	constexpr domain::CardCode attacker_code{94100001};
	constexpr domain::CardCode negator_code{94100002};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(attacker_code, make_monster(attacker_code));
	game.register_definition(negator_code, make_monster(negator_code));
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto attacker = game.create_instance(attacker_code, domain::Player::FIRST);
	REQUIRE(attacker.valid());
	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto negator = game.create_instance(negator_code, domain::Player::SECOND);
	REQUIRE(negator.valid());
	game.place(*game.find_instance_mutable(negator), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	// C1:回合玩家的诱发即时
	CHECK(duel.activate_effect(attacker, 0));
	REQUIRE(duel.chain().size() == 1);
	CHECK(duel.chain().front().controller() == domain::Player::FIRST);

	// C2:对方的诱发即时(速度 2,可连锁)
	CHECK(duel.activate_effect(negator, 0));
	REQUIRE(duel.chain().size() == 2);
	CHECK(duel.chain().back().controller() == domain::Player::SECOND);

	// 旧时点已关闭
	CHECK(duel.windows().empty());

	// 逆序结算:C2 先无效 C1 的效果
	CHECK(duel.pass_priority());
	CHECK(duel.chain().empty());
	CHECK(game.life_points(domain::Player::FIRST) == 8000);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel negates an activation and sends the spell to the graveyard")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_chain_negate_activation";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"94200001.json",
		R"({"identity":{"kind":"SPELL","subtypes":["NORMAL"]},"effects":[{"type":"activated","activation":{"mode":"activation"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})"
	);
	write(
		"94200002.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"quick"},"effect":[{"call":"negate_activation","args":{}}]}]})"
	);

	constexpr domain::CardCode spell_code{94200001};
	constexpr domain::CardCode negator_code{94200002};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(spell_code, make_spell(spell_code, {domain::CardSubtype::NORMAL}));
	game.register_definition(negator_code, make_monster(negator_code));
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto negator = game.create_instance(negator_code, domain::Player::SECOND);
	REQUIRE(negator.valid());
	game.place(*game.find_instance_mutable(negator), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	// 卡发动先放到场上,再进入连锁
	CHECK(duel.activate_card(spell));
	CHECK(game.zone_of(spell) == domain::Zone::SPELL_TRAP);

	CHECK(duel.activate_effect(negator, 0));
	REQUIRE(duel.chain().size() == 2);

	CHECK(duel.pass_priority());
	CHECK(duel.chain().empty());

	// 发动无效:魔法卡送墓,且不造成伤害
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(negator) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel rolls back usage for negated activations")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_chain_usage_rollback";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"94200003.json",
		R"({"identity":{"kind":"SPELL","subtypes":["NORMAL"]},"effects":[{"type":"activated","usage":{"kind":"TURN_PER_NAME","verb":"ACTIVATE"},"activation":{"mode":"activation"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})"
	);
	write(
		"94200004.json",
		R"({"identity":{"kind":"SPELL","subtypes":["NORMAL"]},"effects":[{"type":"activated","usage":{"kind":"TURN_PER_NAME","verb":"USE"},"activation":{"mode":"activation"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})"
	);
	write(
		"94200005.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"quick"},"effect":[{"call":"negate_activation","args":{}}]}]})"
	);

	constexpr domain::CardCode activate_code{94200003};
	constexpr domain::CardCode use_code{94200004};
	constexpr domain::CardCode negator_code{94200005};

	const auto run = [&](const domain::CardCode spell_code, const bool expect_second) -> void
	{
		flow::Duel duel{g_selector};
		auto& game = duel.game();

		game.register_definition(activate_code, make_spell(activate_code, {domain::CardSubtype::NORMAL}));
		game.register_definition(use_code, make_spell(use_code, {domain::CardSubtype::NORMAL}));
		game.register_definition(negator_code, make_monster(negator_code));
		REQUIRE(duel.load_scripts(directory));

		game.set_life_points(domain::Player::FIRST, 8000);
		game.set_life_points(domain::Player::SECOND, 8000);

		const auto negator = game.create_instance(negator_code, domain::Player::SECOND);
		REQUIRE(negator.valid());
		game.place(*game.find_instance_mutable(negator), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

		const auto first = game.create_instance(spell_code, domain::Player::FIRST);
		REQUIRE(first.valid());
		game.place(*game.find_instance_mutable(first), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

		const auto second = game.create_instance(spell_code, domain::Player::FIRST);
		REQUIRE(second.valid());
		game.place(*game.find_instance_mutable(second), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});

		advance_phase(duel); // STANDBY
		advance_phase(duel); // MAIN_1
		REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

		CHECK(duel.activate_card(first));
		CHECK(duel.activate_effect(negator, 0));
		CHECK(duel.pass_priority());

		// ACTIVATE:发动被无效可再发动;USE:仍然算使用过
		CHECK(duel.activate_card(second) == expect_second);

		if (expect_second)
		{
			settle(duel);
		}

		CHECK(game.life_points(domain::Player::SECOND) == (expect_second ? 7500 : 8000));
	};

	run(activate_code, true);
	run(use_code, false);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel enforces spell speed when chaining")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_chain_speed";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"94300001.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"quick"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":500}}]}]})"
	);
	write(
		"94300002.json",
		R"({"identity":{"kind":"TRAP","subtypes":["COUNTER"]},"effects":[{"type":"activated","activation":{"mode":"activation"},"effect":[{"call":"negate_effect","args":{}}]}]})"
	);
	write(
		"94300003.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"quick"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":300}}]}]})"
	);

	constexpr domain::CardCode first_quick_code{94300001};
	constexpr domain::CardCode counter_code{94300002};
	constexpr domain::CardCode second_quick_code{94300003};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(first_quick_code, make_monster(first_quick_code));
	game.register_definition(counter_code, make_trap(counter_code, {domain::CardSubtype::COUNTER}));
	game.register_definition(second_quick_code, make_monster(second_quick_code));
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto first_quick = game.create_instance(first_quick_code, domain::Player::FIRST);
	REQUIRE(first_quick.valid());
	game.place(*game.find_instance_mutable(first_quick), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto second_quick = game.create_instance(second_quick_code, domain::Player::FIRST);
	REQUIRE(second_quick.valid());
	game.place(*game.find_instance_mutable(second_quick), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	const auto counter = game.create_instance(counter_code, domain::Player::SECOND);
	REQUIRE(counter.valid());
	game.place(*game.find_instance_mutable(counter), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	// 先攻空过
	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	advance_phase(duel); // END
	REQUIRE(duel.end_turn());

	// 后攻盖放反击陷阱
	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.set_spell_trap(counter, domain::FieldColumn::C1));
	advance_phase(duel); // BATTLE
	advance_phase(duel); // MAIN_2
	advance_phase(duel); // END
	REQUIRE(duel.end_turn());

	// 先攻回合:速度 2 → 速度 3 → 速度 2 不能继续
	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	CHECK(duel.activate_effect(first_quick, 0));
	CHECK(duel.activate_card(counter));
	REQUIRE(duel.chain().size() == 2);
	CHECK(duel.chain().back().speed() == domain::EffectSpeed::COUNTER);
	CHECK_FALSE(duel.activate_effect(second_quick, 0));

	CHECK(duel.pass_priority());
	CHECK(duel.chain().empty());
	CHECK(game.life_points(domain::Player::SECOND) == 8000);
	CHECK(game.zone_of(counter) == domain::Zone::GRAVEYARD);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel collects simultaneous triggers into one chain")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_chain_segoc";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"94400001.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"ignition"},"effect":[{"call":"destroy","args":{"cards":{"call":"find_cards","args":{"players":["SELF","OPPONENT"],"zones":["MONSTER","EXTRA_MONSTER"]}},"reason":"EFFECT"}}]}]})"
	);
	write(
		"94400002.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"DESTROYED"},"timing":"WHEN","optional":false},"effect":[{"call":"draw","args":{"players":["SELF"],"count":1}}]}]})"
	);

	constexpr domain::CardCode destroyer_code{94400001};
	constexpr domain::CardCode victim_code{94400002};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(destroyer_code, make_monster(destroyer_code));
	game.register_definition(victim_code, make_monster(victim_code));
	REQUIRE(duel.load_scripts(directory));

	const auto destroyer = game.create_instance(destroyer_code, domain::Player::FIRST);
	REQUIRE(destroyer.valid());
	game.place(*game.find_instance_mutable(destroyer), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto first_victim = game.create_instance(victim_code, domain::Player::FIRST);
	REQUIRE(first_victim.valid());
	game.place(*game.find_instance_mutable(first_victim), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	const auto second_victim = game.create_instance(victim_code, domain::Player::SECOND);
	REQUIRE(second_victim.valid());
	game.place(*game.find_instance_mutable(second_victim), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto first_deck = game.create_instance(victim_code, domain::Player::FIRST);
	REQUIRE(first_deck.valid());
	game.place(*game.find_instance_mutable(first_deck), domain::DeckPlacement{.index = 0});

	const auto second_deck = game.create_instance(victim_code, domain::Player::SECOND);
	REQUIRE(second_deck.valid());
	game.place(*game.find_instance_mutable(second_deck), domain::DeckPlacement{.index = 0});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_effect(destroyer, 0));
	REQUIRE(duel.chain().size() == 1);

	// 一组破坏链结算后,两个 DESTROYED 事件应组成同一组连锁
	CHECK(duel.pass_priority());
	REQUIRE(duel.chain().size() == 2);
	CHECK(duel.chain()[0].controller() == domain::Player::FIRST);
	CHECK(duel.chain()[1].controller() == domain::Player::SECOND);

	settle(duel);

	CHECK(game.zone_of(destroyer) == domain::Zone::GRAVEYARD);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == 1);
	CHECK(game.count_zone_card(domain::Player::SECOND, domain::Zone::HAND) == 1);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::DECK) == 0);
	CHECK(game.count_zone_card(domain::Player::SECOND, domain::Zone::DECK) == 0);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel allows different effects of one card in one chain")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_chain_same_card";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"94500001.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"quick"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":100}}]},{"type":"activated","activation":{"mode":"quick"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":200}}]}]})"
	);
	write(
		"94500002.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"quick"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":300}}]},{"type":"activated","activation":{"mode":"quick"},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":400}}]}]})"
	);

	constexpr domain::CardCode double_code{94500001};
	constexpr domain::CardCode opponent_code{94500002};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(double_code, make_monster(double_code));
	game.register_definition(opponent_code, make_monster(opponent_code));
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto double_card = game.create_instance(double_code, domain::Player::FIRST);
	REQUIRE(double_card.valid());
	game.place(*game.find_instance_mutable(double_card), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto opponent_card = game.create_instance(opponent_code, domain::Player::SECOND);
	REQUIRE(opponent_card.valid());
	game.place(*game.find_instance_mutable(opponent_card), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	// C1:同一张卡的效果0
	CHECK(duel.activate_effect(double_card, 0));

	// C2:对方连锁;优先权回到回合玩家
	CHECK(duel.activate_effect(opponent_card, 0));

	// C3:同一张卡的不同效果 → 允许
	CHECK(duel.activate_effect(double_card, 1));

	// C4:对方再连锁;优先权回到回合玩家
	CHECK(duel.activate_effect(opponent_card, 1));

	// 同一效果不能在同一连锁上再次发动
	CHECK_FALSE(duel.activate_effect(double_card, 0));
	REQUIRE(duel.chain().size() == 4);

	CHECK(duel.pass_priority());
	CHECK(duel.chain().empty());

	// C4 400 + C2 300 打 FIRST;C3 200 + C1 100 打 SECOND
	CHECK(game.life_points(domain::Player::FIRST) == 7300);
	CHECK(game.life_points(domain::Player::SECOND) == 7700);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel collects multiple effects of one card into one chain")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_chain_one_card_segoc";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"94600001.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"DESTROYED"},"timing":"WHEN","optional":false},"effect":[{"call":"draw","args":{"players":["SELF"],"count":1}}]},{"type":"activated","activation":{"mode":"trigger","source":{"event":"DESTROYED"},"timing":"WHEN","optional":false},"effect":[{"call":"recover_life","args":{"players":["SELF"],"amount":500}}]}]})"
	);
	write(
		"94600002.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"ignition"},"effect":[{"call":"destroy","args":{"cards":{"call":"find_cards","args":{"players":["SELF","OPPONENT"],"zones":["MONSTER","EXTRA_MONSTER"]}},"reason":"EFFECT"}}]}]})"
	);

	constexpr domain::CardCode victim_code{94600001};
	constexpr domain::CardCode destroyer_code{94600002};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.register_definition(victim_code, make_monster(victim_code));
	game.register_definition(destroyer_code, make_monster(destroyer_code));
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto destroyer = game.create_instance(destroyer_code, domain::Player::FIRST);
	REQUIRE(destroyer.valid());
	game.place(*game.find_instance_mutable(destroyer), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto victim = game.create_instance(victim_code, domain::Player::FIRST);
	REQUIRE(victim.valid());
	game.place(*game.find_instance_mutable(victim), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	const auto deck_card = game.create_instance(victim_code, domain::Player::FIRST);
	REQUIRE(deck_card.valid());
	game.place(*game.find_instance_mutable(deck_card), domain::DeckPlacement{.index = 0});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_effect(destroyer, 0));

	// 破坏链结算后,同一张卡的两个 DESTROYED 触发应同时入链
	CHECK(duel.pass_priority());
	REQUIRE(duel.chain().size() == 2);
	CHECK(duel.chain()[0].controller() == domain::Player::FIRST);
	CHECK(duel.chain()[1].controller() == domain::Player::FIRST);

	settle(duel);

	CHECK(game.life_points(domain::Player::FIRST) == 8500);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == 1);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::DECK) == 0);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel asks the driver for optional triggers")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_driver_missing";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"94700001.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"DESTROYED"},"timing":"WHEN","optional":true},"effect":[{"call":"draw","args":{"players":["SELF"],"count":1}}]}]})"
	);
	write(
		"94700002.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"ignition"},"effect":[{"call":"destroy","args":{"cards":{"call":"find_cards","args":{"players":["SELF","OPPONENT"],"zones":["MONSTER","EXTRA_MONSTER"]}},"reason":"EFFECT"}}]}]})"
	);

	constexpr domain::CardCode victim_code{94700001};
	constexpr domain::CardCode destroyer_code{94700002};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	game.register_definition(victim_code, make_monster(victim_code));
	game.register_definition(destroyer_code, make_monster(destroyer_code));
	REQUIRE(duel.load_scripts(directory));

	const auto destroyer = game.create_instance(destroyer_code, domain::Player::FIRST);
	REQUIRE(destroyer.valid());
	game.place(*game.find_instance_mutable(destroyer), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto victim = game.create_instance(victim_code, domain::Player::FIRST);
	REQUIRE(victim.valid());
	game.place(*game.find_instance_mutable(victim), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	const auto deck_card = game.create_instance(victim_code, domain::Player::FIRST);
	REQUIRE(deck_card.valid());
	game.place(*game.find_instance_mutable(deck_card), domain::DeckPlacement{.index = 0});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_effect(destroyer, 0));
	CHECK(duel.pass_priority());

	// 驱动被询问并拒绝选发;引擎不自动发动
	CHECK(duel.chain().empty());
	CHECK(game.zone_of(victim) == domain::Zone::GRAVEYARD);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == 0);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::DECK) == 1);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel lets the driver decline optional triggers")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_driver_decline";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"94700003.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"DESTROYED"},"timing":"WHEN","optional":false},"effect":[{"call":"recover_life","args":{"players":["SELF"],"amount":500}}]},{"type":"activated","activation":{"mode":"trigger","source":{"event":"DESTROYED"},"timing":"WHEN","optional":true},"effect":[{"call":"draw","args":{"players":["SELF"],"count":1}}]}]})"
	);
	write(
		"94700004.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"ignition"},"effect":[{"call":"destroy","args":{"cards":{"call":"find_cards","args":{"players":["SELF","OPPONENT"],"zones":["MONSTER","EXTRA_MONSTER"]}},"reason":"EFFECT"}}]}]})"
	);

	constexpr domain::CardCode victim_code{94700003};
	constexpr domain::CardCode destroyer_code{94700004};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	game.register_definition(victim_code, make_monster(victim_code));
	game.register_definition(destroyer_code, make_monster(destroyer_code));
	REQUIRE(duel.load_scripts(directory));

	const auto destroyer = game.create_instance(destroyer_code, domain::Player::FIRST);
	REQUIRE(destroyer.valid());
	game.place(*game.find_instance_mutable(destroyer), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto victim = game.create_instance(victim_code, domain::Player::FIRST);
	REQUIRE(victim.valid());
	game.place(*game.find_instance_mutable(victim), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	const auto deck_card = game.create_instance(victim_code, domain::Player::FIRST);
	REQUIRE(deck_card.valid());
	game.place(*game.find_instance_mutable(deck_card), domain::DeckPlacement{.index = 0});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_effect(destroyer, 0));
	CHECK(duel.pass_priority());

	// 必发已入链,选发被驱动拒绝
	REQUIRE(duel.chain().size() == 1);
	CHECK(duel.chain()[0].controller() == domain::Player::FIRST);

	settle(duel);

	CHECK(game.life_points(domain::Player::FIRST) == 8500);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == 0);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::DECK) == 1);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel lets the driver order optional triggers")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_driver_order";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"94700005.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"DESTROYED"},"timing":"WHEN","optional":true},"effect":[{"call":"draw","args":{"players":["SELF"],"count":1}}]},{"type":"activated","activation":{"mode":"trigger","source":{"event":"DESTROYED"},"timing":"WHEN","optional":true},"effect":[{"call":"recover_life","args":{"players":["SELF"],"amount":500}}]}]})"
	);
	write(
		"94700006.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"ignition"},"effect":[{"call":"destroy","args":{"cards":{"call":"find_cards","args":{"players":["SELF","OPPONENT"],"zones":["MONSTER","EXTRA_MONSTER"]}},"reason":"EFFECT"}}]}]})"
	);

	constexpr domain::CardCode victim_code{94700005};
	constexpr domain::CardCode destroyer_code{94700006};

	driver::LocalTestSelector selector
	{
			nullptr,
			[](const execution::TriggerDecision& decision) -> std::vector<std::size_t>
			{
				std::vector<std::size_t> indices(decision.candidates.size());
				std::ranges::iota(indices, std::size_t{0});

				if (decision.optional)
				{
					std::ranges::reverse(indices);
				}
				return indices;
			}
	};
	flow::Duel duel{selector};
	auto& game = duel.game();

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	game.register_definition(victim_code, make_monster(victim_code));
	game.register_definition(destroyer_code, make_monster(destroyer_code));
	REQUIRE(duel.load_scripts(directory));

	const auto destroyer = game.create_instance(destroyer_code, domain::Player::FIRST);
	REQUIRE(destroyer.valid());
	game.place(*game.find_instance_mutable(destroyer), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto victim = game.create_instance(victim_code, domain::Player::FIRST);
	REQUIRE(victim.valid());
	game.place(*game.find_instance_mutable(victim), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	const auto deck_card = game.create_instance(victim_code, domain::Player::FIRST);
	REQUIRE(deck_card.valid());
	game.place(*game.find_instance_mutable(deck_card), domain::DeckPlacement{.index = 0});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	const auto* script = duel.scripts().find(victim_code);
	REQUIRE(script != nullptr);

	CHECK(duel.activate_effect(destroyer, 0));
	CHECK(duel.pass_priority());

	// 驱动返回 [1,0]:链上顺序与脚本顺序相反
	REQUIRE(duel.chain().size() == 2);
	CHECK(duel.chain()[0].effect() == &script->effects[1]);
	CHECK(duel.chain()[1].effect() == &script->effects[0]);

	settle(duel);

	CHECK(game.life_points(domain::Player::FIRST) == 8500);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == 1);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::DECK) == 0);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel requires an external selector for target choice")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_driver_target";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"94700007.json",
		R"({"identity":{"kind":"SPELL","subtypes":["NORMAL"]},"effects":[{"type":"activated","activation":{"mode":"activation"},"target":[{"call":"pick_target","args":{"players":["OPPONENT"],"zones":["MONSTER"],"min":1,"max":1},"bind":"target"}],"effect":[{"call":"destroy","args":{"cards":{"var":"target"},"reason":"EFFECT"}}]}]})"
	);

	constexpr domain::CardCode spell_code{94700007};
	constexpr domain::CardCode target_code{94700008};

	auto allow = false;

	driver::LocalTestSelector selector
	{
			[&]([[maybe_unused]] const domain::Player player, const execution::Group& candidates, [[maybe_unused]] const std::size_t min, [[maybe_unused]] const std::size_t max) -> execution::Group
			{
				if (!allow)
				{
					return {};
				}

				const auto last = candidates.begin() + static_cast<std::ptrdiff_t>(candidates.size() - 1);
				return execution::Group{last, last + 1};
			},
			nullptr
	};
	flow::Duel duel{selector};
	auto& game = duel.game();

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	game.register_definition(spell_code, make_spell(spell_code, {domain::CardSubtype::NORMAL}));
	game.register_definition(target_code, make_monster(target_code));
	REQUIRE(duel.load_scripts(directory));

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto first_target = game.create_instance(target_code, domain::Player::SECOND);
	REQUIRE(first_target.valid());
	game.place(*game.find_instance_mutable(first_target), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto second_target = game.create_instance(target_code, domain::Player::SECOND);
	REQUIRE(second_target.valid());
	game.place(*game.find_instance_mutable(second_target), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	// selector 返回非法选择(空):引擎不自动补选,发动失败
	CHECK_FALSE(duel.activate_card(spell));
	CHECK(game.zone_of(spell) == domain::Zone::HAND);
	CHECK(game.zone_of(first_target) == domain::Zone::MONSTER);
	CHECK(game.zone_of(second_target) == domain::Zone::MONSTER);

	// 外部 selector 选择第二个对象
	allow = true;

	CHECK(duel.activate_card(spell));
	settle(duel);

	CHECK(game.zone_of(first_target) == domain::Zone::MONSTER);
	CHECK(game.zone_of(second_target) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);

	std::filesystem::remove_all(directory);
}

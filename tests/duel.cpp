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
	) -> dsl::CardIdentity
	{
		dsl::CardIdentity definition{};
		definition.code = code;
		definition.kind = domain::CardType::MONSTER;
		definition.subtypes = {domain::CardSubtype::NORMAL};
		definition.monster = dsl::CardIdentity::Monster
		{
				.attribute = domain::Attribute::EARTH,
				.race = race,
				.level = domain::LevelRank{level},
				.attack = domain::AttackDefense{attack},
				.defense = domain::AttackDefense{defense},
				.pendulum_scale = std::nullopt,
				.link_marker = std::nullopt,
		};

		return definition;
	}

	[[nodiscard]] auto make_spell(const domain::CardCode code, const std::initializer_list<domain::CardSubtype> subtypes) -> dsl::CardIdentity
	{
		dsl::CardIdentity definition{};
		definition.code = code;
		definition.kind = domain::CardType::SPELL;
		definition.subtypes = subtypes;
		definition.spell = dsl::CardIdentity::Spell{};

		return definition;
	}

	[[nodiscard]] auto make_trap(const domain::CardCode code, const std::initializer_list<domain::CardSubtype> subtypes) -> dsl::CardIdentity
	{
		dsl::CardIdentity definition{};
		definition.code = code;
		definition.kind = domain::CardType::TRAP;
		definition.subtypes = subtypes;
		definition.trap = dsl::CardIdentity::Trap{};

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

	[[nodiscard]] auto select_up_to_max(
		[[maybe_unused]] domain::Player player,
		const execution::Group& candidates,
		[[maybe_unused]] std::size_t min,
		const std::size_t max
	) -> execution::Group
	{
		const auto count = std::ranges::min(candidates.size(), max);
		return {candidates.begin(), candidates.begin() + static_cast<std::ptrdiff_t>(count)};
	}

	[[nodiscard]] auto choose_all_triggers(const execution::TriggerDecision& decision) -> std::vector<std::size_t>
	{
		std::vector<std::size_t> indices(decision.candidates.size());
		std::ranges::iota(indices, std::size_t{0});
		return indices;
	}

	driver::LocalTestSelector g_selector{};
}

TEST_CASE("flow::Duel performs core actions")
{
	constexpr domain::CardCode monster_code{91000001};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto monster_definition = make_monster(monster_code);
	game.register_definition(monster_definition);

	const auto deck_card = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(deck_card.valid());
	const auto hand_card = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(hand_card.valid());

	game.place(*game.find_instance_mutable(deck_card), domain::DeckPlacement{.index = 0});
	game.place(*game.find_instance_mutable(hand_card), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	// 抽卡阶段: 先手第一回合不抽卡
	CHECK(duel.turn().phase() == domain::Phase::DRAW);
	CHECK_FALSE(duel.draw_for_turn());
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::HAND) == 1);
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

TEST_CASE("flow::Duel rejects Flip Summon on the turn the monster was Set")
{
	constexpr domain::CardCode monster_code{91100002};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto monster_definition = make_monster(monster_code);
	game.register_definition(monster_definition);
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(monster.valid());
	game.place(*game.find_instance_mutable(monster), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1
	CHECK(duel.set_monster(monster, domain::FieldColumn::C1));
	CHECK_FALSE(duel.flip_summon(monster));
}

TEST_CASE("flow::Duel replays an attack when the target leaves before damage")
{
	constexpr domain::CardCode attacker_code{91100010};
	constexpr domain::CardCode defender_code{91100011};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	const auto defender_definition = make_monster(defender_code, 800, 1000);
	game.register_definition(attacker_definition);
	game.register_definition(defender_definition);

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

	game.move_to_graveyard(*game.find_instance_mutable(defender), {domain::Reason::RULE});
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.pass_priority());
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.pass_priority());

	CHECK(duel.pending_attack());
	CHECK(duel.pending_replay());
	CHECK(duel.replay_attack(std::nullopt));
	CHECK_FALSE(duel.pending_replay());
	resolve_attack(duel);

	CHECK_FALSE(duel.pending_attack());
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 7000);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);
}

TEST_CASE("flow::Duel aborts an attack when the attacker leaves attack position")
{
	constexpr domain::CardCode attacker_code{91100020};
	constexpr domain::CardCode defender_code{91100021};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	const auto defender_definition = make_monster(defender_code, 800, 1000);
	game.register_definition(attacker_definition);
	game.register_definition(defender_definition);

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

	game.place(*game.find_instance_mutable(attacker), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_DEFENSE});
	resolve_attack(duel);

	CHECK_FALSE(duel.pending_attack());
	CHECK(game.zone_of(attacker) == domain::Zone::MONSTER);
	CHECK(game.life_points(domain::Player::FIRST) == 8000);
	CHECK(game.life_points(domain::Player::SECOND) == 8000);
}

TEST_CASE("flow::Duel performs a Pendulum Summon")
{
	constexpr domain::CardCode left_scale_code{91200001};
	constexpr domain::CardCode right_scale_code{91200002};
	constexpr domain::CardCode low_code{91200003};
	constexpr domain::CardCode high_code{91200004};

	state::CardInstanceId low_id{};
	state::CardInstanceId high_id{};
	driver::LocalTestSelector selector
	{
			nullptr,
			[&]([[maybe_unused]] const domain::Player player,
			    const state::CardInstanceId card,
			    const std::span<const domain::MonsterLocation> candidates) noexcept -> std::optional<domain::MonsterLocation>
			{
				const auto preferred = card == low_id ? domain::FieldColumn::C2 : domain::FieldColumn::C4;
				for (const auto candidate: candidates)
				{
					if (candidate.column == preferred)
					{
						return candidate;
					}
				}

				return candidates.empty() ? std::nullopt : std::optional<domain::MonsterLocation>{candidates.front()};
			},
			nullptr,
			nullptr,
	};

	flow::Duel duel{selector};
	auto& game = duel.game();

	auto left_scale = make_monster(left_scale_code, 1000, 1000, 4);
	left_scale.subtypes = {domain::CardSubtype::PENDULUM};
	left_scale.monster->pendulum_scale = dsl::CardIdentity::Monster::PendulumScale{.left = domain::PendulumScale{1}, .right = domain::PendulumScale{1}};
	auto right_scale = make_monster(right_scale_code, 1000, 1000, 4);
	right_scale.subtypes = {domain::CardSubtype::PENDULUM};
	right_scale.monster->pendulum_scale = dsl::CardIdentity::Monster::PendulumScale{.left = domain::PendulumScale{8}, .right = domain::PendulumScale{8}};
	const auto low_definition = make_monster(low_code, 1000, 1000, 4);
	const auto high_definition = make_monster(high_code, 1000, 1000, 6);
	game.register_definition(left_scale);
	game.register_definition(right_scale);
	game.register_definition(low_definition);
	game.register_definition(high_definition);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto left = game.create_instance(left_scale_code, domain::Player::FIRST);
	REQUIRE(left.valid());
	const auto right = game.create_instance(right_scale_code, domain::Player::FIRST);
	REQUIRE(right.valid());
	const auto low = game.create_instance(low_code, domain::Player::FIRST);
	REQUIRE(low.valid());
	const auto high = game.create_instance(high_code, domain::Player::FIRST);
	REQUIRE(high.valid());
	low_id = low;
	high_id = high;

	game.place(*game.find_instance_mutable(left), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(right), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(low), domain::HandPlacement{.index = 2, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(high), domain::HandPlacement{.index = 3, .position = domain::Position::FACE_DOWN_DEFENSE});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.set_pendulum(left, domain::PendulumSide::LEFT));
	CHECK(duel.set_pendulum(right, domain::PendulumSide::RIGHT));
	CHECK(game.zone_of(left) == domain::Zone::SPELL_TRAP);
	CHECK(game.zone_of(right) == domain::Zone::SPELL_TRAP);

	const std::array monsters{low, high};
	CHECK(duel.pendulum_summon(monsters));
	CHECK(duel.pending_summon());
	CHECK(game.zone_of(low) == domain::Zone::HAND);
	CHECK(game.zone_of(high) == domain::Zone::HAND);
	resolve_summon(duel);

	CHECK(game.zone_of(low) == domain::Zone::MONSTER);
	CHECK(game.zone_of(high) == domain::Zone::MONSTER);
	CHECK(game.find_instance(low)->summon_record().method == domain::SummonMethod::PENDULUM);
	CHECK(game.find_instance(high)->summon_record().method == domain::SummonMethod::PENDULUM);
	CHECK(game.zone_of(left) == domain::Zone::SPELL_TRAP);
	CHECK(game.zone_of(right) == domain::Zone::SPELL_TRAP);

	const auto low_placement_value = game.find_instance(low)->placement();
	const auto high_placement_value = game.find_instance(high)->placement();
	const auto* low_placement = std::get_if<domain::MonsterPlacement>(&low_placement_value);
	const auto* high_placement = std::get_if<domain::MonsterPlacement>(&high_placement_value);
	REQUIRE(low_placement != nullptr);
	REQUIRE(high_placement != nullptr);
	CHECK(low_placement->column == domain::FieldColumn::C2);
	CHECK(high_placement->column == domain::FieldColumn::C4);
}

TEST_CASE("flow::Duel: Solemn Judgment negates a Pendulum Summon")
{
	constexpr domain::CardCode solemn_code{41420027};
	constexpr domain::CardCode left_scale_code{91200011};
	constexpr domain::CardCode right_scale_code{91200012};
	constexpr domain::CardCode low_code{91200013};
	constexpr domain::CardCode high_code{91200014};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto solemn_definition = make_trap(solemn_code, {domain::CardSubtype::COUNTER});
	game.register_definition(solemn_definition);

	auto left_scale = make_monster(left_scale_code, 1000, 1000, 4);
	left_scale.subtypes = {domain::CardSubtype::PENDULUM};
	left_scale.monster->pendulum_scale = dsl::CardIdentity::Monster::PendulumScale{.left = domain::PendulumScale{1}, .right = domain::PendulumScale{1}};
	auto right_scale = make_monster(right_scale_code, 1000, 1000, 4);
	right_scale.subtypes = {domain::CardSubtype::PENDULUM};
	right_scale.monster->pendulum_scale = dsl::CardIdentity::Monster::PendulumScale{.left = domain::PendulumScale{8}, .right = domain::PendulumScale{8}};
	game.register_definition(left_scale);
	game.register_definition(right_scale);
	const auto low_definition = make_monster(low_code, 1000, 1000, 4);
	const auto high_definition = make_monster(high_code, 1000, 1000, 6);
	game.register_definition(low_definition);
	game.register_definition(high_definition);

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto solemn = game.create_instance(solemn_code, domain::Player::FIRST);
	REQUIRE(solemn.valid());
	const auto left = game.create_instance(left_scale_code, domain::Player::SECOND);
	REQUIRE(left.valid());
	const auto right = game.create_instance(right_scale_code, domain::Player::SECOND);
	REQUIRE(right.valid());
	const auto low = game.create_instance(low_code, domain::Player::SECOND);
	REQUIRE(low.valid());
	const auto high = game.create_instance(high_code, domain::Player::SECOND);
	REQUIRE(high.valid());

	game.place(*game.find_instance_mutable(solemn), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(left), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(right), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(low), domain::HandPlacement{.index = 2, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(high), domain::HandPlacement{.index = 3, .position = domain::Position::FACE_DOWN_DEFENSE});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(duel.set_spell_trap(solemn, domain::FieldColumn::C1));
	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(duel.set_pendulum(left, domain::PendulumSide::LEFT));
	CHECK(duel.set_pendulum(right, domain::PendulumSide::RIGHT));

	const std::array monsters{low, high};
	CHECK(duel.pendulum_summon(monsters));
	CHECK(duel.pending_summon());
	CHECK(game.zone_of(low) == domain::Zone::HAND);
	CHECK(game.zone_of(high) == domain::Zone::HAND);

	// 把优先权交给对手后,对手才能发动盖放的「神的宣告」
	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.activate_card(solemn));
	CHECK(duel.chain().size() == 1);
	CHECK(game.life_points(domain::Player::FIRST) == 4000);

	settle(duel);
	CHECK(duel.pending_summon());

	resolve_summon(duel);

	CHECK_FALSE(duel.pending_summon());
	CHECK(game.zone_of(low) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(high) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(left) == domain::Zone::SPELL_TRAP);
	CHECK(game.zone_of(right) == domain::Zone::SPELL_TRAP);
	CHECK(game.zone_of(solemn) == domain::Zone::GRAVEYARD);
	CHECK(game.life_points(domain::Player::FIRST) == 4000);
}

TEST_CASE("flow::Duel performs a Flip Summon")
{
	constexpr domain::CardCode monster_code{91100001};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto monster_definition = make_monster(monster_code);
	game.register_definition(monster_definition);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(monster.valid());
	game.place(*game.find_instance_mutable(monster), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_DOWN_DEFENSE});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.flip_summon(monster));
	CHECK(duel.pending_summon());
	resolve_summon(duel);

	CHECK(game.zone_of(monster) == domain::Zone::MONSTER);
	const auto placement = game.find_instance(monster)->placement();
	const auto* monster_placement = std::get_if<domain::MonsterPlacement>(&placement);
	REQUIRE(monster_placement != nullptr);
	CHECK(monster_placement->position == domain::Position::FACE_UP_ATTACK);
	CHECK(game.find_instance(monster)->summon_record().method == domain::SummonMethod::FLIP);

	// 已经是表侧表示的怪兽不能再次翻转召唤。
	CHECK_FALSE(duel.flip_summon(monster));
}

TEST_CASE("flow::Duel resolves an attack-position battle")
{
	constexpr domain::CardCode attacker_code{91000010};
	constexpr domain::CardCode defender_code{91000011};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	const auto defender_definition = make_monster(defender_code, 800, 1500);
	game.register_definition(attacker_definition);
	game.register_definition(defender_definition);

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

	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	const auto defender_definition = make_monster(defender_code, 800, 1000);
	game.register_definition(attacker_definition);
	game.register_definition(defender_definition);

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

	const auto watcher_definition = make_monster(watcher_code, 800, 1000);
	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	game.register_definition(watcher_definition);
	game.register_definition(attacker_definition);

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

	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	const auto defender_definition = make_monster(defender_code, 800, 1000);
	game.register_definition(attacker_definition);
	game.register_definition(defender_definition);

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

	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	const auto defender_definition = make_monster(defender_code, 800, 1000);
	game.register_definition(attacker_definition);
	game.register_definition(defender_definition);

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

	const auto quick_definition = make_monster(quick_code, 1000, 1000);
	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	const auto defender_definition = make_monster(defender_code, 1000, 1000);
	game.register_definition(quick_definition);
	game.register_definition(attacker_definition);
	game.register_definition(defender_definition);

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

TEST_CASE("flow::Duel filters auto-collected damage-step triggers by declared properties")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_damage_step_auto_triggers";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"97000061.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"BATTLE_DESTROYED"},"timing":"WHEN","optional":false},"properties":["ATK_DEF_CHANGE"],"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":100}}]}]})"
	);
	write(
		"97000062.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"BATTLE_DESTROYED"},"timing":"WHEN","optional":false},"properties":["DAMAGE_STEP"],"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":100}}]}]})"
	);

	constexpr domain::CardCode atk_property_code{97000061};
	constexpr domain::CardCode damage_step_property_code{97000062};
	constexpr domain::CardCode attacker_code{97000063};
	constexpr domain::CardCode defender_code{97000064};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto atk_property_definition = make_monster(atk_property_code, 1000, 1000);
	const auto damage_step_property_definition = make_monster(damage_step_property_code, 1000, 1000);
	const auto attacker_definition = make_monster(attacker_code, 2000, 2000);
	const auto defender_definition = make_monster(defender_code, 1000, 1000);
	game.register_definition(atk_property_definition);
	game.register_definition(damage_step_property_definition);
	game.register_definition(attacker_definition);
	game.register_definition(defender_definition);

	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto atk_property = game.create_instance(atk_property_code, domain::Player::FIRST);
	REQUIRE(atk_property.valid());
	game.place(*game.find_instance_mutable(atk_property), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	const auto damage_step_property = game.create_instance(damage_step_property_code, domain::Player::FIRST);
	REQUIRE(damage_step_property.valid());
	game.place(*game.find_instance_mutable(damage_step_property), domain::MonsterPlacement{.column = domain::FieldColumn::C3, .position = domain::Position::FACE_UP_ATTACK});

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

	// 战斗伤害 1000 -> FIRST 7000
	CHECK(game.life_points(domain::Player::FIRST) == 7000);
	// ATK_DEF_CHANGE 属性在伤害步骤结束时不可发动, 只有 DAMAGE_STEP 属性的诱发生效
	CHECK(game.life_points(domain::Player::SECOND) == 7900);
	CHECK(game.zone_of(atk_property) == domain::Zone::MONSTER);
	CHECK(game.zone_of(damage_step_property) == domain::Zone::MONSTER);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel opens a CARD_DRAWN timing for draw triggers")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_draw_trigger";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	std::ofstream stream{directory / "97000071.json", std::ios::binary};
	stream <<
			R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"CARD_DRAWN"},"timing":"WHEN","optional":false},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":100}}]}]})";
	stream.close();

	constexpr domain::CardCode trigger_code{97000071};
	constexpr domain::CardCode filler_code{97000072};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto trigger_definition = make_monster(trigger_code, 1000, 1000);
	const auto filler_definition = make_monster(filler_code, 1000, 1000);
	game.register_definition(trigger_definition);
	game.register_definition(filler_definition);
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto trigger = game.create_instance(trigger_code, domain::Player::FIRST);
	REQUIRE(trigger.valid());
	game.place(*game.find_instance_mutable(trigger), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto filler = game.create_instance(filler_code, domain::Player::SECOND);
	REQUIRE(filler.valid());
	game.place(*game.find_instance_mutable(filler), domain::DeckPlacement{.index = 0});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());
	REQUIRE(duel.turn().phase() == domain::Phase::DRAW);

	CHECK(duel.draw_for_turn());
	CHECK(duel.chain().size() == 1);
	settle(duel);

	CHECK(game.life_points(domain::Player::SECOND) == 7900);

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel opens a SPELL_TRAP_SET timing for set triggers")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_set_trigger";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	std::ofstream stream{directory / "97000073.json", std::ios::binary};
	stream <<
			R"({"identity":{"kind":"MONSTER","subtypes":["EFFECT"],"monster":{"attribute":"DARK","race":"FIEND","level":4,"attack":1000,"defense":1000}},"effects":[{"type":"activated","activation":{"mode":"trigger","source":{"event":"SPELL_TRAP_SET"},"timing":"WHEN","optional":false},"effect":[{"call":"effect_damage","args":{"players":["OPPONENT"],"amount":100}}]}]})";
	stream.close();

	constexpr domain::CardCode trigger_code{97000073};
	constexpr domain::CardCode set_card_code{97000074};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto trigger_definition = make_monster(trigger_code, 1000, 1000);
	const auto set_card_definition = make_spell(set_card_code, {domain::CardSubtype::NORMAL});
	game.register_definition(trigger_definition);
	game.register_definition(set_card_definition);
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto trigger = game.create_instance(trigger_code, domain::Player::FIRST);
	REQUIRE(trigger.valid());
	game.place(*game.find_instance_mutable(trigger), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto set_card = game.create_instance(set_card_code, domain::Player::FIRST);
	REQUIRE(set_card.valid());
	game.place(*game.find_instance_mutable(set_card), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.set_spell_trap(set_card, domain::FieldColumn::C2));
	CHECK(duel.chain().size() == 1);
	settle(duel);

	CHECK(game.life_points(domain::Player::SECOND) == 7900);
	CHECK(game.zone_of(set_card) == domain::Zone::SPELL_TRAP);

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

	const auto counter_definition = make_trap(counter_code, {domain::CardSubtype::COUNTER});
	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	const auto defender_definition = make_monster(defender_code, 1000, 1000);
	game.register_definition(counter_definition);
	game.register_definition(attacker_definition);
	game.register_definition(defender_definition);

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

	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	const auto watcher_definition = make_monster(watcher_code, 1000, 1000);
	const auto defender_definition = make_monster(defender_code, 800, 800);
	game.register_definition(attacker_definition);
	game.register_definition(watcher_definition);
	game.register_definition(defender_definition);

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

	const auto attacker_definition = make_monster(attacker_code, 800, 800);
	const auto watcher_definition = make_monster(watcher_code, 1000, 1000);
	const auto defender_definition = make_monster(defender_code, 1000, 1000);
	game.register_definition(attacker_definition);
	game.register_definition(watcher_definition);
	game.register_definition(defender_definition);

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

	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	const auto defender_definition = make_monster(defender_code, 800, 1500);
	game.register_definition(attacker_definition);
	game.register_definition(defender_definition);

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

	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	game.register_definition(attacker_definition);

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

	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	const auto defender_definition = make_monster(defender_code, 800, 800);
	game.register_definition(attacker_definition);
	game.register_definition(defender_definition);

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

	const auto high_definition = make_monster(high_code, 2000, 2000, 5);
	game.register_definition(high_definition);
	const auto low_definition = make_monster(low_code, 1000, 1000);
	game.register_definition(low_definition);
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

	const auto high_definition = make_monster(high_code, 2500, 2500, 7);
	game.register_definition(high_definition);
	const auto low_definition = make_monster(low_code, 1000, 1000);
	game.register_definition(low_definition);
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

	// 先手第一回合不抽卡, 因而不判负
	CHECK_FALSE(duel.draw_for_turn());
	CHECK_FALSE(duel.winner().has_value());

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	// 后手第一抽时卡组为空 -> 后手判负
	CHECK_FALSE(duel.draw_for_turn());
	CHECK(duel.winner() == domain::Player::FIRST);
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

	const auto jar_definition = make_monster(jar_code);
	game.register_definition(jar_definition);
	const auto monster_definition = make_monster(monster_code);
	game.register_definition(monster_definition);
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

	dsl::CardIdentity mountain{};
	mountain.code = mountain_code;
	mountain.kind = domain::CardType::SPELL;
	mountain.subtypes = {domain::CardSubtype::FIELD};
	mountain.spell = dsl::CardIdentity::Spell{};
	game.register_definition(mountain);

	const auto dragon_definition = make_monster(dragon_code, 1000, 1000, 4, domain::Race::DRAGON);
	game.register_definition(dragon_definition);
	const auto warrior_definition = make_monster(warrior_code, 1000, 1000, 4, domain::Race::WARRIOR);
	game.register_definition(warrior_definition);
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

		const auto mountain_definition = make_spell(mountain_code, {domain::CardSubtype::FIELD});
		game.register_definition(mountain_definition);
		const auto monster_definition = make_monster(monster_code, 1000, 1000, 4, domain::Race::DRAGON);
		game.register_definition(monster_definition);
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

		const auto field_definition = make_spell(field_code, {domain::CardSubtype::FIELD});
		game.register_definition(field_definition);
		dsl::CardIdentity monster{};
		monster.code = monster_code;
		monster.kind = domain::CardType::MONSTER;
		monster.subtypes = {domain::CardSubtype::EFFECT};
		monster.monster = dsl::CardIdentity::Monster
		{
				.attribute = domain::Attribute::WIND,
				.race = domain::Race::DRAGON,
				.level = domain::LevelRank{4},
				.attack = domain::AttackDefense{1000},
				.defense = domain::AttackDefense{1000},
				.pendulum_scale = std::nullopt,
				.link_marker = std::nullopt,
		};
		game.register_definition(monster);

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

	game.register_definition(tuner);
	game.register_definition(non_tuner);
	game.register_definition(synchro);
	game.register_definition(xyz);
	game.register_definition(material_a);
	game.register_definition(material_b);
	game.register_definition(synchro_restricted);
	game.register_definition(xyz_restricted);

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
		"99700013.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["FUSION"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000}},"summons":[{"method":"fusion","materials":[{"filter":{"call":"is_card_type","args":{"card":{"call":"$"},"card_type":"MONSTER"}}},{"card":99700011}]}]})"
	);
	write(
		"99700020.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["LINK"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000},"link_marker":["TOP","BOTTOM"]},"summons":[{"method":"link","material":{"call":"is_card_type","args":{"card":{"call":"$"},"card_type":"MONSTER"}},"count":2}]})"
	);
	write(
		"99700023.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["LINK"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000},"link_marker":["TOP","BOTTOM","LEFT"]},"summons":[{"method":"link","material":{"call":"is_card_type","args":{"card":{"call":"$"},"card_type":"MONSTER"}},"count":2}]})"
	);
	write(
		"99700030.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["RITUAL"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000}},"summons":[{"method":"ritual","material":{"call":"is_card_type","args":{"card":{"call":"$"},"card_type":"MONSTER"}},"level":4}]})"
	);
	write(
		"99700040.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["SPECIAL_SUMMON"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000}},"summons":[{"method":"special","condition":true,"cost":[],"source_zones":["HAND"]}]})"
	);
	write(
		"99700041.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["SPECIAL_SUMMON"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000}},"summons":[{"method":"special","condition":false,"cost":[],"source_zones":["HAND"]}]})"
	);

	constexpr domain::CardCode fusion_code{99700010};
	constexpr domain::CardCode fusion_material_a{99700011};
	constexpr domain::CardCode fusion_material_b{99700012};
	constexpr domain::CardCode fusion_alternate_code{99700013};
	constexpr domain::CardCode link_code{99700020};
	constexpr domain::CardCode link_material_a{99700021};
	constexpr domain::CardCode link_material_b{99700022};
	constexpr domain::CardCode link_bad_rating_code{99700023};
	constexpr domain::CardCode ritual_code{99700030};
	constexpr domain::CardCode ritual_material_a{99700031};
	constexpr domain::CardCode ritual_material_b{99700032};
	constexpr domain::CardCode self_code{99700040};
	constexpr domain::CardCode self_false_code{99700041};

	auto fusion_definition = make_monster(fusion_code, 2000, 2000, 4);
	fusion_definition.subtypes = {domain::CardSubtype::FUSION};
	auto fusion_alternate_definition = make_monster(fusion_alternate_code, 2000, 2000, 4);
	fusion_alternate_definition.subtypes = {domain::CardSubtype::FUSION};
	auto link_definition = make_monster(link_code, 2000, 2000, 4);
	link_definition.subtypes = {domain::CardSubtype::LINK};
	link_definition.monster->link_marker = utility::EnumMask
	{
			domain::LinkMarker::TOP,
			domain::LinkMarker::BOTTOM,
	};
	auto link_bad_rating_definition = make_monster(link_bad_rating_code, 2000, 2000, 4);
	link_bad_rating_definition.subtypes = {domain::CardSubtype::LINK};
	link_bad_rating_definition.monster->link_marker = utility::EnumMask
	{
			domain::LinkMarker::TOP,
			domain::LinkMarker::BOTTOM,
			domain::LinkMarker::LEFT,
	};
	auto ritual_definition = make_monster(ritual_code, 2000, 2000, 4);
	ritual_definition.subtypes = {domain::CardSubtype::RITUAL};
	auto self_definition = make_monster(self_code, 2000, 2000, 4);
	self_definition.subtypes = {domain::CardSubtype::SPECIAL_SUMMON};
	auto self_false_definition = make_monster(self_false_code, 2000, 2000, 4);
	self_false_definition.subtypes = {domain::CardSubtype::SPECIAL_SUMMON};

	const auto fusion_material_a_definition = make_monster(fusion_material_a, 1000, 1000, 2);
	const auto fusion_material_b_definition = make_monster(fusion_material_b, 1000, 1000, 2);
	const auto link_material_a_definition = make_monster(link_material_a, 1000, 1000, 2);
	const auto link_material_b_definition = make_monster(link_material_b, 1000, 1000, 2);
	const auto ritual_material_a_definition = make_monster(ritual_material_a, 1000, 1000, 2);
	const auto ritual_material_b_definition = make_monster(ritual_material_b, 1000, 1000, 2);

	const auto prepare = [&](flow::Duel& duel) -> void
	{
		auto& game = duel.game();

		game.register_definition(fusion_definition);
		game.register_definition(fusion_alternate_definition);
		game.register_definition(fusion_material_a_definition);
		game.register_definition(fusion_material_b_definition);
		game.register_definition(link_definition);
		game.register_definition(link_bad_rating_definition);
		game.register_definition(link_material_a_definition);
		game.register_definition(link_material_b_definition);
		game.register_definition(ritual_definition);
		game.register_definition(ritual_material_a_definition);
		game.register_definition(ritual_material_b_definition);
		game.register_definition(self_definition);
		game.register_definition(self_false_definition);

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

	SUBCASE("fusion alternates")
	{
		flow::Duel duel{g_selector};
		prepare(duel);
		auto& game = duel.game();

		const auto fusion = game.create_instance(fusion_alternate_code, domain::Player::FIRST);
		REQUIRE(fusion.valid());
		game.place(*game.find_instance_mutable(fusion), domain::ExtraDeckPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

		const auto material_a = game.create_instance(fusion_material_a, domain::Player::FIRST);
		REQUIRE(material_a.valid());
		game.place(*game.find_instance_mutable(material_a), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});
		const auto material_b = game.create_instance(fusion_material_b, domain::Player::FIRST);
		REQUIRE(material_b.valid());
		game.place(*game.find_instance_mutable(material_b), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

		advance_phase(duel);
		advance_phase(duel);

		// 具体的 99700011 必须留给第二个素材槽,通用表达式先占用 99700012
		const std::array materials{material_a, material_b};
		CHECK(duel.fusion_summon(fusion, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, materials));
		resolve_summon(duel);

		CHECK(game.zone_of(fusion) == domain::Zone::EXTRA_MONSTER);
		CHECK(game.zone_of(material_a) == domain::Zone::GRAVEYARD);
		CHECK(game.zone_of(material_b) == domain::Zone::GRAVEYARD);
	}

	SUBCASE("link")
	{
		flow::Duel duel{g_selector};
		prepare(duel);
		auto& game = duel.game();

		const auto link = game.create_instance(link_code, domain::Player::FIRST);
		REQUIRE(link.valid());
		game.place(*game.find_instance_mutable(link), domain::ExtraDeckPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
		const auto link_bad_rating = game.create_instance(link_bad_rating_code, domain::Player::FIRST);
		REQUIRE(link_bad_rating.valid());
		game.place(*game.find_instance_mutable(link_bad_rating), domain::ExtraDeckPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});

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
		CHECK_FALSE(duel.link_summon(link_bad_rating, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, materials));
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

	SUBCASE("self special failures")
	{
		flow::Duel duel{g_selector};
		prepare(duel);
		auto& game = duel.game();

		const auto on_field = game.create_instance(self_code, domain::Player::FIRST);
		REQUIRE(on_field.valid());
		game.place(*game.find_instance_mutable(on_field), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

		const auto condition_false = game.create_instance(self_false_code, domain::Player::FIRST);
		REQUIRE(condition_false.valid());
		game.place(*game.find_instance_mutable(condition_false), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

		advance_phase(duel);
		advance_phase(duel);

		// 来源区域不是 procedure 允许的HAND
		CHECK_FALSE(duel.self_special_summon(on_field, domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK}));
		// 条件本身不成立。
		CHECK_FALSE(duel.self_special_summon(condition_false, domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK}));
		CHECK_FALSE(duel.pending_summon());
		CHECK(game.zone_of(condition_false) == domain::Zone::HAND);
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

	const auto solemn_definition = make_trap(solemn_code, {domain::CardSubtype::COUNTER});
	game.register_definition(solemn_definition);
	const auto monster_definition = make_monster(monster_code);
	game.register_definition(monster_definition);
	const auto second_definition = make_monster(second_code);
	game.register_definition(second_definition);
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

	const auto solemn_definition = make_trap(solemn_code, {domain::CardSubtype::COUNTER});
	game.register_definition(solemn_definition);
	auto xyz_definition = make_monster(xyz_code, 2000, 2000, 4);
	xyz_definition.subtypes = {domain::CardSubtype::XYZ};
	game.register_definition(xyz_definition);
	const auto material_a_definition = make_monster(material_a_code, 1000, 1000, 4);
	game.register_definition(material_a_definition);
	const auto material_b_definition = make_monster(material_b_code, 1000, 1000, 4);
	game.register_definition(material_b_definition);
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

TEST_CASE("flow::Duel: Solemn Judgment negates a Fusion Summon")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_solemn_fusion";
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

	constexpr domain::CardCode solemn_code{41420027};
	constexpr domain::CardCode fusion_code{99700010};
	constexpr domain::CardCode material_a_code{99700011};
	constexpr domain::CardCode material_b_code{99700012};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto solemn_definition = make_trap(solemn_code, {domain::CardSubtype::COUNTER});
	game.register_definition(solemn_definition);
	auto fusion_definition = make_monster(fusion_code, 2000, 2000, 4);
	fusion_definition.subtypes = {domain::CardSubtype::FUSION};
	game.register_definition(fusion_definition);
	const auto material_a_definition = make_monster(material_a_code, 1000, 1000, 2);
	game.register_definition(material_a_definition);
	const auto material_b_definition = make_monster(material_b_code, 1000, 1000, 2);
	game.register_definition(material_b_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));
	REQUIRE(duel.load_scripts(directory));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto solemn = game.create_instance(solemn_code, domain::Player::FIRST);
	REQUIRE(solemn.valid());
	game.place(*game.find_instance_mutable(solemn), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto fusion = game.create_instance(fusion_code, domain::Player::SECOND);
	REQUIRE(fusion.valid());
	game.place(*game.find_instance_mutable(fusion), domain::ExtraDeckPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto material_a = game.create_instance(material_a_code, domain::Player::SECOND);
	REQUIRE(material_a.valid());
	game.place(*game.find_instance_mutable(material_a), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});
	const auto material_b = game.create_instance(material_b_code, domain::Player::SECOND);
	REQUIRE(material_b.valid());
	game.place(*game.find_instance_mutable(material_b), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(duel.set_spell_trap(solemn, domain::FieldColumn::C1));
	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1

	const std::array materials{material_a, material_b};
	CHECK(duel.fusion_summon(fusion, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, materials));
	CHECK(duel.pending_summon());
	CHECK_FALSE(duel.pass_priority());

	CHECK(duel.activate_card(solemn));
	CHECK(game.life_points(domain::Player::FIRST) == 4000);
	resolve_summon(duel);

	CHECK_FALSE(duel.pending_summon());
	CHECK(game.zone_of(fusion) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(material_a) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(material_b) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(solemn) == domain::Zone::GRAVEYARD);
	CHECK(game.find_instance(fusion)->overlays().empty());

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel: Solemn Judgment negates Link, Ritual, and SelfSpecial Summons")
{
	const auto directory = std::filesystem::temp_directory_path() / "cg_duel_solemn_extra_summons";
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);

	const auto write = [&](const std::string_view name, const std::string_view text) -> void
	{
		std::ofstream stream{directory / name, std::ios::binary};
		stream << text;
	};

	write(
		"97000101.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["LINK"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000},"link_marker":["TOP","BOTTOM"]},"summons":[{"method":"link","material":{"call":"is_card_type","args":{"card":{"call":"$"},"card_type":"MONSTER"}},"count":2}]})"
	);
	write(
		"97000110.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["RITUAL"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000}},"summons":[{"method":"ritual","material":{"call":"is_card_type","args":{"card":{"call":"$"},"card_type":"MONSTER"}},"level":4}]})"
	);
	write(
		"97000120.json",
		R"({"identity":{"kind":"MONSTER","subtypes":["SPECIAL_SUMMON"],"monster":{"attribute":"EARTH","race":"DRAGON","level":4,"attack":2000,"defense":2000}},"summons":[{"method":"special","condition":true,"cost":[],"source_zones":["HAND"]}]})"
	);

	constexpr domain::CardCode solemn_code{41420027};
	constexpr domain::CardCode link_code{97000101};
	constexpr domain::CardCode link_material_a_code{97000102};
	constexpr domain::CardCode link_material_b_code{97000103};
	constexpr domain::CardCode ritual_code{97000110};
	constexpr domain::CardCode ritual_material_a_code{97000111};
	constexpr domain::CardCode ritual_material_b_code{97000112};
	constexpr domain::CardCode self_code{97000120};

	const auto solemn_definition = make_trap(solemn_code, {domain::CardSubtype::COUNTER});

	auto link_definition = make_monster(link_code, 2000, 2000, 4);
	link_definition.subtypes = {domain::CardSubtype::LINK};
	link_definition.monster->link_marker = utility::EnumMask
	{
			domain::LinkMarker::TOP,
			domain::LinkMarker::BOTTOM,
	};
	const auto link_material_a_definition = make_monster(link_material_a_code, 1000, 1000, 2);
	const auto link_material_b_definition = make_monster(link_material_b_code, 1000, 1000, 2);

	auto ritual_definition = make_monster(ritual_code, 2000, 2000, 4);
	ritual_definition.subtypes = {domain::CardSubtype::RITUAL};
	const auto ritual_material_a_definition = make_monster(ritual_material_a_code, 1000, 1000, 2);
	const auto ritual_material_b_definition = make_monster(ritual_material_b_code, 1000, 1000, 2);

	auto self_definition = make_monster(self_code, 2000, 2000, 4);
	self_definition.subtypes = {domain::CardSubtype::SPECIAL_SUMMON};

	const auto prepare = [&](flow::Duel& duel) -> void
	{
		auto& game = duel.game();

		game.register_definition(solemn_definition);
		game.register_definition(link_definition);
		game.register_definition(link_material_a_definition);
		game.register_definition(link_material_b_definition);
		game.register_definition(ritual_definition);
		game.register_definition(ritual_material_a_definition);
		game.register_definition(ritual_material_b_definition);
		game.register_definition(self_definition);

		REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));
		REQUIRE(duel.load_scripts(directory));

		game.set_life_points(domain::Player::FIRST, 8000);
		game.set_life_points(domain::Player::SECOND, 8000);
	};

	const auto set_solemn_and_pass = [](flow::Duel& duel, const state::CardInstanceId solemn) -> void
	{
		CHECK(advance_phase(duel)); // STANDBY
		CHECK(advance_phase(duel)); // MAIN_1
		CHECK(duel.set_spell_trap(solemn, domain::FieldColumn::C1));
		CHECK(advance_phase(duel)); // END
		CHECK(duel.end_turn());

		CHECK(advance_phase(duel)); // STANDBY
		CHECK(advance_phase(duel)); // MAIN_1
	};

	SUBCASE("link")
	{
		flow::Duel duel{g_selector};
		prepare(duel);
		auto& game = duel.game();

		const auto link = game.create_instance(link_code, domain::Player::SECOND);
		REQUIRE(link.valid());
		game.place(*game.find_instance_mutable(link), domain::ExtraDeckPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

		const auto material_a = game.create_instance(link_material_a_code, domain::Player::SECOND);
		REQUIRE(material_a.valid());
		game.place(*game.find_instance_mutable(material_a), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});
		const auto material_b = game.create_instance(link_material_b_code, domain::Player::SECOND);
		REQUIRE(material_b.valid());
		game.place(*game.find_instance_mutable(material_b), domain::MonsterPlacement{.column = domain::FieldColumn::C3, .position = domain::Position::FACE_UP_ATTACK});

		const auto solemn = game.create_instance(solemn_code, domain::Player::FIRST);
		REQUIRE(solemn.valid());
		game.place(*game.find_instance_mutable(solemn), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
		set_solemn_and_pass(duel, solemn);

		const std::array materials{material_a, material_b};
		CHECK(duel.link_summon(link, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, materials));
		CHECK(duel.pending_summon());
		CHECK_FALSE(duel.pass_priority());
		CHECK(duel.activate_card(solemn));
		CHECK(game.life_points(domain::Player::FIRST) == 4000);
		resolve_summon(duel);

		CHECK_FALSE(duel.pending_summon());
		CHECK(game.zone_of(link) == domain::Zone::GRAVEYARD);
		CHECK(game.zone_of(material_a) == domain::Zone::GRAVEYARD);
		CHECK(game.zone_of(material_b) == domain::Zone::GRAVEYARD);
		CHECK(game.zone_of(solemn) == domain::Zone::GRAVEYARD);
	}

	SUBCASE("ritual")
	{
		flow::Duel duel{g_selector};
		prepare(duel);
		auto& game = duel.game();

		const auto ritual = game.create_instance(ritual_code, domain::Player::SECOND);
		REQUIRE(ritual.valid());
		game.place(*game.find_instance_mutable(ritual), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

		const auto material_a = game.create_instance(ritual_material_a_code, domain::Player::SECOND);
		REQUIRE(material_a.valid());
		game.place(*game.find_instance_mutable(material_a), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});
		const auto material_b = game.create_instance(ritual_material_b_code, domain::Player::SECOND);
		REQUIRE(material_b.valid());
		game.place(*game.find_instance_mutable(material_b), domain::MonsterPlacement{.column = domain::FieldColumn::C3, .position = domain::Position::FACE_UP_ATTACK});

		const auto solemn = game.create_instance(solemn_code, domain::Player::FIRST);
		REQUIRE(solemn.valid());
		game.place(*game.find_instance_mutable(solemn), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
		set_solemn_and_pass(duel, solemn);

		const std::array materials{material_a, material_b};
		CHECK(duel.ritual_summon(ritual, domain::MonsterPlacement{.column = domain::FieldColumn::C4, .position = domain::Position::FACE_UP_ATTACK}, materials));
		CHECK(duel.pending_summon());
		CHECK_FALSE(duel.pass_priority());
		CHECK(duel.activate_card(solemn));
		CHECK(game.life_points(domain::Player::FIRST) == 4000);
		resolve_summon(duel);

		CHECK_FALSE(duel.pending_summon());
		CHECK(game.zone_of(ritual) == domain::Zone::GRAVEYARD);
		CHECK(game.zone_of(material_a) == domain::Zone::GRAVEYARD);
		CHECK(game.zone_of(material_b) == domain::Zone::GRAVEYARD);
		CHECK(game.zone_of(solemn) == domain::Zone::GRAVEYARD);
	}

	SUBCASE("self special")
	{
		flow::Duel duel{g_selector};
		prepare(duel);
		auto& game = duel.game();

		const auto self = game.create_instance(self_code, domain::Player::SECOND);
		REQUIRE(self.valid());
		game.place(*game.find_instance_mutable(self), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

		const auto solemn = game.create_instance(solemn_code, domain::Player::FIRST);
		REQUIRE(solemn.valid());
		game.place(*game.find_instance_mutable(solemn), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
		set_solemn_and_pass(duel, solemn);

		CHECK(duel.self_special_summon(self, domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK}));
		CHECK(duel.pending_summon());
		CHECK_FALSE(duel.pass_priority());
		CHECK(duel.activate_card(solemn));
		CHECK(game.life_points(domain::Player::FIRST) == 4000);
		resolve_summon(duel);

		CHECK_FALSE(duel.pending_summon());
		CHECK(game.zone_of(self) == domain::Zone::GRAVEYARD);
		CHECK(game.zone_of(solemn) == domain::Zone::GRAVEYARD);
	}

	std::filesystem::remove_all(directory);
}

TEST_CASE("flow::Duel: Solemn Judgment negates and destroys a Spell activation")
{
	constexpr domain::CardCode solemn_code{41420027};
	constexpr domain::CardCode raigeki_code{12580477};
	constexpr domain::CardCode monster_code{97000092};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto solemn_definition = make_trap(solemn_code, {domain::CardSubtype::COUNTER});
	game.register_definition(solemn_definition);
	const auto raigeki_definition = make_spell(raigeki_code, {domain::CardSubtype::NORMAL});
	game.register_definition(raigeki_definition);
	const auto monster_definition = make_monster(monster_code);
	game.register_definition(monster_definition);
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

	const auto solemn_definition = make_trap(solemn_code, {domain::CardSubtype::COUNTER});
	game.register_definition(solemn_definition);
	const auto monster_definition = make_monster(monster_code);
	game.register_definition(monster_definition);
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

	const auto field_definition = make_spell(field_code, {domain::CardSubtype::FIELD});
	game.register_definition(field_definition);
	const auto defender_definition = make_monster(defender_code, 800, 800);
	game.register_definition(defender_definition);
	const auto attacker_definition = make_monster(attacker_code, 1000, 1000);
	game.register_definition(attacker_definition);
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
			nullptr,
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

	const auto rat_definition = make_monster(rat_code, 1400, 1450, 4, domain::Race::BEAST);
	game.register_definition(rat_definition);
	const auto attacker_definition = make_monster(attacker_code, 1600, 1000);
	game.register_definition(attacker_definition);
	const auto recruit_definition = make_monster(recruit_code, 1000, 1000);
	game.register_definition(recruit_definition);
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

	const auto igniter_definition = make_monster(igniter_code);
	game.register_definition(igniter_definition);
	const auto victim_definition = make_monster(victim_code);
	game.register_definition(victim_definition);
	const auto draw_definition = make_monster(draw_code);
	game.register_definition(draw_definition);
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

	const auto card_definition = make_monster(card_code);
	game.register_definition(card_definition);
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

	const auto card_definition = make_monster(card_code);
	game.register_definition(card_definition);
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
	const auto card_definition = make_monster(card_code);
	duel.game().register_definition(card_definition);
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

	const auto destroyer_definition = make_monster(destroyer_code);
	game.register_definition(destroyer_definition);
	const auto victim_definition = make_monster(victim_code);
	game.register_definition(victim_definition);
	const auto draw_definition = make_monster(draw_code);
	game.register_definition(draw_definition);
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

	const auto torrential_definition = make_trap(torrential_code, {domain::CardSubtype::NORMAL});
	game.register_definition(torrential_definition);
	const auto monster_definition = make_monster(monster_code);
	game.register_definition(monster_definition);
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

	const auto card_94000001_definition = make_monster(domain::CardCode{94000001});
	game.register_definition(card_94000001_definition);
	const auto card_94000002_definition = make_monster(domain::CardCode{94000002});
	game.register_definition(card_94000002_definition);
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

	const auto card_95000001_definition = make_monster(domain::CardCode{95000001});
	game.register_definition(card_95000001_definition);
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

	const auto raigeki_definition = make_spell(raigeki_code, {domain::CardSubtype::NORMAL});
	game.register_definition(raigeki_definition);
	const auto mountain_definition = make_spell(mountain_code, {domain::CardSubtype::FIELD});
	game.register_definition(mountain_definition);
	const auto monster_definition = make_monster(monster_code);
	game.register_definition(monster_definition);
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

TEST_CASE("flow::Duel resolves a Ritual Spell through effect summon")
{
	constexpr domain::CardCode spell_code{97000125};
	constexpr domain::CardCode ritual_code{97000130};
	constexpr domain::CardCode material_a_code{97000131};
	constexpr domain::CardCode material_b_code{97000132};

	state::CardInstanceId ritual_id{};
	state::CardInstanceId material_a_id{};
	state::CardInstanceId material_b_id{};

	driver::LocalTestSelector selector
	{
			[&]([[maybe_unused]] const domain::Player player, const execution::Group& candidates, const std::size_t min, [[maybe_unused]] const std::size_t max) -> execution::Group
			{
				if (ritual_id.valid() && std::ranges::contains(candidates, ritual_id))
				{
					execution::Group result{};
					result.push_back(ritual_id);
					return result;
				}

				if (material_a_id.valid() &&
				    material_b_id.valid() &&
				    std::ranges::contains(candidates, material_a_id) &&
				    std::ranges::contains(candidates, material_b_id))
				{
					execution::Group result{};
					result.push_back(material_a_id);
					result.push_back(material_b_id);
					return result;
				}

				const auto count = std::ranges::min(min, candidates.size());
				return {candidates.begin(), candidates.begin() + static_cast<std::ptrdiff_t>(count)};
			},
			nullptr,
			nullptr,
			nullptr
	};
	flow::Duel duel{selector};
	auto& game = duel.game();

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::RITUAL});
	game.register_definition(spell_definition);
	auto ritual = make_monster(ritual_code, 2000, 2000, 4);
	ritual.subtypes = {domain::CardSubtype::RITUAL};
	game.register_definition(ritual);
	const auto material_a_definition = make_monster(material_a_code, 1000, 1000, 2);
	game.register_definition(material_a_definition);
	const auto material_b_definition = make_monster(material_b_code, 1000, 1000, 2);
	game.register_definition(material_b_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	const auto ritual_card = game.create_instance(ritual_code, domain::Player::FIRST);
	REQUIRE(ritual_card.valid());
	const auto material_a = game.create_instance(material_a_code, domain::Player::FIRST);
	REQUIRE(material_a.valid());
	const auto material_b = game.create_instance(material_b_code, domain::Player::FIRST);
	REQUIRE(material_b.valid());

	ritual_id = ritual_card;
	material_a_id = material_a;
	material_b_id = material_b;

	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(ritual_card), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(material_a), domain::HandPlacement{.index = 2, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(material_b), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(spell));
	CHECK(game.zone_of(spell) == domain::Zone::SPELL_TRAP);
	REQUIRE(duel.chain().size() == 1);

	settle(duel);

	CHECK_FALSE(duel.pending_summon());
	CHECK(game.zone_of(ritual_card) == domain::Zone::MONSTER);
	CHECK(game.zone_of(material_a) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(material_b) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);
	CHECK(game.find_instance(ritual_card)->summon_record().method == domain::SummonMethod::RITUAL);
	CHECK(game.find_instance(ritual_card)->summon_record().materials.size() == 2);
}

TEST_CASE("flow::Duel filters the Ritual Spell monster choice")
{
	constexpr domain::CardCode spell_code{97000128};
	constexpr domain::CardCode earth_ritual_code{97000130};
	constexpr domain::CardCode dark_ritual_code{97000135};
	constexpr domain::CardCode material_code{97000133};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::RITUAL});
	game.register_definition(spell_definition);
	auto earth_ritual = make_monster(earth_ritual_code, 2000, 2000, 4);
	earth_ritual.subtypes = {domain::CardSubtype::RITUAL};
	game.register_definition(earth_ritual);
	auto dark_ritual = make_monster(dark_ritual_code, 2000, 2000, 4);
	dark_ritual.subtypes = {domain::CardSubtype::RITUAL};
	dark_ritual.monster->attribute = domain::Attribute::DARK;
	game.register_definition(dark_ritual);
	const auto material_definition = make_monster(material_code, 1000, 1000, 4);
	game.register_definition(material_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	const auto material = game.create_instance(material_code, domain::Player::FIRST);
	REQUIRE(material.valid());
	const auto earth_ritual_card = game.create_instance(earth_ritual_code, domain::Player::FIRST);
	REQUIRE(earth_ritual_card.valid());
	const auto dark_ritual_card = game.create_instance(dark_ritual_code, domain::Player::FIRST);
	REQUIRE(dark_ritual_card.valid());

	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(material), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(earth_ritual_card), domain::HandPlacement{.index = 2, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(dark_ritual_card), domain::HandPlacement{.index = 3, .position = domain::Position::FACE_DOWN_DEFENSE});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(spell));
	settle(duel);

	CHECK(game.zone_of(earth_ritual_card) == domain::Zone::MONSTER);
	CHECK(game.zone_of(dark_ritual_card) == domain::Zone::HAND);
	CHECK(game.zone_of(material) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel fizzles a Ritual Spell with an invalid level sum")
{
	constexpr domain::CardCode spell_code{97000125};
	constexpr domain::CardCode ritual_code{97000130};
	constexpr domain::CardCode material_code{97000131};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::RITUAL});
	game.register_definition(spell_definition);
	auto ritual = make_monster(ritual_code, 2000, 2000, 4);
	ritual.subtypes = {domain::CardSubtype::RITUAL};
	game.register_definition(ritual);
	const auto material_definition = make_monster(material_code, 1000, 1000, 2);
	game.register_definition(material_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	const auto ritual_card = game.create_instance(ritual_code, domain::Player::FIRST);
	REQUIRE(ritual_card.valid());
	const auto material = game.create_instance(material_code, domain::Player::FIRST);
	REQUIRE(material.valid());

	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(ritual_card), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(material), domain::HandPlacement{.index = 2, .position = domain::Position::FACE_DOWN_DEFENSE});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(spell));
	settle(duel);

	CHECK_FALSE(duel.pending_summon());
	CHECK(game.zone_of(ritual_card) == domain::Zone::HAND);
	CHECK(game.zone_of(material) == domain::Zone::HAND);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel negates a Ritual Spell activation")
{
	constexpr domain::CardCode spell_code{97000125};
	constexpr domain::CardCode ritual_code{97000130};
	constexpr domain::CardCode negator_code{97000134};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::RITUAL});
	game.register_definition(spell_definition);
	auto ritual = make_monster(ritual_code, 2000, 2000, 4);
	ritual.subtypes = {domain::CardSubtype::RITUAL};
	game.register_definition(ritual);
	const auto negator_definition = make_monster(negator_code);
	game.register_definition(negator_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	const auto ritual_card = game.create_instance(ritual_code, domain::Player::FIRST);
	REQUIRE(ritual_card.valid());
	const auto negator = game.create_instance(negator_code, domain::Player::SECOND);
	REQUIRE(negator.valid());

	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(ritual_card), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(negator), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(spell));
	CHECK(duel.activate_effect(negator, 0));
	REQUIRE(duel.chain().size() == 2);

	settle(duel);

	CHECK_FALSE(duel.pending_summon());
	CHECK(game.zone_of(ritual_card) == domain::Zone::HAND);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(negator) == domain::Zone::MONSTER);
}

TEST_CASE("flow::Duel cannot summon-negate an effect-driven Ritual Summon")
{
	constexpr domain::CardCode solemn_code{41420027};
	constexpr domain::CardCode spell_code{97000125};
	constexpr domain::CardCode ritual_code{97000130};
	constexpr domain::CardCode material_code{97000133};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto solemn_definition = make_trap(solemn_code, {domain::CardSubtype::COUNTER});
	game.register_definition(solemn_definition);
	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::RITUAL});
	game.register_definition(spell_definition);
	auto ritual = make_monster(ritual_code, 2000, 2000, 4);
	ritual.subtypes = {domain::CardSubtype::RITUAL};
	game.register_definition(ritual);
	const auto material_definition = make_monster(material_code, 1000, 1000, 4);
	game.register_definition(material_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto solemn = game.create_instance(solemn_code, domain::Player::FIRST);
	REQUIRE(solemn.valid());
	const auto spell = game.create_instance(spell_code, domain::Player::SECOND);
	REQUIRE(spell.valid());
	const auto ritual_card = game.create_instance(ritual_code, domain::Player::SECOND);
	REQUIRE(ritual_card.valid());
	const auto material = game.create_instance(material_code, domain::Player::SECOND);
	REQUIRE(material.valid());

	game.place(*game.find_instance_mutable(solemn), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(ritual_card), domain::HandPlacement{.index = 2, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(material), domain::HandPlacement{.index = 3, .position = domain::Position::FACE_DOWN_DEFENSE});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(duel.set_spell_trap(solemn, domain::FieldColumn::C1));
	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1

	CHECK(duel.activate_card(spell));
	CHECK(duel.chain().size() == 1);
	settle(duel);

	CHECK_FALSE(duel.pending_summon());
	CHECK(game.zone_of(ritual_card) == domain::Zone::MONSTER);
	CHECK(game.zone_of(material) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(solemn) == domain::Zone::SPELL_TRAP);
	CHECK(game.life_points(domain::Player::FIRST) == 8000);

	// 效果召唤不会打开 SPECIAL_SUMMON_DECLARED 窗口。
	CHECK_FALSE(duel.windows().empty());
	CHECK(
		std::ranges::none_of(
			duel.windows(),
			[](const execution::EventWindow& window) noexcept -> bool
			{
			const auto* code = std::get_if<domain::EventCode>(&window.source);
			return code != nullptr && *code == domain::EventCode::SPECIAL_SUMMON_DECLARED;
			}
		)
	);
}

TEST_CASE("flow::Duel resolves an effect-driven Special Summon from the Graveyard")
{
	constexpr domain::CardCode spell_code{97000150};
	constexpr domain::CardCode monster_code{97000151};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::NORMAL});
	game.register_definition(spell_definition);
	const auto monster_definition = make_monster(monster_code, 1800, 1200, 4);
	game.register_definition(monster_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(monster.valid());

	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(monster), domain::GraveyardPlacement{.index = 0});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(spell));
	CHECK(duel.chain().size() == 1);
	settle(duel);

	CHECK_FALSE(duel.pending_summon());
	CHECK(game.zone_of(monster) == domain::Zone::MONSTER);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);
	CHECK(game.find_instance(monster)->summon_record().method == domain::SummonMethod::SPECIAL);
	CHECK(
		std::ranges::none_of(
			duel.windows(),
			[](const execution::EventWindow& window) noexcept -> bool
			{
			const auto* code = std::get_if<domain::EventCode>(&window.source);
			return code != nullptr && *code == domain::EventCode::SPECIAL_SUMMON_DECLARED;
			}
		)
	);
}

TEST_CASE("flow::Duel uses the selector for effect special summon placement")
{
	constexpr domain::CardCode spell_code{97000150};
	constexpr domain::CardCode monster_code{97000151};

	state::CardInstanceId monster_id{};
	driver::LocalTestSelector selector
	{
			nullptr,
			[&](
		[[maybe_unused]] const domain::Player player,
		const state::CardInstanceId card,
		const std::span<const domain::MonsterLocation> candidates
	) noexcept -> std::optional<domain::MonsterLocation>
			{
				if (card == monster_id)
				{
					for (const auto candidate: candidates)
					{
						if (candidate.column == domain::FieldColumn::C3)
						{
							return candidate;
						}
					}
				}

				return candidates.empty() ? std::nullopt : std::optional<domain::MonsterLocation>{candidates.front()};
			},
			nullptr,
			nullptr,
	};

	flow::Duel duel{selector};
	auto& game = duel.game();

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::NORMAL});
	game.register_definition(spell_definition);
	const auto monster_definition = make_monster(monster_code, 1800, 1200, 4);
	game.register_definition(monster_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(monster.valid());
	monster_id = monster;

	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(monster), domain::GraveyardPlacement{.index = 0});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(spell));
	settle(duel);

	const auto placement_value = game.find_instance(monster)->placement();
	const auto* placement = std::get_if<domain::MonsterPlacement>(&placement_value);
	REQUIRE(placement != nullptr);
	CHECK(game.zone_of(monster) == domain::Zone::MONSTER);
	CHECK(placement->column == domain::FieldColumn::C3);
}

TEST_CASE("flow::Duel does not fall back when effect special summon location is rejected")
{
	constexpr domain::CardCode spell_code{97000150};
	constexpr domain::CardCode monster_code{97000151};

	driver::LocalTestSelector selector
	{
			nullptr,
			[](const domain::Player, const state::CardInstanceId, const std::span<const domain::MonsterLocation>) noexcept -> std::optional<domain::MonsterLocation>
			{
				return std::nullopt;
			},
			nullptr,
			nullptr,
	};

	flow::Duel duel{selector};
	auto& game = duel.game();

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::NORMAL});
	game.register_definition(spell_definition);
	const auto monster_definition = make_monster(monster_code, 1800, 1200, 4);
	game.register_definition(monster_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(monster.valid());

	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(monster), domain::GraveyardPlacement{.index = 0});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(spell));
	settle(duel);

	CHECK(game.zone_of(monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel uses the selector for spell trap placement")
{
	constexpr domain::CardCode spell_code{97000150};
	constexpr domain::CardCode monster_code{97000151};

	driver::LocalTestSelector selector
	{
			nullptr,
			nullptr,
			[](const domain::Player, const state::CardInstanceId, const std::span<const domain::SpellTrapLocation> candidates) noexcept -> std::optional<domain::SpellTrapLocation>
			{
				for (const auto candidate: candidates)
				{
					if (candidate.column == domain::FieldColumn::C4)
					{
						return candidate;
					}
				}

				return candidates.empty() ? std::nullopt : std::optional<domain::SpellTrapLocation>{candidates.front()};
			},
			nullptr,
	};

	flow::Duel duel{selector};
	auto& game = duel.game();

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::NORMAL});
	game.register_definition(spell_definition);
	const auto monster_definition = make_monster(monster_code, 1800, 1200, 4);
	game.register_definition(monster_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(monster.valid());

	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(monster), domain::GraveyardPlacement{.index = 0});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(spell));

	const auto placement_value = game.find_instance(spell)->placement();
	const auto* placement = std::get_if<domain::SpellTrapPlacement>(&placement_value);
	REQUIRE(placement != nullptr);
	CHECK(placement->column == domain::FieldColumn::C4);

	settle(duel);
}

TEST_CASE("flow::Duel negates an effect-driven Special Summon effect")
{
	constexpr domain::CardCode spell_code{97000150};
	constexpr domain::CardCode monster_code{97000151};
	constexpr domain::CardCode negator_code{97000152};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::NORMAL});
	game.register_definition(spell_definition);
	const auto monster_definition = make_monster(monster_code, 1800, 1200, 4);
	game.register_definition(monster_definition);
	const auto negator_definition = make_monster(negator_code);
	game.register_definition(negator_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(monster.valid());
	const auto negator = game.create_instance(negator_code, domain::Player::SECOND);
	REQUIRE(negator.valid());

	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(monster), domain::GraveyardPlacement{.index = 0});
	game.place(*game.find_instance_mutable(negator), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(spell));
	CHECK(duel.activate_effect(negator, 0));
	REQUIRE(duel.chain().size() == 2);

	settle(duel);

	CHECK_FALSE(duel.pending_summon());
	CHECK(game.zone_of(monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(negator) == domain::Zone::MONSTER);
}

TEST_CASE("flow::Duel requires a proper summon before reviving an Extra Deck monster")
{
	constexpr domain::CardCode spell_code{97000150};
	constexpr domain::CardCode fusion_code{97000153};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::NORMAL});
	game.register_definition(spell_definition);
	auto fusion_definition = make_monster(fusion_code, 2000, 2000, 4);
	fusion_definition.subtypes = {domain::CardSubtype::FUSION};
	game.register_definition(fusion_definition);

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto first_spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(first_spell.valid());
	const auto second_spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(second_spell.valid());
	const auto fusion = game.create_instance(fusion_code, domain::Player::FIRST);
	REQUIRE(fusion.valid());

	game.place(*game.find_instance_mutable(first_spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(second_spell), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(fusion), domain::GraveyardPlacement{.index = 0});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	// 直接从额外卡组送入墓地的融合怪兽不能苏生。
	CHECK(duel.activate_card(first_spell));
	settle(duel);
	CHECK(game.zone_of(fusion) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(first_spell) == domain::Zone::GRAVEYARD);
	CHECK_FALSE(game.find_instance(fusion)->properly_summoned());

	// 非正规召唤记录不会置位正规召唤状态。
	game.set_summon_record(
		*game.find_instance_mutable(fusion),
		state::CardInstance::SummonRecord
		{
				.player = domain::Player::FIRST,
				.method = domain::SummonMethod::SPECIAL,
				.from = {domain::GraveyardPlacement{.index = 0}},
				.materials = {},
		}
	);
	CHECK_FALSE(game.find_instance(fusion)->properly_summoned());

	// 模拟已正规融合召唤后送墓。
	game.set_summon_record(
		*game.find_instance_mutable(fusion),
		state::CardInstance::SummonRecord
		{
				.player = domain::Player::FIRST,
				.method = domain::SummonMethod::FUSION,
				.from = {domain::ExtraDeckPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE}},
				.materials = {},
		}
	);

	CHECK(duel.activate_card(second_spell));
	settle(duel);
	CHECK(game.zone_of(fusion) == domain::Zone::MONSTER);
	CHECK(game.zone_of(second_spell) == domain::Zone::GRAVEYARD);
	CHECK(game.find_instance(fusion)->summon_record().method == domain::SummonMethod::SPECIAL);
	CHECK(game.find_instance(fusion)->properly_summoned());

	// 回到额外卡组后失去正规召唤状态
	game.place(
		*game.find_instance_mutable(fusion),
		domain::ExtraDeckPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE}
	);
	CHECK(game.zone_of(fusion) == domain::Zone::EXTRA_DECK);
	CHECK_FALSE(game.find_instance(fusion)->properly_summoned());
}

TEST_CASE("flow::Duel rejects a non-monster target for an effect Special Summon")
{
	constexpr domain::CardCode summon_spell_code{97000157};
	constexpr domain::CardCode target_spell_code{97000125};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto summon_spell_definition = make_spell(summon_spell_code, {domain::CardSubtype::NORMAL});
	game.register_definition(summon_spell_definition);
	const auto target_spell_definition = make_spell(target_spell_code, {domain::CardSubtype::RITUAL});
	game.register_definition(target_spell_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));

	const auto summon_spell = game.create_instance(summon_spell_code, domain::Player::FIRST);
	REQUIRE(summon_spell.valid());
	const auto target_spell = game.create_instance(target_spell_code, domain::Player::FIRST);
	REQUIRE(target_spell.valid());

	game.place(*game.find_instance_mutable(summon_spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(target_spell), domain::GraveyardPlacement{.index = 0});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(summon_spell));
	settle(duel);

	CHECK(game.zone_of(target_spell) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(summon_spell) == domain::Zone::GRAVEYARD);
	CHECK_FALSE(duel.pending_summon());
}

TEST_CASE("flow::Duel rejects an unproper Ritual Monster from hand")
{
	constexpr domain::CardCode summon_spell_code{97000158};
	constexpr domain::CardCode ritual_code{97000130};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto summon_spell_definition = make_spell(summon_spell_code, {domain::CardSubtype::RITUAL});
	game.register_definition(summon_spell_definition);
	auto ritual = make_monster(ritual_code, 2000, 2000, 4);
	ritual.subtypes = {domain::CardSubtype::RITUAL};
	game.register_definition(ritual);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));

	const auto summon_spell = game.create_instance(summon_spell_code, domain::Player::FIRST);
	REQUIRE(summon_spell.valid());
	const auto ritual_card = game.create_instance(ritual_code, domain::Player::FIRST);
	REQUIRE(ritual_card.valid());

	game.place(*game.find_instance_mutable(summon_spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(ritual_card), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(summon_spell));
	settle(duel);

	CHECK(game.zone_of(ritual_card) == domain::Zone::HAND);
	CHECK(game.zone_of(summon_spell) == domain::Zone::GRAVEYARD);
	CHECK_FALSE(duel.pending_summon());
}

TEST_CASE("flow::Duel negates an effect-driven Special Summon activation")
{
	constexpr domain::CardCode spell_code{97000150};
	constexpr domain::CardCode monster_code{97000151};
	constexpr domain::CardCode negator_code{97000134};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::NORMAL});
	game.register_definition(spell_definition);
	const auto monster_definition = make_monster(monster_code, 1800, 1200, 4);
	game.register_definition(monster_definition);
	const auto negator_definition = make_monster(negator_code);
	game.register_definition(negator_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(monster.valid());
	const auto negator = game.create_instance(negator_code, domain::Player::SECOND);
	REQUIRE(negator.valid());

	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(monster), domain::GraveyardPlacement{.index = 0});
	game.place(*game.find_instance_mutable(negator), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(spell));
	CHECK(duel.activate_effect(negator, 0));
	REQUIRE(duel.chain().size() == 2);
	settle(duel);

	CHECK(game.zone_of(monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(negator) == domain::Zone::MONSTER);
}

TEST_CASE("flow::Duel cannot summon-negate an effect-driven Special Summon")
{
	constexpr domain::CardCode solemn_code{41420027};
	constexpr domain::CardCode spell_code{97000150};
	constexpr domain::CardCode monster_code{97000151};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto solemn_definition = make_trap(solemn_code, {domain::CardSubtype::COUNTER});
	game.register_definition(solemn_definition);
	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::NORMAL});
	game.register_definition(spell_definition);
	const auto monster_definition = make_monster(monster_code, 1800, 1200, 4);
	game.register_definition(monster_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto solemn = game.create_instance(solemn_code, domain::Player::FIRST);
	REQUIRE(solemn.valid());
	const auto spell = game.create_instance(spell_code, domain::Player::SECOND);
	REQUIRE(spell.valid());
	const auto monster = game.create_instance(monster_code, domain::Player::SECOND);
	REQUIRE(monster.valid());

	game.place(*game.find_instance_mutable(solemn), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(monster), domain::GraveyardPlacement{.index = 0});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(duel.set_spell_trap(solemn, domain::FieldColumn::C1));
	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1

	CHECK(duel.activate_card(spell));
	settle(duel);

	CHECK_FALSE(duel.pending_summon());
	CHECK(game.zone_of(monster) == domain::Zone::MONSTER);
	CHECK(game.find_instance(monster)->controller() == domain::Player::SECOND);
	CHECK(game.zone_of(solemn) == domain::Zone::SPELL_TRAP);
	CHECK(game.life_points(domain::Player::FIRST) == 8000);
	CHECK(
		std::ranges::none_of(
			duel.windows(),
			[](const execution::EventWindow& window) noexcept -> bool
			{
			const auto* code = std::get_if<domain::EventCode>(&window.source);
			return code != nullptr && *code == domain::EventCode::SPECIAL_SUMMON_DECLARED;
			}
		)
	);
}

TEST_CASE("flow::Duel resolves an effect Special Summon from the Banished zone")
{
	constexpr domain::CardCode spell_code{97000159};
	constexpr domain::CardCode monster_code{97000151};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::NORMAL});
	game.register_definition(spell_definition);
	const auto monster_definition = make_monster(monster_code, 1800, 1200, 4);
	game.register_definition(monster_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(monster.valid());

	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(monster), domain::BanishedPlacement{.index = 0, .position = domain::Position::FACE_UP_ATTACK});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(spell));
	settle(duel);

	CHECK(game.zone_of(monster) == domain::Zone::MONSTER);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);
	CHECK(game.find_instance(monster)->summon_record().method == domain::SummonMethod::SPECIAL);
}

TEST_CASE("flow::Duel takes control with an effect Special Summon from the opponent's Graveyard")
{
	constexpr domain::CardCode spell_code{97000154};
	constexpr domain::CardCode monster_code{97000151};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::NORMAL});
	game.register_definition(spell_definition);
	const auto monster_definition = make_monster(monster_code, 1800, 1200, 4);
	game.register_definition(monster_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	const auto monster = game.create_instance(monster_code, domain::Player::SECOND);
	REQUIRE(monster.valid());

	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(monster), domain::GraveyardPlacement{.index = 0});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	CHECK(duel.activate_card(spell));
	settle(duel);

	CHECK_FALSE(duel.pending_summon());
	CHECK(game.zone_of(monster) == domain::Zone::MONSTER);
	CHECK(game.find_instance(monster)->controller() == domain::Player::FIRST);
	CHECK(game.find_instance(monster)->owner() == domain::Player::SECOND);
	CHECK(game.find_instance(monster)->summon_record().player == domain::Player::FIRST);
	CHECK(game.find_instance(monster)->summon_record().method == domain::SummonMethod::SPECIAL);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);

	// 离开场上后控制权回到持有者,并进入持有者墓地
	game.move_to_graveyard(*game.find_instance_mutable(monster), {domain::Reason::RULE});
	CHECK(game.find_instance(monster)->move_provenance().controller == domain::Player::FIRST);
	CHECK(game.find_instance(monster)->controller() == domain::Player::SECOND);
	CHECK(game.zone_of(monster) == domain::Zone::GRAVEYARD);
	CHECK(game.count_zone_card(domain::Player::SECOND, domain::Zone::GRAVEYARD) == 1);
}

TEST_CASE("flow::Duel matches Fusion materials by code_list")
{
	constexpr domain::CardCode fusion_code{97000137};
	constexpr domain::CardCode alias_material_code{97000136};
	constexpr domain::CardCode plain_material_code{97000138};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	auto fusion = make_monster(fusion_code, 2000, 2000, 4);
	fusion.subtypes = {domain::CardSubtype::FUSION};
	game.register_definition(fusion);
	const auto alias_material_definition = make_monster(alias_material_code, 1000, 1000, 4);
	game.register_definition(alias_material_definition);
	const auto plain_material_definition = make_monster(plain_material_code, 1000, 1000, 4);
	game.register_definition(plain_material_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));
	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto fusion_card = game.create_instance(fusion_code, domain::Player::FIRST);
	REQUIRE(fusion_card.valid());
	const auto plain_material = game.create_instance(plain_material_code, domain::Player::FIRST);
	REQUIRE(plain_material.valid());
	const auto alias_material = game.create_instance(alias_material_code, domain::Player::FIRST);
	REQUIRE(alias_material.valid());

	game.place(*game.find_instance_mutable(fusion_card), domain::ExtraDeckPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(plain_material), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});
	game.place(*game.find_instance_mutable(alias_material), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});

	advance_phase(duel); // STANDBY
	advance_phase(duel); // MAIN_1

	const std::array plain_materials{plain_material};
	CHECK_FALSE(duel.fusion_summon(fusion_card, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, plain_materials));

	const std::array alias_materials{alias_material};
	CHECK(duel.fusion_summon(fusion_card, domain::ExtraMonsterPlacement{.column = domain::FieldColumn::EX1, .position = domain::Position::FACE_UP_ATTACK}, alias_materials));
	resolve_summon(duel);

	CHECK(game.zone_of(fusion_card) == domain::Zone::EXTRA_MONSTER);
	CHECK(game.zone_of(alias_material) == domain::Zone::GRAVEYARD);
	CHECK(game.find_instance(fusion_card)->summon_record().method == domain::SummonMethod::FUSION);
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

	const auto card_97000001_definition = make_trap(domain::CardCode{97000001}, {domain::CardSubtype::NORMAL});
	game.register_definition(card_97000001_definition);
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

	const auto attacker_definition = make_monster(attacker_code);
	game.register_definition(attacker_definition);
	const auto negator_definition = make_monster(negator_code);
	game.register_definition(negator_definition);
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

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::NORMAL});
	game.register_definition(spell_definition);
	const auto negator_definition = make_monster(negator_code);
	game.register_definition(negator_definition);
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

		const auto activate_definition = make_spell(activate_code, {domain::CardSubtype::NORMAL});
		game.register_definition(activate_definition);
		const auto use_definition = make_spell(use_code, {domain::CardSubtype::NORMAL});
		game.register_definition(use_definition);
		const auto negator_definition = make_monster(negator_code);
		game.register_definition(negator_definition);
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

	const auto first_quick_definition = make_monster(first_quick_code);
	game.register_definition(first_quick_definition);
	const auto counter_definition = make_trap(counter_code, {domain::CardSubtype::COUNTER});
	game.register_definition(counter_definition);
	const auto second_quick_definition = make_monster(second_quick_code);
	game.register_definition(second_quick_definition);
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

	const auto destroyer_definition = make_monster(destroyer_code);
	game.register_definition(destroyer_definition);
	const auto victim_definition = make_monster(victim_code);
	game.register_definition(victim_definition);
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

	const auto double_definition = make_monster(double_code);
	game.register_definition(double_definition);
	const auto opponent_definition = make_monster(opponent_code);
	game.register_definition(opponent_definition);
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

	const auto victim_definition = make_monster(victim_code);
	game.register_definition(victim_definition);
	const auto destroyer_definition = make_monster(destroyer_code);
	game.register_definition(destroyer_definition);
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

	const auto victim_definition = make_monster(victim_code);
	game.register_definition(victim_definition);
	const auto destroyer_definition = make_monster(destroyer_code);
	game.register_definition(destroyer_definition);
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

	const auto victim_definition = make_monster(victim_code);
	game.register_definition(victim_definition);
	const auto destroyer_definition = make_monster(destroyer_code);
	game.register_definition(destroyer_definition);
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
			nullptr,
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
			},
	};
	flow::Duel duel{selector};
	auto& game = duel.game();

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto victim_definition = make_monster(victim_code);
	game.register_definition(victim_definition);
	const auto destroyer_definition = make_monster(destroyer_code);
	game.register_definition(destroyer_definition);
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

	const auto* script = duel.find_script(victim_code);
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
			nullptr,
			nullptr,
			nullptr,
	};
	flow::Duel duel{selector};
	auto& game = duel.game();

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell_definition = make_spell(spell_code, {domain::CardSubtype::NORMAL});
	game.register_definition(spell_definition);
	const auto target_definition = make_monster(target_code);
	game.register_definition(target_definition);
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

TEST_CASE("flow::Duel resolves Reinforcement of the Army")
{
	constexpr domain::CardCode rota_code{32807846};
	constexpr domain::CardCode warrior_code{97001001};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto rota_definition = make_spell(rota_code, {domain::CardSubtype::NORMAL});
	const auto warrior_definition = make_monster(warrior_code, 1000, 1000, 4, domain::Race::WARRIOR);
	game.register_definition(rota_definition);
	game.register_definition(warrior_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto rota = game.create_instance(rota_code, domain::Player::FIRST);
	REQUIRE(rota.valid());
	game.place(*game.find_instance_mutable(rota), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto warrior = game.create_instance(warrior_code, domain::Player::FIRST);
	REQUIRE(warrior.valid());
	game.place(*game.find_instance_mutable(warrior), domain::DeckPlacement{.index = 0});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(rota));
	settle(duel);

	CHECK(game.zone_of(warrior) == domain::Zone::HAND);
	CHECK(game.zone_of(rota) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel resolves Mystical Space Typhoon")
{
	constexpr domain::CardCode mystical_space_typhoon_code{5318639};
	constexpr domain::CardCode set_trap_code{97001004};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto mystical_space_typhoon_definition = make_spell(
		mystical_space_typhoon_code,
		{domain::CardSubtype::QUICK_PLAY}
	);
	const auto set_trap_definition = make_trap(set_trap_code, {domain::CardSubtype::NORMAL});
	game.register_definition(mystical_space_typhoon_definition);
	game.register_definition(set_trap_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto mystical_space_typhoon = game.create_instance(mystical_space_typhoon_code, domain::Player::FIRST);
	REQUIRE(mystical_space_typhoon.valid());
	game.place(
		*game.find_instance_mutable(mystical_space_typhoon),
		domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE}
	);

	const auto set_trap = game.create_instance(set_trap_code, domain::Player::SECOND);
	REQUIRE(set_trap.valid());
	game.place(
		*game.find_instance_mutable(set_trap),
		domain::SpellTrapPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_DOWN_DEFENSE}
	);

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(mystical_space_typhoon));
	settle(duel);

	CHECK(game.zone_of(set_trap) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(mystical_space_typhoon) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel resolves Monster Reborn")
{
	constexpr domain::CardCode monster_reborn_code{83764718};
	constexpr domain::CardCode monster_code{97001005};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto monster_reborn_definition = make_spell(monster_reborn_code, {domain::CardSubtype::NORMAL});
	const auto monster_definition = make_monster(monster_code, 1800, 1200, 4, domain::Race::DRAGON);
	game.register_definition(monster_reborn_definition);
	game.register_definition(monster_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto monster_reborn = game.create_instance(monster_reborn_code, domain::Player::FIRST);
	REQUIRE(monster_reborn.valid());
	game.place(
		*game.find_instance_mutable(monster_reborn),
		domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE}
	);

	const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(monster.valid());
	game.place(*game.find_instance_mutable(monster), domain::GraveyardPlacement{.index = 0});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(monster_reborn));
	settle(duel);

	CHECK(game.zone_of(monster) == domain::Zone::MONSTER);
	CHECK(game.zone_of(monster_reborn) == domain::Zone::GRAVEYARD);
	CHECK(game.find_instance(monster)->summon_record().method == domain::SummonMethod::SPECIAL);
}

TEST_CASE("flow::Duel resolves Silver's Cry")
{
	constexpr domain::CardCode silvers_cry_code{87025064};
	constexpr domain::CardCode dragon_code{97001006};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto silvers_cry_definition = make_spell(silvers_cry_code, {domain::CardSubtype::QUICK_PLAY});
	const auto dragon_definition = make_monster(dragon_code, 1800, 1200, 4, domain::Race::DRAGON);
	game.register_definition(silvers_cry_definition);
	game.register_definition(dragon_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto silvers_cry = game.create_instance(silvers_cry_code, domain::Player::FIRST);
	REQUIRE(silvers_cry.valid());
	game.place(
		*game.find_instance_mutable(silvers_cry),
		domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE}
	);

	const auto dragon = game.create_instance(dragon_code, domain::Player::FIRST);
	REQUIRE(dragon.valid());
	game.place(*game.find_instance_mutable(dragon), domain::GraveyardPlacement{.index = 0});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(silvers_cry));
	settle(duel);

	CHECK(game.zone_of(dragon) == domain::Zone::MONSTER);
	CHECK(game.zone_of(silvers_cry) == domain::Zone::GRAVEYARD);
	CHECK(game.find_instance(dragon)->summon_record().method == domain::SummonMethod::SPECIAL);
}

TEST_CASE("flow::Duel resolves Trade-In")
{
	constexpr domain::CardCode trade_in_code{38120068};
	constexpr domain::CardCode level_eight_dragon_code{97001007};
	constexpr domain::CardCode drawn_monster_code{97001008};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto trade_in_definition = make_spell(trade_in_code, {domain::CardSubtype::NORMAL});
	const auto level_eight_dragon_definition = make_monster(
		level_eight_dragon_code,
		3000,
		2500,
		8,
		domain::Race::DRAGON
	);
	const auto drawn_monster_definition = make_monster(drawn_monster_code, 1000, 1000, 4);
	game.register_definition(trade_in_definition);
	game.register_definition(level_eight_dragon_definition);
	game.register_definition(drawn_monster_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto trade_in = game.create_instance(trade_in_code, domain::Player::FIRST);
	REQUIRE(trade_in.valid());
	game.place(*game.find_instance_mutable(trade_in), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto level_eight_dragon = game.create_instance(level_eight_dragon_code, domain::Player::FIRST);
	REQUIRE(level_eight_dragon.valid());
	game.place(*game.find_instance_mutable(level_eight_dragon), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto first_draw = game.create_instance(drawn_monster_code, domain::Player::FIRST);
	REQUIRE(first_draw.valid());
	game.place(*game.find_instance_mutable(first_draw), domain::DeckPlacement{.index = 0});

	const auto second_draw = game.create_instance(drawn_monster_code, domain::Player::FIRST);
	REQUIRE(second_draw.valid());
	game.place(*game.find_instance_mutable(second_draw), domain::DeckPlacement{.index = 1});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(trade_in));
	settle(duel);

	CHECK(game.zone_of(trade_in) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(level_eight_dragon) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(first_draw) == domain::Zone::HAND);
	CHECK(game.zone_of(second_draw) == domain::Zone::HAND);
	CHECK(game.count_zone_card(domain::Player::FIRST, domain::Zone::DECK) == 0);
}

TEST_CASE("flow::Duel resolves Book of Moon")
{
	constexpr domain::CardCode book_of_moon_code{14087893};
	constexpr domain::CardCode monster_code{97001009};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto book_of_moon_definition = make_spell(book_of_moon_code, {domain::CardSubtype::QUICK_PLAY});
	const auto monster_definition = make_monster(monster_code, 1800, 1200, 4);
	game.register_definition(book_of_moon_definition);
	game.register_definition(monster_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto book_of_moon = game.create_instance(book_of_moon_code, domain::Player::FIRST);
	REQUIRE(book_of_moon.valid());
	game.place(
		*game.find_instance_mutable(book_of_moon),
		domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE}
	);

	const auto monster = game.create_instance(monster_code, domain::Player::SECOND);
	REQUIRE(monster.valid());
	game.place(
		*game.find_instance_mutable(monster),
		domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK}
	);

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(book_of_moon));
	settle(duel);

	const auto placement = game.find_instance(monster)->placement();
	const auto* monster_placement = std::get_if<domain::MonsterPlacement>(&placement);
	REQUIRE(monster_placement != nullptr);
	CHECK(monster_placement->position == domain::Position::FACE_DOWN_DEFENSE);
	CHECK(game.zone_of(book_of_moon) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel resolves Raigeki Break")
{
	constexpr domain::CardCode raigeki_break_code{4178474};
	constexpr domain::CardCode cost_code{97001010};
	constexpr domain::CardCode target_code{97001011};

	state::CardInstanceId target_id{};
	driver::LocalTestSelector selector
	{
			[&]([[maybe_unused]] const domain::Player player, const execution::Group& candidates, [[maybe_unused]] const std::size_t min, [[maybe_unused]] const std::size_t max) -> execution::Group
			{
				if (std::ranges::contains(candidates, target_id))
				{
					return execution::Group{std::vector<state::CardInstanceId>{target_id}};
				}
				if (candidates.size() == 1)
				{
					return execution::Group{candidates.begin(), candidates.end()};
				}
				return {};
			},
			nullptr,
			nullptr,
			nullptr,
	};
	flow::Duel duel{selector};
	auto& game = duel.game();

	const auto raigeki_break_definition = make_trap(raigeki_break_code, {domain::CardSubtype::NORMAL});
	const auto cost_definition = make_monster(cost_code, 1000, 1000, 4);
	const auto target_definition = make_monster(target_code, 1000, 1000, 4);
	game.register_definition(raigeki_break_definition);
	game.register_definition(cost_definition);
	game.register_definition(target_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto raigeki_break = game.create_instance(raigeki_break_code, domain::Player::FIRST);
	REQUIRE(raigeki_break.valid());
	game.place(*game.find_instance_mutable(raigeki_break), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto cost = game.create_instance(cost_code, domain::Player::FIRST);
	REQUIRE(cost.valid());
	game.place(*game.find_instance_mutable(cost), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto target = game.create_instance(target_code, domain::Player::SECOND);
	REQUIRE(target.valid());
	target_id = target;
	game.place(*game.find_instance_mutable(target), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	CHECK(duel.set_spell_trap(raigeki_break, domain::FieldColumn::C1));

	CHECK(advance_phase(duel)); // END
	CHECK(duel.end_turn());

	CHECK_FALSE(duel.pass_priority());
	CHECK(duel.activate_card(raigeki_break));
	settle(duel);

	CHECK(game.zone_of(cost) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(target) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(raigeki_break) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel resolves Dark Hole")
{
	constexpr domain::CardCode dark_hole_code{53129443};
	constexpr domain::CardCode own_monster_code{97001002};
	constexpr domain::CardCode opponent_monster_code{97001003};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	const auto dark_hole_definition = make_spell(dark_hole_code, {domain::CardSubtype::NORMAL});
	const auto own_monster_definition = make_monster(own_monster_code, 1000, 1000, 4);
	const auto opponent_monster_definition = make_monster(opponent_monster_code, 1000, 1000, 4);
	game.register_definition(dark_hole_definition);
	game.register_definition(own_monster_definition);
	game.register_definition(opponent_monster_definition);
	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto dark_hole = game.create_instance(dark_hole_code, domain::Player::FIRST);
	REQUIRE(dark_hole.valid());
	game.place(*game.find_instance_mutable(dark_hole), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto own_monster = game.create_instance(own_monster_code, domain::Player::FIRST);
	REQUIRE(own_monster.valid());
	game.place(*game.find_instance_mutable(own_monster), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto opponent_monster = game.create_instance(opponent_monster_code, domain::Player::SECOND);
	REQUIRE(opponent_monster.valid());
	game.place(*game.find_instance_mutable(opponent_monster), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(dark_hole));
	settle(duel);

	CHECK(game.zone_of(own_monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(opponent_monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(dark_hole) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel pays life and sends an archetype card to the graveyard")
{
	constexpr domain::CardCode spell_code{97000160};
	constexpr domain::CardCode target_code{97000161};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));

	const auto* spell_script = duel.find_script(spell_code);
	REQUIRE(spell_script != nullptr);
	const auto* target_script = duel.find_script(target_code);
	REQUIRE(target_script != nullptr);

	game.register_definition(spell_script->identity);
	game.register_definition(target_script->identity);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto target = game.create_instance(target_code, domain::Player::FIRST);
	REQUIRE(target.valid());
	game.place(*game.find_instance_mutable(target), domain::DeckPlacement{.index = 0});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(spell));
	settle(duel);

	CHECK(game.life_points(domain::Player::FIRST) == 7000);
	CHECK(game.zone_of(target) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel releases a monster as an effect")
{
	constexpr domain::CardCode spell_code{97000162};
	constexpr domain::CardCode monster_code{97000163};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));

	const auto* spell_script = duel.find_script(spell_code);
	REQUIRE(spell_script != nullptr);
	const auto* monster_script = duel.find_script(monster_code);
	REQUIRE(monster_script != nullptr);

	game.register_definition(spell_script->identity);
	game.register_definition(monster_script->identity);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(spell.valid());
	game.place(*game.find_instance_mutable(spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto monster = game.create_instance(monster_code, domain::Player::FIRST);
	REQUIRE(monster.valid());
	game.place(*game.find_instance_mutable(monster), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(spell));
	settle(duel);

	CHECK(game.zone_of(monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(spell) == domain::Zone::GRAVEYARD);
}


TEST_CASE("flow::Duel resolves Dragon Shrine and triggers The White Stone of Legend")
{
	constexpr domain::CardCode dragon_shrine_code{41620959};
	constexpr domain::CardCode white_stone_code{79814787};
	constexpr domain::CardCode blue_eyes_code{89631139};

	driver::LocalTestSelector selector
	{
			[](domain::Player, const execution::Group& candidates, const std::size_t min, const std::size_t max) -> execution::Group
			{
				const auto count = min == 0 ? std::size_t{0} : std::ranges::min(candidates.size(), max);
				return {candidates.begin(), candidates.begin() + static_cast<std::ptrdiff_t>(count)};
			},
			nullptr,
			nullptr,
			choose_all_triggers,
	};

	flow::Duel duel{selector};
	auto& game = duel.game();

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	const auto* dragon_shrine_script = duel.find_script(dragon_shrine_code);
	REQUIRE(dragon_shrine_script != nullptr);
	const auto* white_stone_script = duel.find_script(white_stone_code);
	REQUIRE(white_stone_script != nullptr);
	const auto* blue_eyes_script = duel.find_script(blue_eyes_code);
	REQUIRE(blue_eyes_script != nullptr);

	game.register_definition(dragon_shrine_script->identity);
	game.register_definition(white_stone_script->identity);
	game.register_definition(blue_eyes_script->identity);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto dragon_shrine = game.create_instance(dragon_shrine_code, domain::Player::FIRST);
	REQUIRE(dragon_shrine.valid());
	game.place(*game.find_instance_mutable(dragon_shrine), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto white_stone = game.create_instance(white_stone_code, domain::Player::FIRST);
	REQUIRE(white_stone.valid());
	game.place(*game.find_instance_mutable(white_stone), domain::DeckPlacement{.index = 0});

	const auto blue_eyes = game.create_instance(blue_eyes_code, domain::Player::FIRST);
	REQUIRE(blue_eyes.valid());
	game.place(*game.find_instance_mutable(blue_eyes), domain::DeckPlacement{.index = 1});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(dragon_shrine));
	settle(duel);

	CHECK(game.zone_of(white_stone) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(blue_eyes) == domain::Zone::HAND);
	CHECK(game.zone_of(dragon_shrine) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel lets Dragon Shrine send a second Normal Dragon")
{
	constexpr domain::CardCode dragon_shrine_code{41620959};
	constexpr domain::CardCode blue_eyes_code{89631139};
	constexpr domain::CardCode filler_code{97001010};

	driver::LocalTestSelector selector
	{
			select_up_to_max,
			nullptr,
			nullptr,
			choose_all_triggers,
	};

	flow::Duel duel{selector};
	auto& game = duel.game();

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	const auto* dragon_shrine_script = duel.find_script(dragon_shrine_code);
	REQUIRE(dragon_shrine_script != nullptr);
	const auto* blue_eyes_script = duel.find_script(blue_eyes_code);
	REQUIRE(blue_eyes_script != nullptr);

	game.register_definition(dragon_shrine_script->identity);
	game.register_definition(blue_eyes_script->identity);
	const auto filler_definition = make_monster(filler_code, 1000, 1000, 4, domain::Race::DRAGON);
	game.register_definition(filler_definition);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto dragon_shrine = game.create_instance(dragon_shrine_code, domain::Player::FIRST);
	REQUIRE(dragon_shrine.valid());
	game.place(*game.find_instance_mutable(dragon_shrine), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto blue_eyes = game.create_instance(blue_eyes_code, domain::Player::FIRST);
	REQUIRE(blue_eyes.valid());
	game.place(*game.find_instance_mutable(blue_eyes), domain::DeckPlacement{.index = 0});

	const auto filler = game.create_instance(filler_code, domain::Player::FIRST);
	REQUIRE(filler.valid());
	game.place(*game.find_instance_mutable(filler), domain::DeckPlacement{.index = 1});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(dragon_shrine));
	settle(duel);

	CHECK(game.zone_of(blue_eyes) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(filler) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(dragon_shrine) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel resolves Cards of Consonance and triggers The White Stone of Legend")
{
	constexpr domain::CardCode consonance_code{39701395};
	constexpr domain::CardCode white_stone_code{79814787};
	constexpr domain::CardCode blue_eyes_code{89631139};
	constexpr domain::CardCode filler_code{97001011};

	driver::LocalTestSelector selector
	{
			select_up_to_max,
			nullptr,
			nullptr,
			choose_all_triggers,
	};

	flow::Duel duel{selector};
	auto& game = duel.game();

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	const auto* consonance_script = duel.find_script(consonance_code);
	REQUIRE(consonance_script != nullptr);
	const auto* white_stone_script = duel.find_script(white_stone_code);
	REQUIRE(white_stone_script != nullptr);
	const auto* blue_eyes_script = duel.find_script(blue_eyes_code);
	REQUIRE(blue_eyes_script != nullptr);

	game.register_definition(consonance_script->identity);
	game.register_definition(white_stone_script->identity);
	game.register_definition(blue_eyes_script->identity);
	const auto filler_definition = make_monster(filler_code, 1000, 1000, 4);
	game.register_definition(filler_definition);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto consonance = game.create_instance(consonance_code, domain::Player::FIRST);
	REQUIRE(consonance.valid());
	game.place(*game.find_instance_mutable(consonance), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto white_stone = game.create_instance(white_stone_code, domain::Player::FIRST);
	REQUIRE(white_stone.valid());
	game.place(*game.find_instance_mutable(white_stone), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto first_draw = game.create_instance(filler_code, domain::Player::FIRST);
	REQUIRE(first_draw.valid());
	game.place(*game.find_instance_mutable(first_draw), domain::DeckPlacement{.index = 0});

	const auto second_draw = game.create_instance(filler_code, domain::Player::FIRST);
	REQUIRE(second_draw.valid());
	game.place(*game.find_instance_mutable(second_draw), domain::DeckPlacement{.index = 1});

	const auto blue_eyes = game.create_instance(blue_eyes_code, domain::Player::FIRST);
	REQUIRE(blue_eyes.valid());
	game.place(*game.find_instance_mutable(blue_eyes), domain::DeckPlacement{.index = 2});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(consonance));
	settle(duel);

	CHECK(game.zone_of(white_stone) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(first_draw) == domain::Zone::HAND);
	CHECK(game.zone_of(second_draw) == domain::Zone::HAND);
	CHECK(game.zone_of(blue_eyes) == domain::Zone::HAND);
	CHECK(game.zone_of(consonance) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel resolves The Melody of Awakening Dragon")
{
	constexpr domain::CardCode melody_code{48800175};
	constexpr domain::CardCode blue_eyes_code{89631139};
	constexpr domain::CardCode filler_code{97001012};
	constexpr domain::CardCode non_qualifier_code{97001013};

	driver::LocalTestSelector selector
	{
			select_up_to_max,
			nullptr,
			nullptr,
			choose_all_triggers,
	};

	flow::Duel duel{selector};
	auto& game = duel.game();

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	const auto* melody_script = duel.find_script(melody_code);
	REQUIRE(melody_script != nullptr);
	const auto* blue_eyes_script = duel.find_script(blue_eyes_code);
	REQUIRE(blue_eyes_script != nullptr);

	game.register_definition(melody_script->identity);
	game.register_definition(blue_eyes_script->identity);
	const auto filler_definition = make_monster(filler_code, 1000, 1000, 4);
	game.register_definition(filler_definition);
	const auto non_qualifier_definition = make_monster(non_qualifier_code, 3000, 3000, 8, domain::Race::DRAGON);
	game.register_definition(non_qualifier_definition);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto melody = game.create_instance(melody_code, domain::Player::FIRST);
	REQUIRE(melody.valid());
	game.place(*game.find_instance_mutable(melody), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto filler = game.create_instance(filler_code, domain::Player::FIRST);
	REQUIRE(filler.valid());
	game.place(*game.find_instance_mutable(filler), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto first_blue_eyes = game.create_instance(blue_eyes_code, domain::Player::FIRST);
	REQUIRE(first_blue_eyes.valid());
	game.place(*game.find_instance_mutable(first_blue_eyes), domain::DeckPlacement{.index = 0});

	const auto second_blue_eyes = game.create_instance(blue_eyes_code, domain::Player::FIRST);
	REQUIRE(second_blue_eyes.valid());
	game.place(*game.find_instance_mutable(second_blue_eyes), domain::DeckPlacement{.index = 1});

	const auto non_qualifier = game.create_instance(non_qualifier_code, domain::Player::FIRST);
	REQUIRE(non_qualifier.valid());
	game.place(*game.find_instance_mutable(non_qualifier), domain::DeckPlacement{.index = 2});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(melody));
	settle(duel);

	CHECK(game.zone_of(first_blue_eyes) == domain::Zone::HAND);
	CHECK(game.zone_of(second_blue_eyes) == domain::Zone::HAND);
	CHECK(game.zone_of(non_qualifier) == domain::Zone::DECK);
	CHECK(game.zone_of(filler) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(melody) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel resolves Twin Twisters")
{
	constexpr domain::CardCode twin_twisters_code{43898403};
	constexpr domain::CardCode filler_code{97001014};
	constexpr domain::CardCode trap_code{97001015};

	driver::LocalTestSelector selector
	{
			select_up_to_max,
			nullptr,
			nullptr,
			choose_all_triggers,
	};

	flow::Duel duel{selector};
	auto& game = duel.game();

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	const auto* twin_twisters_script = duel.find_script(twin_twisters_code);
	REQUIRE(twin_twisters_script != nullptr);
	game.register_definition(twin_twisters_script->identity);

	const auto filler_definition = make_monster(filler_code, 1000, 1000, 4);
	game.register_definition(filler_definition);
	const auto trap_definition = make_trap(trap_code, {domain::CardSubtype::NORMAL});
	game.register_definition(trap_definition);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto twin_twisters = game.create_instance(twin_twisters_code, domain::Player::FIRST);
	REQUIRE(twin_twisters.valid());
	game.place(*game.find_instance_mutable(twin_twisters), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto filler = game.create_instance(filler_code, domain::Player::FIRST);
	REQUIRE(filler.valid());
	game.place(*game.find_instance_mutable(filler), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto first_trap = game.create_instance(trap_code, domain::Player::SECOND);
	REQUIRE(first_trap.valid());
	game.place(*game.find_instance_mutable(first_trap), domain::SpellTrapPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto second_trap = game.create_instance(trap_code, domain::Player::SECOND);
	REQUIRE(second_trap.valid());
	game.place(*game.find_instance_mutable(second_trap), domain::SpellTrapPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_DOWN_DEFENSE});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(twin_twisters));
	settle(duel);

	CHECK(game.zone_of(first_trap) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(second_trap) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(filler) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(twin_twisters) == domain::Zone::GRAVEYARD);
}

TEST_CASE("flow::Duel resolves Sage with Eyes of Blue normal-summon search")
{
	constexpr domain::CardCode sage_code{8240199};
	constexpr domain::CardCode white_stone_code{79814787};

	driver::LocalTestSelector selector
	{
			select_up_to_max,
			nullptr,
			nullptr,
			choose_all_triggers,
	};

	flow::Duel duel{selector};
	auto& game = duel.game();

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	const auto* sage_script = duel.find_script(sage_code);
	REQUIRE(sage_script != nullptr);
	const auto* white_stone_script = duel.find_script(white_stone_code);
	REQUIRE(white_stone_script != nullptr);

	game.register_definition(sage_script->identity);
	game.register_definition(white_stone_script->identity);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto sage = game.create_instance(sage_code, domain::Player::FIRST);
	REQUIRE(sage.valid());
	game.place(*game.find_instance_mutable(sage), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto white_stone = game.create_instance(white_stone_code, domain::Player::FIRST);
	REQUIRE(white_stone.valid());
	game.place(*game.find_instance_mutable(white_stone), domain::DeckPlacement{.index = 0});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.normal_summon(sage, domain::FieldColumn::C1));
	resolve_summon(duel);
	settle(duel);

	CHECK(game.zone_of(sage) == domain::Zone::MONSTER);
	CHECK(game.zone_of(white_stone) == domain::Zone::HAND);
}

TEST_CASE("flow::Duel resolves Sage with Eyes of Blue discard effect")
{
	constexpr domain::CardCode sage_code{8240199};
	constexpr domain::CardCode blue_eyes_code{89631139};
	constexpr domain::CardCode effect_monster_code{97001016};

	driver::LocalTestSelector selector
	{
			select_up_to_max,
			nullptr,
			nullptr,
			choose_all_triggers,
	};

	flow::Duel duel{selector};
	auto& game = duel.game();

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	const auto* sage_script = duel.find_script(sage_code);
	REQUIRE(sage_script != nullptr);
	const auto* blue_eyes_script = duel.find_script(blue_eyes_code);
	REQUIRE(blue_eyes_script != nullptr);

	game.register_definition(sage_script->identity);
	game.register_definition(blue_eyes_script->identity);

	auto effect_monster_definition = make_monster(effect_monster_code, 1000, 1000, 4);
	effect_monster_definition.subtypes = {domain::CardSubtype::EFFECT};
	game.register_definition(effect_monster_definition);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto sage = game.create_instance(sage_code, domain::Player::FIRST);
	REQUIRE(sage.valid());
	game.place(*game.find_instance_mutable(sage), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto effect_monster = game.create_instance(effect_monster_code, domain::Player::FIRST);
	REQUIRE(effect_monster.valid());
	game.place(*game.find_instance_mutable(effect_monster), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto blue_eyes = game.create_instance(blue_eyes_code, domain::Player::FIRST);
	REQUIRE(blue_eyes.valid());
	game.place(*game.find_instance_mutable(blue_eyes), domain::DeckPlacement{.index = 0});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_effect(sage, 1));
	settle(duel);

	CHECK(game.zone_of(sage) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(effect_monster) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(blue_eyes) == domain::Zone::MONSTER);
	CHECK(game.find_instance(blue_eyes)->summon_record().method == domain::SummonMethod::SPECIAL);
}

TEST_CASE("flow::Duel excludes the activating card from pick_cost")
{
	constexpr domain::CardCode spell_code{97000164};
	constexpr domain::CardCode filler_code{97000165};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_TEST_CARDS_DIR}));

	const auto* spell_script = duel.find_script(spell_code);
	REQUIRE(spell_script != nullptr);
	game.register_definition(spell_script->identity);

	const auto filler_definition = make_monster(filler_code, 1000, 1000, 4);
	game.register_definition(filler_definition);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto first_spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(first_spell.valid());
	game.place(*game.find_instance_mutable(first_spell), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto draw_card = game.create_instance(filler_code, domain::Player::FIRST);
	REQUIRE(draw_card.valid());
	game.place(*game.find_instance_mutable(draw_card), domain::DeckPlacement{.index = 0});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	// 只有自身可作代价: 必须失败, 不能把自己丢弃
	CHECK_FALSE(duel.activate_card(first_spell));
	CHECK(game.zone_of(first_spell) == domain::Zone::HAND);

	// 第二张同名卡可以作为代价
	const auto second_spell = game.create_instance(spell_code, domain::Player::FIRST);
	REQUIRE(second_spell.valid());
	game.place(*game.find_instance_mutable(second_spell), domain::HandPlacement{.index = 1, .position = domain::Position::FACE_DOWN_DEFENSE});

	CHECK(duel.activate_card(second_spell));
	settle(duel);

	CHECK(game.zone_of(first_spell) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(second_spell) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(draw_card) == domain::Zone::HAND);
}

TEST_CASE("flow::Duel resolves real-card SEGOC with two The White Stone of Legend")
{
	constexpr domain::CardCode dark_hole_code{53129443};
	constexpr domain::CardCode white_stone_code{79814787};
	constexpr domain::CardCode blue_eyes_code{89631139};

	flow::Duel duel{g_selector};
	auto& game = duel.game();

	REQUIRE(duel.load_scripts(std::filesystem::path{CG_CARDS_DIR}));

	const auto* dark_hole_script = duel.find_script(dark_hole_code);
	REQUIRE(dark_hole_script != nullptr);
	const auto* white_stone_script = duel.find_script(white_stone_code);
	REQUIRE(white_stone_script != nullptr);
	const auto* blue_eyes_script = duel.find_script(blue_eyes_code);
	REQUIRE(blue_eyes_script != nullptr);

	game.register_definition(dark_hole_script->identity);
	game.register_definition(white_stone_script->identity);
	game.register_definition(blue_eyes_script->identity);

	game.set_life_points(domain::Player::FIRST, 8000);
	game.set_life_points(domain::Player::SECOND, 8000);

	const auto dark_hole = game.create_instance(dark_hole_code, domain::Player::FIRST);
	REQUIRE(dark_hole.valid());
	game.place(*game.find_instance_mutable(dark_hole), domain::HandPlacement{.index = 0, .position = domain::Position::FACE_DOWN_DEFENSE});

	const auto first_white_stone = game.create_instance(white_stone_code, domain::Player::FIRST);
	REQUIRE(first_white_stone.valid());
	game.place(*game.find_instance_mutable(first_white_stone), domain::MonsterPlacement{.column = domain::FieldColumn::C1, .position = domain::Position::FACE_UP_ATTACK});

	const auto second_white_stone = game.create_instance(white_stone_code, domain::Player::FIRST);
	REQUIRE(second_white_stone.valid());
	game.place(*game.find_instance_mutable(second_white_stone), domain::MonsterPlacement{.column = domain::FieldColumn::C2, .position = domain::Position::FACE_UP_ATTACK});

	const auto first_blue_eyes = game.create_instance(blue_eyes_code, domain::Player::FIRST);
	REQUIRE(first_blue_eyes.valid());
	game.place(*game.find_instance_mutable(first_blue_eyes), domain::DeckPlacement{.index = 0});

	const auto second_blue_eyes = game.create_instance(blue_eyes_code, domain::Player::FIRST);
	REQUIRE(second_blue_eyes.valid());
	game.place(*game.find_instance_mutable(second_blue_eyes), domain::DeckPlacement{.index = 1});

	CHECK(advance_phase(duel)); // STANDBY
	CHECK(advance_phase(duel)); // MAIN_1
	REQUIRE(duel.turn().phase() == domain::Phase::MAIN_1);

	CHECK(duel.activate_card(dark_hole));
	CHECK(duel.pass_priority());

	REQUIRE(duel.chain().size() == 2);
	CHECK(duel.chain()[0].controller() == domain::Player::FIRST);
	CHECK(duel.chain()[1].controller() == domain::Player::FIRST);

	settle(duel);

	CHECK(game.zone_of(first_white_stone) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(second_white_stone) == domain::Zone::GRAVEYARD);
	CHECK(game.zone_of(first_blue_eyes) == domain::Zone::HAND);
	CHECK(game.zone_of(second_blue_eyes) == domain::Zone::HAND);
}

#pragma once

#include <array>
#include <span>

#include <core/domain/numeric.hpp>
#include <core/domain/zone.hpp>
#include <core/domain/player.hpp>
#include <core/domain/random.hpp>
#include <core/domain/turn.hpp>

#include <core/engine/collection.hpp>

namespace cg::engine
{
	class Duel;

	class PlayerField
	{
	public:
		// 不包含额外怪兽区
		using monster_field_type = std::array<CardOptional, domain::field_monster_main_count>;
		// 魔陷区+场地魔法区
		using spell_trap_field_type = std::array<CardOptional, domain::field_spell_trap_count>;

		// 玩家生命值
		domain::life_point_type life_point;
		// 玩家起始手牌数量
		domain::zone_sequence_type start_hand;
		// 玩家每回合抽牌数量
		domain::zone_sequence_type draw_count;
		// 卡组
		Sequence deck;
		// 额外卡组
		Sequence extra_deck;
		// 手牌
		Sequence hand;
		// 墓地
		Sequence graveyard;
		// 除外区
		Sequence removed;
		// 怪兽区
		monster_field_type monster;
		// 魔陷区
		spell_trap_field_type spell_trap;
	};

	class Field
	{
	public:
		class PlaygroundHandler;
		class RandomHandler;
		class TurnHandler;

	private:
		class Playground
		{
		public:
			using player_fields_type = std::array<PlayerField, domain::player_count>;
			using shared_extra_monster_field_type = std::array<CardOptional, domain::field_monster_extra_count>;

			// 双方玩家场地
			player_fields_type player_fields;
			// 额外怪兽区
			shared_extra_monster_field_type shared_extra_monster_field;
		};

		class Random
		{
		public:
			std::vector<domain::DiceValue> dice_results;
			std::vector<domain::CoinSide> coin_results;
		};

		class Turn
		{
		public:
			// 当前回合数
			domain::TurnId id;
			// 当前阶段
			domain::Phase phase;
			// 当前玩家
			domain::Player player;
			// 当前回合是否允许攻击
			bool can_battle;
			// 当前回合是否可通常召唤
			bool can_normal_summon;
			// 当前回合是否可以特殊召唤
			bool can_special_summon;
		};

		std::reference_wrapper<Duel> duel_;

		Playground playground_;
		Random random_;
		Turn turn_;

		[[nodiscard]] auto duel() const noexcept -> Duel&;

	public:
		// ==================== 场地 ====================

		class PlaygroundHandler
		{
			friend Field;

			std::reference_wrapper<Field> field_;

			explicit PlaygroundHandler(Field& field) noexcept;

			[[nodiscard]] auto playground_data() noexcept -> Playground&;
			[[nodiscard]] auto playground_data() const noexcept -> const Playground&;

			[[nodiscard]] auto player_field(domain::Player player) noexcept -> PlayerField&;
			[[nodiscard]] auto player_field(domain::Player player) const noexcept -> const PlayerField&;

		public:
			auto set_life_point(domain::Player player, domain::life_point_type life_point) noexcept -> void;
			auto set_start_hand(domain::Player player, domain::zone_sequence_type start_hand) noexcept -> void;
			auto set_draw_count(domain::Player player, domain::zone_sequence_type draw_count) noexcept -> void;

			[[nodiscard]] auto life_point(domain::Player player) const noexcept -> domain::life_point_type;
			[[nodiscard]] auto start_hand(domain::Player player) const noexcept -> domain::zone_sequence_type;
			[[nodiscard]] auto draw_count(domain::Player player) const noexcept -> domain::zone_sequence_type;

			// 改变生命值
			auto update_life_point(domain::Player player, domain::life_point_type delta) noexcept -> void;

			// 起始出牌
			auto start_draw(domain::Player player) noexcept -> void;
			// 从卡组抽牌
			auto draw(domain::Player player, domain::zone_sequence_type count) noexcept -> void;
			// 切洗卡组
			auto shuffle_deck(domain::Player player) noexcept -> void;
			// 反转卡组
			auto reverse_deck(domain::Player player) noexcept -> void;
			// 切洗额外卡组
			auto shuffle_extra_deck(domain::Player player) noexcept -> void;
			// 切洗手牌
			auto shuffle_hand(domain::Player player) noexcept -> void;

			// 检查指定场地区域是否被占用
			[[nodiscard]] auto occupied(domain::Player player, domain::Zone zone, domain::FieldZoneSequence field_zone) const noexcept -> bool;
			// 获取指定场地区域的卡牌
			[[nodiscard]] auto select(domain::Player player, domain::Zone zone, domain::FieldZoneSequence field_zone) const noexcept -> CardOptional;
			// 获取指定非场地区域的卡牌
			[[nodiscard]] auto select(domain::Player player, domain::Zone zone, domain::zone_sequence_type zone_index) const noexcept -> CardOptional;
			// 获取指定区域的卡牌数量
			[[nodiscard]] auto count(domain::Player player, domain::Zone zone) const noexcept -> domain::zone_sequence_type;
			// 获取指定区域的卡牌
			[[nodiscard]] auto select(domain::Player player, domain::Zone zone) const noexcept -> View;
			// 获取指定场地区域可用区域
			[[nodiscard]] auto free_area(domain::Player player, domain::Zone zone) const noexcept -> std::vector<domain::FieldZoneSequence>;
		};

		// ==================== 随机 ====================

		class RandomHandler
		{
			friend Field;

			std::reference_wrapper<Field> field_;

			explicit RandomHandler(Field& field) noexcept;

			[[nodiscard]] auto random_data() noexcept -> Random&;
			[[nodiscard]] auto random_data() const noexcept -> const Random&;

		public:
			auto toss_dice(domain::Player player, std::size_t count) noexcept -> void;
			auto toss_coin(domain::Player player, std::size_t count) noexcept -> void;

			[[nodiscard]] auto dice_results(domain::Player player) const noexcept -> std::span<const domain::DiceValue>;
			[[nodiscard]] auto coin_results(domain::Player player) const noexcept -> std::span<const domain::CoinSide>;
		};

		// ==================== 回合/阶段 ====================

		class TurnHandler
		{
			friend Field;

			std::reference_wrapper<Field> field_;

			explicit TurnHandler(Field& field) noexcept;

			[[nodiscard]] auto turn_data() noexcept -> Turn&;
			[[nodiscard]] auto turn_data() const noexcept -> const Turn&;

		public:
			auto set_turn_id(domain::TurnId id) noexcept -> void;
			auto set_phase(domain::Phase phase) noexcept -> void;
			auto set_turn_phase(domain::TurnPhase turn_phase) noexcept -> void;
			auto set_battle_step(domain::BattleStep battle_step) noexcept -> void;
			auto set_damage_step(domain::DamageStep damage_step) noexcept -> void;
			auto set_player(domain::Player player) noexcept -> void;
			auto set_can_battle(bool can_battle) noexcept -> void;
			auto set_can_normal_summon(bool can_normal_summon) noexcept -> void;
			auto set_can_special_summon(bool can_special_summon) noexcept -> void;

			[[nodiscard]] auto turn_id() const noexcept -> domain::TurnId;
			[[nodiscard]] auto phase() const noexcept -> domain::Phase;
			[[nodiscard]] auto turn_phase() const noexcept -> domain::TurnPhase;
			[[nodiscard]] auto battle_step() const noexcept -> domain::BattleStep;
			[[nodiscard]] auto damage_step() const noexcept -> domain::DamageStep;
			[[nodiscard]] auto can_battle() const noexcept -> bool;
			[[nodiscard]] auto can_normal_summon() const noexcept -> bool;
			[[nodiscard]] auto can_special_summon() const noexcept -> bool;

			[[nodiscard]] auto is_turn_id(domain::TurnId expected_id) const noexcept -> bool;
			[[nodiscard]] auto is_phase(domain::Phase expected_phase) const noexcept -> bool;
			[[nodiscard]] auto is_turn_phase(domain::TurnPhase expected_turn_phase) const noexcept -> bool;
			[[nodiscard]] auto is_battle_step(domain::BattleStep expected_battle_step) const noexcept -> bool;
			[[nodiscard]] auto is_damage_step(domain::DamageStep expected_damage_step) const noexcept -> bool;

			// 开始下一回合
			auto new_turn(domain::Player player) noexcept -> void;
			// 前进到指定阶段(可用于跳过某个阶段)
			auto advance_phase(domain::TurnPhase turn_phase) noexcept -> void;
		};

		// ==================== | ====================

		explicit Field(Duel& duel) noexcept;

		[[nodiscard]] auto playground() noexcept -> PlaygroundHandler;
		[[nodiscard]] auto random() noexcept -> RandomHandler;
		[[nodiscard]] auto turn() noexcept -> TurnHandler;
	};
}

#pragma once

#include <array>
#include <span>

#include <core/domain/numeric.hpp>
#include <core/domain/zone.hpp>
#include <core/domain/player.hpp>
#include <core/domain/random.hpp>

#include <core/engine/collection.hpp>

namespace cg::engine
{
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

		// todo: 放到Duel中是不是更好?
		class Random
		{
		public:
			std::vector<domain::DiceValue> dice_results;
			std::vector<domain::CoinSide> coin_results;
		};

		Playground playground_;
		Random random_;

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

		// ==================== | ====================

		[[nodiscard]] auto playground() noexcept -> PlaygroundHandler;
		[[nodiscard]] auto random() noexcept -> RandomHandler;
	};
}

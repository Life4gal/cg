#pragma once

#include <array>

#include <core/utility/random.hpp>

#include <core/domain/numeric.hpp>
#include <core/domain/zone.hpp>
#include <core/domain/player.hpp>

#include <core/engine/collection.hpp>

namespace cg::engine
{
	class PlayerField
	{
	public:
		// 不包含额外怪兽区
		using monster_field_type = std::array<CardOptional, domain::Zone::monster_main_count>;
		// 魔陷区+场地魔法区
		using spell_trap_field_type = std::array<CardOptional, domain::Zone::spell_trap_count>;

		using size_type = domain::Zone::size_type;

		// 玩家生命值
		domain::life_point_type life_point;
		// 玩家起始手牌数量
		size_type start_hand;
		// 玩家每回合抽牌数量
		size_type draw_count;
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

	class Playground
	{
	public:
		using player_fields_type = std::array<PlayerField, domain::player_count>;
		using shared_extra_monster_field_type = std::array<CardOptional, domain::Zone::monster_extra_count>;

		using size_type = PlayerField::size_type;

	private:
		// 双方玩家场地
		player_fields_type player_fields_;
		// 额外怪兽区
		shared_extra_monster_field_type shared_extra_monster_field_;

		[[nodiscard]] auto player_field(domain::Player player) noexcept -> PlayerField&;
		[[nodiscard]] auto player_field(domain::Player player) const noexcept -> const PlayerField&;

	public:
		Playground() noexcept;

		// 设置玩家生命值
		auto set_life_point(domain::Player player, domain::life_point_type life_point) noexcept -> void;
		// 设置玩家起手手牌数量
		auto set_start_hand(domain::Player player, size_type start_hand) noexcept -> void;
		// 设置玩家每回合抽牌数量
		auto set_draw_count(domain::Player player, size_type draw_count) noexcept -> void;

		// 获取玩家生命值
		[[nodiscard]] auto life_point(domain::Player player) const noexcept -> domain::life_point_type;
		// 获取玩家起手手牌数量
		[[nodiscard]] auto start_hand(domain::Player player) const noexcept -> size_type;
		// 获取玩家每回合抽牌数量
		[[nodiscard]] auto draw_count(domain::Player player) const noexcept -> size_type;

		// 改变生命值
		auto update_life_point(domain::Player player, domain::life_point_type delta) noexcept -> void;

		// 起始抽牌
		auto start_draw(domain::Player player) noexcept -> void;
		// 从卡组抽牌
		auto draw(domain::Player player, size_type count) noexcept -> void;
		// 切洗卡组
		auto shuffle_deck(domain::Player player, utility::Random& random) noexcept -> void;
		// 反转卡组
		auto reverse_deck(domain::Player player) noexcept -> void;
		// 切洗额外卡组
		auto shuffle_extra_deck(domain::Player player, utility::Random& random) noexcept -> void;
		// 切洗手牌
		auto shuffle_hand(domain::Player player, utility::Random& random) noexcept -> void;

		// 检查指定区域是否被占用
		[[nodiscard]] auto occupied(domain::Player player, domain::Zone zone) const noexcept -> bool;
		// 获取指定区域的卡牌
		[[nodiscard]] auto select(domain::Player player, domain::Zone zone) const noexcept -> CardOptional;
		// 获取指定区域的卡牌数量
		[[nodiscard]] auto count(domain::Player player, domain::Zone zone) const noexcept -> size_type;
		// 获取指定场地区域可用区域
		[[nodiscard]] auto free_area(domain::Player player, domain::Zone zone) const noexcept -> std::vector<domain::Zone::size_type>;

		// 将卡牌从其所在区域移除
		auto remove_card(CardReference card) noexcept -> bool;
		// 将卡牌移动到指定位置
		auto move_card(CardReference card, domain::Player player, domain::Zone zone) noexcept -> bool;
	};
}

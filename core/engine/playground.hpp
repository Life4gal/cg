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

		// 双方玩家场地
		player_fields_type player_fields;
		// 额外怪兽区
		shared_extra_monster_field_type shared_extra_monster_field;

		Playground() noexcept;

		// 获取指定玩家的PlayerField
		[[nodiscard]] auto field_of(domain::Player player) noexcept -> PlayerField&;
		[[nodiscard]] auto field_of(domain::Player player) const noexcept -> const PlayerField&;

		// ---------- 基本信息 ----------

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

		// ---------- 生命值 ----------

		// 可以承受指定生命值变化
		[[nodiscard]] auto affordable(domain::Player player, domain::life_point_type delta) const noexcept -> bool;
		// 承受指定生命值变化
		auto afford(domain::Player player, domain::life_point_type delta) noexcept -> void;

		// ---------- 抽牌 ----------

		// 起始抽牌 -- 抽start_hand数量的卡牌
		auto start_draw(domain::Player player) noexcept -> std::span<CardReference>;
		// 抽取指定数量的卡牌 -- 返回抽取的卡牌
		[[nodiscard]] auto draw(domain::Player player, size_type count) noexcept -> std::span<CardReference>;

		// ---------- 卡组 ----------

		// 往指定区域追加卡牌 -- 仅设置卡牌的控制者和其所在区域位置,一般用于初始化卡组/额外卡组
		auto add(domain::Player player, Card& card, domain::AutoZone zone) noexcept -> void;

		// 洗指定区域的牌
		auto shuffle(domain::Player player, domain::AutoZone zone, utility::Random& random) noexcept -> void;
		// 反转指定区域的牌
		auto reverse(domain::Player player, domain::AutoZone zone) noexcept -> void;

		// ---------- 区域 ----------

		// 获取指定区域的牌
		[[nodiscard]] auto select(domain::Player player, domain::Zone zone) const noexcept -> CardOptional;
		// 检查指定区域是否被占用
		[[nodiscard]] auto occupied(domain::Player player, domain::Zone zone) const noexcept -> bool;

		// 获取指定区域卡牌数量 -- zone只决定类型,一般不用这个接口
		[[nodiscard]] auto count(domain::Player player, domain::Zone zone) const noexcept -> size_type;
		// 获取指定自动区域卡牌数量
		[[nodiscard]] auto count(domain::Player player, domain::AutoZone zone) const noexcept -> size_type;
		// 获取指定场地区域卡牌数量
		[[nodiscard]] auto count(domain::Player player, domain::FieldZone zone) const noexcept -> size_type;

		// 获取指定区域所有可用位置 -- zone只决定类型,一般不用这个接口
		[[nodiscard]] auto free(domain::Player player, domain::Zone zone) const noexcept -> std::vector<size_type>;
		// 获取指定自动区域所有可用位置 -- 没有这个概念
		//
		// 获取指定场地区域所有可用位置
		[[nodiscard]] auto free(domain::Player player, domain::FieldZone zone) const noexcept -> std::vector<size_type>;

		// ---------- 移动卡牌 ----------

		// 将卡牌从其所在位置移除 -- 仅从其所在区域移除,不做任何额外处理
		[[nodiscard]] auto remove(CardReference card) noexcept -> bool;
	};
}

#pragma once

#include <array>
#include <vector>

#include <domain/zone.hpp>
#include <domain/card.hpp>
#include <domain/player.hpp>

#include <engine/state/id.hpp>

namespace cg::engine
{
	class GameState;

	// 玩家区域
	class PlayerZone
	{
	public:
		using monster_zones_type = std::array<CardInstanceId, domain::MonsterLocation::count>;
		using spell_trap_zones_type = std::array<CardInstanceId, domain::SpellTrapLocation::count>;
		using bag_zones_type = std::vector<CardInstanceId>;

		// 生命值
		std::underlying_type_t<domain::AttackDefense> life_point;
		// 主要怪兽区
		monster_zones_type monster_zones;
		// 魔陷区+场地魔法区
		spell_trap_zones_type spell_trap_zones;
		// 卡组
		bag_zones_type deck_zones;
		// 额外卡组
		bag_zones_type extra_deck_zones;
		// 手牌
		bag_zones_type hand_zones;
		// 墓地
		bag_zones_type graveyard_zones;
		// 除外区
		bag_zones_type banished_zones;
	};

	// 游戏场地
	class Playground
	{
	public:
		using player_zones_type = std::array<PlayerZone, domain::player_count>;
		using extra_monster_zones_type = std::array<CardInstanceId, domain::ExtraMonsterLocation::count>;

		// 玩家独占区域
		player_zones_type player_zones;
		// 玩家共享区域 - 额外怪兽区
		extra_monster_zones_type extra_monster_zones;
	};

	// ==================================================================================
	// 场地/区域映射
	// ==================================================================================

	// 指定怪兽位置对应的对方怪兽位置(同一绝对列)
	[[nodiscard]] auto opponent_location(domain::MonsterLocation location) noexcept -> domain::MonsterLocation;

	// 指定魔陷位置对应的对方魔陷位置(同一绝对列)
	[[nodiscard]] auto opponent_location(domain::SpellTrapLocation location) noexcept -> domain::SpellTrapLocation;

	// ==================================================================================
	// 玩家所属区域
	// ==================================================================================

	// 获取玩家所属区域
	[[nodiscard]] auto player_zone_of(const GameState& state, domain::Player player) noexcept -> const PlayerZone&;

	// ==================================================================================
	// 指定卡牌的区域/位置
	// ==================================================================================

	// 获取卡牌实例所在区域
	[[nodiscard]] auto zone_of(const GameState& state, CardInstanceId card) noexcept -> domain::Zone;

	// 获取卡牌实例所在位置
	[[nodiscard]] auto location_of(const GameState& state, CardInstanceId card) noexcept -> domain::Location;

	// 获取卡牌实例表示形式
	// position_of需要访问实例,定义在card.hpp中

	// ==================================================================================
	// 指定区域的卡牌实例
	// ==================================================================================

	[[nodiscard]] auto is_deck_zones_empty(const GameState& state, domain::Player player) noexcept -> bool;
	[[nodiscard]] auto is_extra_deck_zones_empty(const GameState& state, domain::Player player) noexcept -> bool;
	[[nodiscard]] auto is_hand_zones_empty(const GameState& state, domain::Player player) noexcept -> bool;
	[[nodiscard]] auto is_graveyard_zones_empty(const GameState& state, domain::Player player) noexcept -> bool;
	[[nodiscard]] auto is_banished_zones_empty(const GameState& state, domain::Player player) noexcept -> bool;
	[[nodiscard]] auto is_monster_zones_empty(const GameState& state, domain::Player player) noexcept -> bool;
	[[nodiscard]] auto is_extra_monster_zones_empty(const GameState& state, domain::Player player) noexcept -> bool;
	[[nodiscard]] auto is_spell_trap_zones_empty(const GameState& state, domain::Player player) noexcept -> bool;

	// 检查某个区域是否不存在卡牌实例
	[[nodiscard]] auto is_zones_empty(const GameState& state, domain::Player player, domain::Zone zone) noexcept -> bool;

	// 检查某个区域是否不存在卡牌实例
	[[nodiscard]] auto is_zones_empty(const GameState& state, domain::Zone zone) noexcept -> bool;

	[[nodiscard]] auto count_deck_zones_card(const GameState& state, domain::Player player) noexcept -> domain::DeckLocation::size_type;
	[[nodiscard]] auto count_extra_deck_zones_card(const GameState& state, domain::Player player) noexcept -> domain::DeckLocation::size_type;
	[[nodiscard]] auto count_hand_zones_card(const GameState& state, domain::Player player) noexcept -> domain::DeckLocation::size_type;
	[[nodiscard]] auto count_graveyard_zones_card(const GameState& state, domain::Player player) noexcept -> domain::DeckLocation::size_type;
	[[nodiscard]] auto count_banished_zones_card(const GameState& state, domain::Player player) noexcept -> domain::DeckLocation::size_type;
	[[nodiscard]] auto count_monster_zones_card(const GameState& state, domain::Player player) noexcept -> domain::DeckLocation::size_type;
	[[nodiscard]] auto count_extra_monster_zones_card(const GameState& state, domain::Player player) noexcept -> domain::DeckLocation::size_type;
	[[nodiscard]] auto count_spell_trap_zones_card(const GameState& state, domain::Player player) noexcept -> domain::DeckLocation::size_type;

	// 获取某个区域所有卡牌实例的数量
	[[nodiscard]] auto count_zones_card(const GameState& state, domain::Player player, domain::Zone zone) noexcept -> std::size_t;

	// 获取某个区域所有卡牌实例的数量
	[[nodiscard]] auto count_zones_card(const GameState& state, domain::Zone zone) noexcept -> std::size_t;

	[[nodiscard]] auto collect_deck_zones_card(const GameState& state, domain::Player player) noexcept -> std::vector<CardInstanceId>;
	[[nodiscard]] auto collect_extra_deck_zones_card(const GameState& state, domain::Player player) noexcept -> std::vector<CardInstanceId>;
	[[nodiscard]] auto collect_hand_zones_card(const GameState& state, domain::Player player) noexcept -> std::vector<CardInstanceId>;
	[[nodiscard]] auto collect_graveyard_zones_card(const GameState& state, domain::Player player) noexcept -> std::vector<CardInstanceId>;
	[[nodiscard]] auto collect_banished_zones_card(const GameState& state, domain::Player player) noexcept -> std::vector<CardInstanceId>;
	[[nodiscard]] auto collect_monster_zones_card(const GameState& state, domain::Player player) noexcept -> std::vector<CardInstanceId>;
	[[nodiscard]] auto collect_extra_monster_zones_card(const GameState& state, domain::Player player) noexcept -> std::vector<CardInstanceId>;
	[[nodiscard]] auto collect_spell_trap_zones_card(const GameState& state, domain::Player player) noexcept -> std::vector<CardInstanceId>;

	// 获取某个区域所有卡牌实例
	[[nodiscard]] auto collect_zones_card(const GameState& state, domain::Player player, domain::Zone zone) noexcept -> std::vector<CardInstanceId>;

	// 获取某个区域所有卡牌实例
	[[nodiscard]] auto collect_zones_card(const GameState& state, domain::Zone zone) noexcept -> std::vector<CardInstanceId>;

	// ==================================================================================
	// 指定位置的卡牌实例
	// ==================================================================================

	[[nodiscard]] auto is_location_empty(const GameState& state, domain::Player player, domain::MonsterLocation location) noexcept -> bool;
	[[nodiscard]] auto is_location_empty(const GameState& state, domain::Player player, domain::ExtraMonsterLocation location) noexcept -> bool;
	[[nodiscard]] auto is_location_empty(const GameState& state, domain::Player player, domain::SpellTrapLocation location) noexcept -> bool;

	// 检查某个位置是否为空(不存在卡牌实例) -- 仅对怪兽区/额外怪兽区/魔陷区有意义
	[[nodiscard]] auto is_location_empty(const GameState& state, domain::Player player, domain::Location location) noexcept -> bool;

	// 检查某个位置是否为空(不存在卡牌实例) -- 仅对怪兽区/额外怪兽区/魔陷区有意义
	[[nodiscard]] auto is_location_empty(const GameState& state, domain::Location location) noexcept -> bool;

	[[nodiscard]] auto select_location_card(const GameState& state, domain::Player player, domain::DeckLocation location) noexcept -> CardInstanceId;
	[[nodiscard]] auto select_location_card(const GameState& state, domain::Player player, domain::ExtraDeckLocation location) noexcept -> CardInstanceId;
	[[nodiscard]] auto select_location_card(const GameState& state, domain::Player player, domain::HandLocation location) noexcept -> CardInstanceId;
	[[nodiscard]] auto select_location_card(const GameState& state, domain::Player player, domain::GraveyardLocation location) noexcept -> CardInstanceId;
	[[nodiscard]] auto select_location_card(const GameState& state, domain::Player player, domain::BanishedLocation location) noexcept -> CardInstanceId;
	[[nodiscard]] auto select_location_card(const GameState& state, domain::Player player, domain::MonsterLocation location) noexcept -> CardInstanceId;
	[[nodiscard]] auto select_location_card(const GameState& state, domain::Player player, domain::ExtraMonsterLocation location) noexcept -> CardInstanceId;
	[[nodiscard]] auto select_location_card(const GameState& state, domain::Player player, domain::SpellTrapLocation location) noexcept -> CardInstanceId;

	// 获取指定位置的卡牌实例
	// 如果目标位置不存在卡牌实例则返回无效ID
	[[nodiscard]] auto select_location_card(const GameState& state, domain::Player player, domain::Location location) noexcept -> CardInstanceId;
}

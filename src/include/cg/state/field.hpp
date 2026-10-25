#pragma once

#include <random>
#include <vector>

#include <cg/utility/enum.hpp>

#include <cg/domain/field.hpp>
#include <cg/domain/card.hpp>
#include <cg/domain/player.hpp>

#include <cg/state/id.hpp>

namespace cg::state
{
	class Game;

	// 单个玩家场地
	class PlayerZone
	{
	public:
		using field_zones_type = std::array<CardInstanceId, domain::total_field_columns>;
		using bag_zones_type = std::vector<CardInstanceId>;

		// 生命值
		std::underlying_type_t<domain::AttackDefense> life_points;
		// 主要怪兽区
		field_zones_type monster_zones;
		// 魔陷区
		field_zones_type spell_trap_zones;
		// 场地魔法区
		CardInstanceId field_spell_zone;
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

	// 整个游戏场地
	class Field
	{
	public:
		using player_zones_type = std::array<PlayerZone, 2>;
		// 额外怪兽区使用两个格子就够了,但是使用5个格子可以极大地简化计算
		using field_zones_type = PlayerZone::field_zones_type;

		using size_type = domain::DeckLocation::size_type;

	private:
		// 玩家独占区域
		player_zones_type player_zones_;
		// 玩家共享区域 - 额外怪兽区
		field_zones_type extra_monster_zones_;

		// 获取玩家所属区域
		[[nodiscard]] auto player_zone_of(domain::Player player) const noexcept -> const PlayerZone&;
		[[nodiscard]] auto player_zone_of(domain::Player player) noexcept -> PlayerZone&;

	public:
		Field() noexcept;

		// ==================================================================================
		// 场地
		// ==================================================================================

		// 初始化:设置初始LP
		auto set_life_points(domain::Player player, std::underlying_type_t<domain::AttackDefense> value) noexcept -> void;

		// 获取当前LP
		[[nodiscard]] auto life_points(domain::Player player) const noexcept -> std::underlying_type_t<domain::AttackDefense>;

		// 改变LP
		auto change_life_points(domain::Player player, std::underlying_type_t<domain::AttackDefense> delta) noexcept -> void;

		// 洗牌(对局初始化/效果洗牌)
		// TODO: 自定义random
		auto shuffle_deck(domain::Player player, std::mt19937_64& random) noexcept -> void;

		// ==================================================================================
		// 指定卡牌的区域/位置
		// ==================================================================================

		// 获取卡牌实例所在区域
		[[nodiscard]] auto zone_of(CardInstanceId card) const noexcept -> domain::Zone;

		// 获取卡牌实例所在位置
		[[nodiscard]] auto location_of(CardInstanceId card) const noexcept -> domain::Location;

		// ==================================================================================
		// 指定区域的卡牌实例
		// ==================================================================================

		// 检查某个区域是否不存在卡牌实例
		[[nodiscard]] auto is_zone_empty(domain::Player player, domain::Zone zone) const noexcept -> bool;
		// 检查双方某个区域是否不存在卡牌实例
		[[nodiscard]] auto is_zone_empty(domain::Zone zone) const noexcept -> bool;

		// 检查某些区域是否不存在卡牌实例
		[[nodiscard]] auto is_zones_empty(domain::Player player, utility::EnumMask<domain::Zone> zones) const noexcept -> bool;
		// 检查双方某些区域是否不存在卡牌实例
		[[nodiscard]] auto is_zones_empty(utility::EnumMask<domain::Zone> zone) const noexcept -> bool;

		// 获取某个区域所有空位的数量 -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义,其他区域一律返回0
		[[nodiscard]] auto count_zone_empty_card(domain::Player player, domain::Zone zone) const noexcept -> size_type;

		// 获取某个区域所有空位的数量 -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义,其他区域一律返回0
		[[nodiscard]] auto count_zones_empty_card(domain::Player player, utility::EnumMask<domain::Zone> zones) const noexcept -> size_type;

		// 获取某个区域所有卡牌实例的数量
		// 如果是额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto count_zone_card(domain::Player player, domain::Zone zone) const noexcept -> size_type;
		// 获取某个区域所有卡牌实例的数量
		// 如果是额外怪兽区,则不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto count_zone_card(const Game& game, domain::Player player, domain::Zone zone) const noexcept -> size_type;

		// 获取某些区域所有卡牌实例的数量
		// 如果包含额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto count_zones_card(domain::Player player, utility::EnumMask<domain::Zone> zones) const noexcept -> size_type;
		// 获取双方某些区域所有卡牌实例的数量
		// 如果包含额外怪兽区,则不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto count_zones_card(const Game& game, domain::Player player, utility::EnumMask<domain::Zone> zones) const noexcept -> size_type;

		// 获取双方某些区域所有卡牌实例的数量
		[[nodiscard]] auto count_zones_card(utility::EnumMask<domain::Zone> zones) const noexcept -> size_type;

	private:
		auto collect_empty_zone(domain::Player player, domain::Zone zone, std::vector<domain::Location>& out) const noexcept -> void;

	public:
		// 获取指定区域的空位 -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义,其他区域一律返回空
		[[nodiscard]] auto collect_empty_zone(domain::Player player, domain::Zone zone) const noexcept -> std::vector<domain::Location>;

		// 获取指定区域的空位 -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义,其他区域一律返回空
		[[nodiscard]] auto collect_empty_zones(domain::Player player, utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<domain::Location>;

		// 获取怪兽区域的空位
		[[nodiscard]] auto collect_monster_empty_zone(domain::Player player) const noexcept -> std::vector<domain::MonsterLocation>;
		// 获取额外怪兽区域的空位
		[[nodiscard]] auto collect_extra_monster_empty_zone(domain::Player player) const noexcept -> std::vector<domain::ExtraMonsterLocation>;
		// 获取任意怪兽区域的空位
		[[nodiscard]] auto collect_any_monster_empty_zone(domain::Player player) const noexcept -> std::vector<domain::AnyMonsterLocation>;
		// 获取任意魔陷区域的空位
		[[nodiscard]] auto collect_spell_trap_empty_zone(domain::Player player) const noexcept -> std::vector<domain::SpellTrapLocation>;
		// 获取场地魔法区域的空位 -- 没有意义 :)

	private:
		auto collect_zone_card(domain::Player player, domain::Zone zone, std::vector<CardInstanceId>& out) const noexcept -> void;
		auto collect_zone_card(const Game& game, domain::Player player, domain::Zone zone, std::vector<CardInstanceId>& out) const noexcept -> void;
		auto collect_zones_card(domain::Player player, utility::EnumMask<domain::Zone> zones, std::vector<CardInstanceId>& out) const noexcept -> void;
		auto collect_zones_card(const Game& game, domain::Player player, utility::EnumMask<domain::Zone> zones, std::vector<CardInstanceId>& out) const noexcept -> void;

	public:
		// 获取某个区域所有卡牌实例
		// 如果是额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto collect_zone_card(domain::Player player, domain::Zone zone) const noexcept -> std::vector<CardInstanceId>;
		// 获取某个区域所有卡牌实例
		// 如果是额外怪兽区,则不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto collect_zone_card(const Game& game, domain::Player player, domain::Zone zone) const noexcept -> std::vector<CardInstanceId>;

		// 获取某些区域所有卡牌实例
		// 如果包含额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto collect_zones_card(domain::Player player, utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<CardInstanceId>;
		// 获取某些区域所有卡牌实例
		// 如果包含额外怪兽区,则不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto collect_zones_card(const Game& game, domain::Player player, utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<CardInstanceId>;

		// 获取双方某些区域所有卡牌实例
		[[nodiscard]] auto collect_zones_card(utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<CardInstanceId>;

		// ==================================================================================
		// 指定位置的卡牌实例
		// ==================================================================================

		// 检查某个位置是否为空(不存在卡牌实例) -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义,其他区域一律返回false
		[[nodiscard]] auto is_location_empty(domain::Player player, domain::Location location) const noexcept -> bool;
		// 检查某个位置是否为空(不存在卡牌实例)
		// [[nodiscard]] auto is_location_empty(domain::Player player, domain::AnyFieldLocation location) const noexcept -> bool;

		// 检查双方玩家某个位置是否为空(不存在卡牌实例) -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义,其他区域一律返回false
		[[nodiscard]] auto is_location_empty(domain::Location location) const noexcept -> bool;
		// 检查双方玩家某个位置是否为空(不存在卡牌实例)
		// [[nodiscard]] auto is_location_empty(domain::AnyFieldLocation location) const noexcept -> bool;

		// 获取指定位置的卡牌实例,如果目标位置不存在卡牌实例则返回无效ID -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义,其他区域一律返回无效ID
		[[nodiscard]] auto select_location_card(domain::Player player, domain::Location location) const noexcept -> CardInstanceId;
		// 获取指定位置的卡牌实例,如果目标位置不存在卡牌实例则返回无效ID
		[[nodiscard]] auto select_location_card(domain::Player player, domain::AnyFieldLocation location) const noexcept -> CardInstanceId;

		// 将卡放入指定位置(不处理原位置),返回是否覆盖(即原位置是否有卡) -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义,其他区域一律追加(无视位置,放在对应区域底部,返回false)
		auto place(domain::Player player, CardInstanceId card, domain::Location location) noexcept -> bool;
		// 将卡放入指定位置(不处理原位置)
		auto place(domain::Player player, CardInstanceId card, domain::AnyBagLocation location, domain::BagPlacement bag) noexcept -> void;
		// 将卡放入指定位置(不处理原位置),返回是否覆盖(即原位置是否有卡)
		auto place(domain::Player player, CardInstanceId card, domain::AnyFieldLocation location) noexcept -> bool;

		// 将卡从指定位置移除,返回是否移除成功 -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义,其他区域一律擦除指定卡(无视位置,返回是否擦除成功(即该卡在指定区域存在))
		auto remove(domain::Player player, CardInstanceId card, domain::Location location) noexcept -> bool;
		// 将卡从指定位置移除,返回是否擦除成功(即该卡在指定区域存在)
		auto remove(domain::Player player, CardInstanceId card, domain::AnyBagLocation location) noexcept -> bool;
		// 将卡从指定位置移除,返回是否移除成功
		auto remove(domain::Player player, domain::AnyFieldLocation location) noexcept -> bool;

		// 将卡放入指定位置(不处理原位置),返回是否覆盖(即原位置是否有卡) -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义,其他区域一律追加(无视位置,放在对应区域底部,返回false)
		auto place(domain::Player player, CardInstanceId card, domain::Placement placement) noexcept -> bool;
		// 将卡放入指定位置(不处理原位置)
		auto place(domain::Player player, CardInstanceId card, domain::AnyBagPlacement placement, domain::BagPlacement bag) noexcept -> void;
		// 将卡放入指定位置(不处理原位置),返回是否覆盖(即原位置是否有卡)
		auto place(domain::Player player, CardInstanceId card, domain::AnyFieldPlacement placement) noexcept -> bool;

		// 将卡从指定位置移除,返回是否移除成功 -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义,其他区域一律擦除指定卡(无视位置,返回是否擦除成功(即该卡在指定区域存在))
		auto remove(domain::Player player, CardInstanceId card, domain::Placement placement) noexcept -> bool;
		// 将卡从指定位置移除,返回是否擦除成功(即该卡在指定区域存在)
		auto remove(domain::Player player, CardInstanceId card, domain::AnyBagPlacement placement) noexcept -> bool;
		// 将卡从指定位置移除,返回是否移除成功
		auto remove(domain::Player player, domain::AnyFieldPlacement placement) noexcept -> bool;
	};
}

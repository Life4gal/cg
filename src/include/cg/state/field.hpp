#pragma once

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
		std::underlying_type_t<domain::AttackDefense> life_point;
		// 主要怪兽区
		field_zones_type monster_zones;
		// 魔陷区
		field_zones_type spell_trap_zones;
		// 场地魔法区
		CardInstanceId field_spell;
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

	public:
		Field() noexcept;

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

		[[nodiscard]] auto is_deck_zones_empty(domain::Player player) const noexcept -> bool;
		[[nodiscard]] auto is_extra_deck_zones_empty(domain::Player player) const noexcept -> bool;
		[[nodiscard]] auto is_hand_zones_empty(domain::Player player) const noexcept -> bool;
		[[nodiscard]] auto is_graveyard_zones_empty(domain::Player player) const noexcept -> bool;
		[[nodiscard]] auto is_banished_zones_empty(domain::Player player) const noexcept -> bool;
		[[nodiscard]] auto is_monster_zones_empty(domain::Player player) const noexcept -> bool;
		[[nodiscard]] auto is_extra_monster_zones_empty(domain::Player player) const noexcept -> bool;
		[[nodiscard]] auto is_spell_trap_zones_empty(domain::Player player) const noexcept -> bool;
		[[nodiscard]] auto is_field_spell_zones_empty(domain::Player player) const noexcept -> bool;

		// 检查某个区域是否不存在卡牌实例
		[[nodiscard]] auto is_zones_empty(domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> bool;

		// 检查某个区域是否不存在卡牌实例
		[[nodiscard]] auto is_zones_empty(utility::EnumMask<domain::Zone> zone) const noexcept -> bool;

		[[nodiscard]] auto count_deck_zones_card(domain::Player player) const noexcept -> size_type;
		[[nodiscard]] auto count_extra_deck_zones_card(domain::Player player) const noexcept -> size_type;
		[[nodiscard]] auto count_hand_zones_card(domain::Player player) const noexcept -> size_type;
		[[nodiscard]] auto count_graveyard_zones_card(domain::Player player) const noexcept -> size_type;
		[[nodiscard]] auto count_banished_zones_card(domain::Player player) const noexcept -> size_type;
		[[nodiscard]] auto count_monster_zones_card(domain::Player player) const noexcept -> size_type;
		// 只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto count_extra_monster_zones_card() const noexcept -> size_type;
		// 不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto count_extra_monster_zones_card(const Game& game, domain::Player player) const noexcept -> size_type;
		[[nodiscard]] auto count_spell_trap_zones_card(domain::Player player) const noexcept -> size_type;
		[[nodiscard]] auto count_field_spell_zones_card(domain::Player player) const noexcept -> size_type;

		// 获取某个区域所有卡牌实例的数量
		// 如果包含额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto count_zones_card(domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t;

		// 获取某个区域所有卡牌实例的数量
		// 如果包含额外怪兽区,则不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto count_zones_card(const Game& game, domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t;

		// 获取某个区域所有卡牌实例的数量
		// 如果包含额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto count_zones_card(utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t;

	private:
		auto collect_zones(domain::Player player, utility::EnumMask<domain::Zone> zone, std::vector<CardInstanceId>& out) const noexcept -> void;

		auto collect_zones(const Game& game, domain::Player player, utility::EnumMask<domain::Zone> zone, std::vector<CardInstanceId>& out) const noexcept -> void;

	public:
		[[nodiscard]] auto collect_deck_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_extra_deck_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_hand_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_graveyard_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_banished_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_monster_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		// 只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto collect_extra_monster_zones_card() const noexcept -> std::vector<CardInstanceId>;
		// 不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto collect_extra_monster_zones_card(const Game& game, domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_spell_trap_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_field_spell_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;

		// 获取某个区域所有卡牌实例
		// 如果包含额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto collect_zones_card(domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>;

		// 获取某个区域所有卡牌实例
		// 如果包含额外怪兽区,则不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto collect_zones_card(const Game& game, domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>;

		// 获取某个区域所有卡牌实例
		[[nodiscard]] auto collect_zones_card(utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>;

		// ==================================================================================
		// 指定位置的卡牌实例
		// ==================================================================================

	private:
		[[nodiscard]] auto is_location_empty(domain::Player player, domain::ExtraMonsterLocation location) const noexcept -> bool;
		[[nodiscard]] auto select_location_card(domain::Player player, domain::ExtraMonsterLocation location) const noexcept -> CardInstanceId;

	public:
		[[nodiscard]] auto is_location_empty(domain::Player player, domain::MonsterLocation location) const noexcept -> bool;
		[[nodiscard]] auto is_location_empty(domain::ExtraMonsterLocation location) const noexcept -> bool;
		[[nodiscard]] auto is_location_empty(domain::Player player, domain::SpellTrapLocation location) const noexcept -> bool;
		[[nodiscard]] auto is_location_empty(domain::Player player, domain::FieldSpellLocation location) const noexcept -> bool;

		// 检查某个位置是否为空(不存在卡牌实例) -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义
		[[nodiscard]] auto is_location_empty(domain::Player player, domain::Location location) const noexcept -> bool;

		// 检查某个位置是否为空(不存在卡牌实例) -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义
		[[nodiscard]] auto is_location_empty(domain::Location location) const noexcept -> bool;

		[[nodiscard]] auto select_location_card(domain::Player player, domain::DeckLocation location) const noexcept -> CardInstanceId;
		[[nodiscard]] auto select_location_card(domain::Player player, domain::ExtraDeckLocation location) const noexcept -> CardInstanceId;
		[[nodiscard]] auto select_location_card(domain::Player player, domain::HandLocation location) const noexcept -> CardInstanceId;
		[[nodiscard]] auto select_location_card(domain::Player player, domain::GraveyardLocation location) const noexcept -> CardInstanceId;
		[[nodiscard]] auto select_location_card(domain::Player player, domain::BanishedLocation location) const noexcept -> CardInstanceId;
		[[nodiscard]] auto select_location_card(domain::Player player, domain::MonsterLocation location) const noexcept -> CardInstanceId;
		[[nodiscard]] auto select_location_card(domain::ExtraMonsterLocation location) const noexcept -> CardInstanceId;
		[[nodiscard]] auto select_location_card(domain::Player player, domain::SpellTrapLocation location) const noexcept -> CardInstanceId;
		[[nodiscard]] auto select_location_card(domain::Player player, domain::FieldSpellLocation location) const noexcept -> CardInstanceId;

		// 获取指定位置的卡牌实例
		// 如果目标位置不存在卡牌实例则返回无效ID
		[[nodiscard]] auto select_location_card(domain::Player player, domain::Location location) const noexcept -> CardInstanceId;
	};
}

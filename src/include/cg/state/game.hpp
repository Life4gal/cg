#pragma once

#include <map>
#include <span>
#include <vector>

#include <cg/state/id.hpp>
#include <cg/state/card.hpp>
#include <cg/state/field.hpp>

namespace cg::state
{
	class Game
	{
	public:
		using card_definitions_type = std::map<domain::CardCode, CardDefinition>;
		using card_instances_type = std::vector<CardInstance>;

	private:
		// 游戏中所有卡牌的定义
		card_definitions_type card_definitions_;
		// 游戏中创建的所有卡牌实例
		card_instances_type card_instances_;
		// 游戏场地
		Field field_;

		[[nodiscard]] auto generate_instance_id() const noexcept -> CardInstanceId;
		[[nodiscard]] auto is_valid_instance_id(CardInstanceId card) const noexcept -> bool;

		[[nodiscard]] auto find_definition(domain::CardCode code) const noexcept -> const CardDefinition*;
		[[nodiscard]] auto find_instance_mutable(CardInstanceId card) noexcept -> CardInstance*;

	public:
		// 注册定义
		auto register_definition(domain::CardCode code, const CardDefinition& definition) noexcept -> const CardDefinition&;

		// 指定卡密所属卡牌是否存在定义
		[[nodiscard]] auto has_definition(domain::CardCode code) const noexcept -> bool;

		// 获取指定卡密所属卡牌的定义

		// 创建实例
		[[nodiscard]] auto create_instance(domain::CardCode code, domain::Player owner) noexcept -> CardInstanceId;

		// 全部实例(只读)
		[[nodiscard]] auto instances() const noexcept -> std::span<const CardInstance>;

		// 指定实例ID所属实例是否存在
		[[nodiscard]] auto has_instance(CardInstanceId card) const noexcept -> bool;

		// 获取实例ID所属实例
		[[nodiscard]] auto find_instance(CardInstanceId card) const noexcept -> const CardInstance*;

		// ================================================================
		//
		// ================================================================

		// ==================================================================================
		// 卡牌实例ID + 接口 ==> 场地 + 卡牌实例ID + 接口 ==> 场地->接口(卡牌实例ID)
		// 玩家 + 区域 + 接口 ==> 场地 + 玩家 + 区域 + 接口 ==> 场地->接口(玩家+场地)
		// ==================================================================================

	private:
		template<typename Interface, typename... Args>
			requires std::is_invocable_v<Interface, Field&, Args...>
		auto field_invoke(Interface&& interface, Args&&... args) noexcept -> decltype(auto)
		{
			return std::invoke(std::forward<Interface>(interface), field_, std::forward<Args>(args)...);
		}

		template<typename Interface, typename... Args>
			requires std::is_invocable_v<Interface, const Field&, Args...>
		auto field_invoke(Interface&& interface, Args&&... args) const noexcept -> decltype(auto)
		{
			return std::invoke(std::forward<Interface>(interface), field_, std::forward<Args>(args)...);
		}

	public:
		// ================================================================
		// 生命值
		// ================================================================

		auto set_life_points(domain::Player player, std::underlying_type_t<domain::AttackDefense> value) noexcept -> void;
		[[nodiscard]] auto life_points(domain::Player player) const noexcept -> std::underlying_type_t<domain::AttackDefense>;
		auto change_life_points(domain::Player player, std::underlying_type_t<domain::AttackDefense> delta) noexcept -> void;

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
		[[nodiscard]] auto is_zones_empty(utility::EnumMask<domain::Zone> zones) const noexcept -> bool;

		// 获取某个区域所有卡牌实例的数量
		// 如果是额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto count_zone_card(domain::Player player, domain::Zone zone) const noexcept -> Field::size_type;
		// 获取某个区域所有卡牌实例的数量
		// 如果是额外怪兽区,则不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto count_zone_card_ex(domain::Player player, domain::Zone zone) const noexcept -> Field::size_type;

		// 获取某些区域所有卡牌实例的数量
		// 如果包含额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto count_zones_card(domain::Player player, utility::EnumMask<domain::Zone> zones) const noexcept -> Field::size_type;
		// 获取双方某些区域所有卡牌实例的数量
		// 如果包含额外怪兽区,则不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto count_zones_card_ex(domain::Player player, utility::EnumMask<domain::Zone> zones) const noexcept -> Field::size_type;

		// 获取双方某些区域所有卡牌实例的数量
		[[nodiscard]] auto count_zones_card(utility::EnumMask<domain::Zone> zones) const noexcept -> Field::size_type;

		// 获取某个区域所有卡牌实例
		// 如果是额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto collect_zone_card(domain::Player player, domain::Zone zone) const noexcept -> std::vector<CardInstanceId>;
		// 获取某个区域所有卡牌实例
		// 如果是额外怪兽区,则不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto collect_zone_card_ex(domain::Player player, domain::Zone zone) const noexcept -> std::vector<CardInstanceId>;

		// 获取某个区域所有卡牌实例
		// 如果包含额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto collect_zones_card(domain::Player player, utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<CardInstanceId>;
		// 获取某个区域所有卡牌实例
		// 如果包含额外怪兽区,则不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto collect_zones_card_ex(domain::Player player, utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<CardInstanceId>;

		// 获取某个区域所有卡牌实例
		[[nodiscard]] auto collect_zones_card(utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<CardInstanceId>;

		// ==================================================================================
		// 指定位置的卡牌实例
		// ==================================================================================

		// 检查某个位置是否为空(不存在卡牌实例) -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义,其他区域一律返回false
		[[nodiscard]] auto is_location_empty(domain::Player player, domain::Location location) const noexcept -> bool;
		// 检查某个位置是否为空(不存在卡牌实例)
		[[nodiscard]] auto is_location_empty(domain::Player player, domain::FieldLocation location) const noexcept -> bool;
		// 检查某个位置是否为空(不存在卡牌实例)
		[[nodiscard]] auto is_location_empty(domain::Player player, domain::AnyMonsterLocation location) const noexcept -> bool;

		// 检查双方玩家某个位置是否为空(不存在卡牌实例) -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义
		[[nodiscard]] auto is_location_empty(domain::Location location) const noexcept -> bool;
		// 检查双方玩家某个位置是否为空(不存在卡牌实例)
		[[nodiscard]] auto is_location_empty(domain::FieldLocation location) const noexcept -> bool;
		// 检查双方玩家某个位置是否为空(不存在卡牌实例)
		[[nodiscard]] auto is_location_empty(domain::AnyMonsterLocation location) const noexcept -> bool;

		// 获取指定位置的卡牌实例,如果目标位置不存在卡牌实例则返回无效ID
		[[nodiscard]] auto select_location_card(domain::Player player, domain::Location location) const noexcept -> CardInstanceId;
		// 获取指定位置的卡牌实例,如果目标位置不存在卡牌实例则返回无效ID
		[[nodiscard]] auto select_location_card(domain::Player player, domain::FieldLocation location) const noexcept -> CardInstanceId;
		// 获取指定位置的卡牌实例,如果目标位置不存在卡牌实例则返回无效ID
		[[nodiscard]] auto select_location_card(domain::Player player, domain::AnyMonsterLocation location) const noexcept -> CardInstanceId;

		// ==================================================================================
		// 指定位置的卡牌实例
		// ==================================================================================

		// 将卡放入其持有者的指定位置(自动从原位置移除) -- 指定实例不存在返回false
		auto place(CardInstanceId card, domain::Placement placement) noexcept -> bool;

	private:
		template<typename PlacementGetter>
			requires std::is_invocable_r_v<domain::Placement, PlacementGetter, domain::Player>
		auto do_move_to(const CardInstanceId card, const utility::EnumMask<domain::Reason> reason, PlacementGetter getter) noexcept -> bool
		{
			auto* instance = find_instance_mutable(card);
			if (instance == nullptr)
			{
				return false;
			}

			instance->set_move_provenance(
				{
						.controller = instance->controller(),
						.reason = reason,
						.placement = instance->placement(),
						.effect = dsl::EffectId{dsl::EffectId::invalid_id}
				}
			);

			return this->place(card, getter(instance->owner()));
		}

	public:
		// 将卡送入其持有者的指定位置 -- 指定实例不存在返回false
		auto move_to_deck(CardInstanceId card, utility::EnumMask<domain::Reason> reason) noexcept -> bool;
		// 将卡送入其持有者的指定位置 -- 指定实例不存在返回false
		auto move_to_extra_deck(CardInstanceId card, utility::EnumMask<domain::Reason> reason, domain::Position position = domain::Position::FACE_DOWN_DEFENSE) noexcept -> bool;
		// 将卡送入其持有者的指定位置 -- 指定实例不存在返回false
		auto move_to_hand(CardInstanceId card, utility::EnumMask<domain::Reason> reason, domain::Position position = domain::Position::FACE_DOWN_DEFENSE) noexcept -> bool;
		// 将卡送入其持有者的指定位置 -- 指定实例不存在返回false
		auto move_to_graveyard(CardInstanceId card, utility::EnumMask<domain::Reason> reason) noexcept -> bool;
		// 将卡送入其持有者的指定位置 -- 指定实例不存在返回false
		auto move_to_banished(CardInstanceId card, utility::EnumMask<domain::Reason> reason, domain::Position position = domain::Position::FACE_UP_ATTACK) noexcept -> bool;
		// 将卡送入其持有者的指定位置 -- 指定实例不存在返回false
		auto move_to_monster(CardInstanceId card, utility::EnumMask<domain::Reason> reason, domain::MonsterPlacement placement) noexcept -> bool;
		// 将卡送入其持有者的指定位置 -- 指定实例不存在返回false
		auto move_to_extra_monster(CardInstanceId card, utility::EnumMask<domain::Reason> reason, domain::ExtraMonsterPlacement placement) noexcept -> bool;
		// 将卡送入其持有者的指定位置 -- 指定实例不存在返回false
		auto move_to_spell_trap(CardInstanceId card, utility::EnumMask<domain::Reason> reason, domain::SpellTrapPlacement placement) noexcept -> bool;
		// 将卡送入其持有者的指定位置 -- 指定实例不存在返回false
		auto move_to_field_spell(CardInstanceId card, utility::EnumMask<domain::Reason> reason, domain::FieldSpellPlacement placement) noexcept -> bool;
	};
}

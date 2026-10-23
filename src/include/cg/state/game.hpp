#pragma once

#include <map>
#include <span>
#include <vector>

#include <cg/state/id.hpp>
#include <cg/state/card.hpp>
#include <cg/state/field.hpp>

namespace cg::state
{
	// 对局状态容器: 定义/实例/场地/生命值.
	// 接口分层:
	// - 装配/初始化: register_definition, create_instance, set_life_points
	// - 只读查询: life_points, zone_of, is_*_empty, count/collect, find_instance
	// - 状态变更: place, move_to_*, set_summon_record, add/remove_overlay
	//   (由 flow/execution 驱动;外部驱动不应直接改状态)
	// - 暂无调用者的查询暂保留,待 server bootstrap 落地后收敛
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

	public:
		// =========================================================================================================
		// 定义 / 实例
		// =========================================================================================================

		// 装配:注册卡牌定义
		auto register_definition(domain::CardCode code, const CardDefinition& definition) noexcept -> const CardDefinition&;

		// 指定卡密所属卡牌是否存在定义
		[[nodiscard]] auto has_definition(domain::CardCode code) const noexcept -> bool;

		// 创建实例,如果指定卡密未注册定义则无效ID
		// 这个接口必须返回ID,指针可能因为重分配失效
		[[nodiscard]] auto create_instance(domain::CardCode code, domain::Player owner) noexcept -> CardInstanceId;

		// 全部实例(只读)
		[[nodiscard]] auto instances() const noexcept -> std::span<const CardInstance>;

		// 指定实例ID所属实例是否存在
		[[nodiscard]] auto has_instance(CardInstanceId card) const noexcept -> bool;

		// 获取实例ID对应的实例
		[[nodiscard]] auto find_instance(CardInstanceId card) const noexcept -> const CardInstance*;

		// 可变访问仅限 flow/execution;外部驱动应使用const查询
		[[nodiscard]] auto find_instance_mutable(CardInstanceId card) noexcept -> CardInstance*;

	private:
		// ===========================================================================================================================
		// 卡牌实例ID + 接口 ==> 场地 + 卡牌实例ID + 接口 ==> 场地->接口(卡牌实例ID)
		// 玩家 + 区域 + 接口 ==> 场地 + 玩家 + 区域 + 接口 ==> 场地->接口(玩家+场地)
		// ===========================================================================================================================

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
		// =========================================================================================================
		// 场地
		// =========================================================================================================

		// 初始化:设置初始 LP(对局前/测试夹具)
		auto set_life_points(domain::Player player, std::underlying_type_t<domain::AttackDefense> value) noexcept -> void;
		[[nodiscard]] auto life_points(domain::Player player) const noexcept -> std::underlying_type_t<domain::AttackDefense>;
		auto change_life_points(domain::Player player, std::underlying_type_t<domain::AttackDefense> delta) noexcept -> void;

		// ================================================================
		// 指定卡牌的区域/位置
		// ================================================================

		// 获取卡牌实例所在区域
		[[nodiscard]] auto zone_of(CardInstanceId card) const noexcept -> domain::Zone;

		// 获取卡牌实例所在位置
		[[nodiscard]] auto location_of(CardInstanceId card) const noexcept -> domain::Location;

		// ================================================================
		// 指定区域的卡牌实例
		// ================================================================

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

		// ================================================================
		// 指定位置的卡牌实例
		// ================================================================

		// 检查某个位置是否为空(不存在卡牌实例) -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义,其他区域一律返回false
		[[nodiscard]] auto is_location_empty(domain::Player player, domain::Location location) const noexcept -> bool;
		// 检查某个位置是否为空(不存在卡牌实例)
		[[nodiscard]] auto is_location_empty(domain::Player player, domain::AnyFieldLocation location) const noexcept -> bool;
		// 检查某个位置是否为空(不存在卡牌实例)
		[[nodiscard]] auto is_location_empty(domain::Player player, domain::AnyMonsterLocation location) const noexcept -> bool;

		// 检查双方玩家某个位置是否为空(不存在卡牌实例) -- 仅对怪兽区/额外怪兽区/魔陷区/场地魔法区有意义
		[[nodiscard]] auto is_location_empty(domain::Location location) const noexcept -> bool;
		// 检查双方玩家某个位置是否为空(不存在卡牌实例)
		[[nodiscard]] auto is_location_empty(domain::AnyFieldLocation location) const noexcept -> bool;
		// 检查双方玩家某个位置是否为空(不存在卡牌实例)
		[[nodiscard]] auto is_location_empty(domain::AnyMonsterLocation location) const noexcept -> bool;

		// 获取指定位置的卡牌实例,如果目标位置不存在卡牌实例则返回无效ID
		[[nodiscard]] auto select_location_card(domain::Player player, domain::Location location) const noexcept -> CardInstanceId;
		// 获取指定位置的卡牌实例,如果目标位置不存在卡牌实例则返回无效ID
		[[nodiscard]] auto select_location_card(domain::Player player, domain::AnyFieldLocation location) const noexcept -> CardInstanceId;
		// 获取指定位置的卡牌实例,如果目标位置不存在卡牌实例则返回无效ID
		[[nodiscard]] auto select_location_card(domain::Player player, domain::AnyMonsterLocation location) const noexcept -> CardInstanceId;

		// =========================================================================================================
		// 卡牌实例
		// =========================================================================================================

		// 将卡放入其持有者的指定位置(自动从原位置移除)
		template<typename P>
			requires(utility::variant_index_v<P, domain::Placement> != utility::invalid_variant_index)
		auto place(CardInstance& card, const P placement) noexcept -> void
		{
			this->place(card, domain::Placement{placement});
		}

		// 将卡放入其持有者的指定位置(自动从原位置移除)
		auto place(CardInstance& card, domain::Placement placement) noexcept -> void;

	private:
		template<typename P>
			requires(utility::variant_index_v<P, domain::Placement> != utility::invalid_variant_index)
		auto move_to(CardInstance& card, const utility::EnumMask<domain::Reason> reason, const P placement) noexcept -> void
		{
			this->move_to(card, reason, domain::Placement{placement});
		}

		auto move_to(CardInstance& card, utility::EnumMask<domain::Reason> reason, domain::Placement placement) noexcept -> void;

	public:
		// 将卡送入其持有者的指定位置
		auto move_to_deck(CardInstance& card, utility::EnumMask<domain::Reason> reason) noexcept -> void;
		// 将卡送入其持有者的指定位置
		auto move_to_extra_deck(CardInstance& card, utility::EnumMask<domain::Reason> reason, domain::Position position = domain::Position::FACE_DOWN_DEFENSE) noexcept -> void;
		// 将卡送入其持有者的指定位置
		auto move_to_hand(CardInstance& card, utility::EnumMask<domain::Reason> reason, domain::Position position = domain::Position::FACE_DOWN_DEFENSE) noexcept -> void;
		// 将卡送入其持有者的指定位置
		auto move_to_graveyard(CardInstance& card, utility::EnumMask<domain::Reason> reason) noexcept -> void;
		// 将卡送入其持有者的指定位置
		auto move_to_banished(CardInstance& card, utility::EnumMask<domain::Reason> reason, domain::Position position = domain::Position::FACE_UP_ATTACK) noexcept -> void;
		// 将卡送入其持有者的指定位置
		auto move_to_monster(CardInstance& card, utility::EnumMask<domain::Reason> reason, domain::MonsterPlacement placement) noexcept -> void;
		// 将卡送入其持有者的指定位置
		auto move_to_extra_monster(CardInstance& card, utility::EnumMask<domain::Reason> reason, domain::ExtraMonsterPlacement placement) noexcept -> void;
		// 将卡送入其持有者的指定位置
		auto move_to_spell_trap(CardInstance& card, utility::EnumMask<domain::Reason> reason, domain::SpellTrapPlacement placement) noexcept -> void;
		// 将卡送入其持有者的指定位置
		auto move_to_field_spell(CardInstance& card, utility::EnumMask<domain::Reason> reason, domain::FieldSpellPlacement placement) noexcept -> void;

		// 记录召唤信息
		auto set_summon_record(CardInstance& card, CardInstance::SummonRecord record) noexcept -> void;

		// 添加超量素材
		auto add_overlay(CardInstance& card, const CardInstance& material) noexcept -> void;
		// 移除超量素材
		auto remove_overlay(CardInstance& card, const CardInstance& material) noexcept -> void;
	};
}

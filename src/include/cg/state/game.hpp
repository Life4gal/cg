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

	public:
		// 注册定义
		auto register_definition(domain::CardCode code, const CardDefinition& definition) noexcept -> const CardDefinition&;

		// 指定卡密所属卡牌是否存在定义
		[[nodiscard]] auto has_definition(domain::CardCode code) const noexcept -> bool;

		// 获取指定卡密所属卡牌的定义
		[[nodiscard]] auto find_definition(domain::CardCode code) const noexcept -> const CardDefinition*;

		// 创建实例
		[[nodiscard]] auto create_instance(domain::CardCode code, domain::Player owner) noexcept -> CardInstanceId;

		// 全部实例(只读)
		[[nodiscard]] auto instances() const noexcept -> std::span<const CardInstance>;

		// 指定实例ID所属实例是否存在
		[[nodiscard]] auto has_instance(CardInstanceId card) const noexcept -> bool;

		// 获取实例ID所属实例
		[[nodiscard]] auto find_instance(CardInstanceId card) const noexcept -> const CardInstance*;

	private:
		[[nodiscard]] auto find_instance_mutable(CardInstanceId card) noexcept -> CardInstance*;

		// ==================================================================================
		// 卡牌实例ID + 接口 ==> instance + 接口 ==> instance->接口
		// 卡牌实例ID对应实例不存在 => 返回null-opt
		// ==================================================================================

		template<typename Interface, typename... Args>
			requires std::is_invocable_v<Interface, const CardInstance*, Args...>
		auto instance_invoke(
			const CardInstanceId card,
			Interface&& interface,
			Args&&... args) const noexcept
			-> std::optional<std::conditional_t<
				std::is_lvalue_reference_v<std::invoke_result_t<Interface, const CardInstance*, Args...>>,
				// 引用类型
				std::reference_wrapper<std::remove_reference_t<std::invoke_result_t<Interface, const CardInstance*, Args...>>>,
				// 值类型
				std::invoke_result_t<Interface, const CardInstance*, Args...>
			>>
		{
			const auto* instance = find_instance(card);
			if (instance == nullptr)
			{
				return std::nullopt;
			}

			return std::invoke(std::forward<Interface>(interface), instance, std::forward<Args>(args)...);
		}

	public:
		// ================================================================
		// 实例信息
		// ================================================================

		// 卡牌定义
		[[nodiscard]] auto definition(CardInstanceId card) const noexcept -> std::optional<std::reference_wrapper<const CardDefinition>>;

		// 卡牌拥有者
		[[nodiscard]] auto owner(CardInstanceId card) const noexcept -> std::optional<domain::Player>;

		// 卡牌控制者
		[[nodiscard]] auto controller(CardInstanceId card) const noexcept -> std::optional<domain::Player>;

		// 卡牌当前位置
		[[nodiscard]] auto placement(CardInstanceId card) const noexcept -> std::optional<domain::Placement>;

		// ================================================================
		// 定义查询
		// ================================================================

		// 卡牌卡密
		[[nodiscard]] auto code(CardInstanceId card) const noexcept -> std::optional<domain::CardCode>;

		// 是指定卡类型
		[[nodiscard]] auto is_card_type(CardInstanceId card, domain::CardType type) const noexcept -> std::optional<bool>;

		// 是否是怪兽卡
		[[nodiscard]] auto is_monster_card(CardInstanceId card) const noexcept -> std::optional<bool>;

		// 是否是魔法卡
		[[nodiscard]] auto is_spell_card(CardInstanceId card) const noexcept -> std::optional<bool>;

		// 是否是陷阱卡
		[[nodiscard]] auto is_trap_card(CardInstanceId card) const noexcept -> std::optional<bool>;

		// 是指定卡子类型
		[[nodiscard]] auto is_card_subtype(CardInstanceId card, domain::CardSubtype subtype) const noexcept -> std::optional<bool>;

		// 是指定卡子类型之一
		[[nodiscard]] auto is_card_subtype(CardInstanceId card, utility::EnumMask<domain::CardSubtype> subtypes) const noexcept -> std::optional<bool>;

		// 获取属性 -- 不是怪兽卡也返回null-opt
		[[nodiscard]] auto attribute(CardInstanceId card) const noexcept -> std::optional<domain::Attribute>;

		// 是指定属性 -- 不是怪兽卡也返回null-opt
		[[nodiscard]] auto is_attribute(CardInstanceId card, domain::Attribute attribute) const noexcept -> std::optional<bool>;

		// 获取种族 -- 不是怪兽卡也返回null-opt
		[[nodiscard]] auto race(CardInstanceId card) const noexcept -> std::optional<domain::Race>;

		// 是指定种族 -- 不是怪兽卡也返回null-opt
		[[nodiscard]] auto is_race(CardInstanceId card, domain::Race race) const noexcept -> std::optional<bool>;

		// 获取等级/阶级 -- 不是怪兽卡也返回null-opt
		[[nodiscard]] auto level(CardInstanceId card) const noexcept -> std::optional<std::underlying_type_t<domain::LevelRank>>;

		// 是指定等级/阶级 -- 不是怪兽卡也返回null-opt
		[[nodiscard]] auto is_level(CardInstanceId card, std::underlying_type_t<domain::LevelRank> level) const noexcept -> std::optional<bool>;

		// 获取攻击力 -- 不是怪兽卡也返回null-opt
		[[nodiscard]] auto attack(CardInstanceId card) const noexcept -> std::optional<std::underlying_type_t<domain::AttackDefense>>;

		// 是指定攻击力 -- 不是怪兽卡也返回null-opt
		[[nodiscard]] auto is_attack(CardInstanceId card, std::underlying_type_t<domain::AttackDefense> attack) const noexcept -> std::optional<bool>;

		// 获取守备力 -- 不是怪兽卡也返回null-opt
		[[nodiscard]] auto defense(CardInstanceId card) const noexcept -> std::optional<std::underlying_type_t<domain::AttackDefense>>;

		// 是指定守备力 -- 不是怪兽卡也返回null-opt
		[[nodiscard]] auto is_defense(CardInstanceId card, std::underlying_type_t<domain::AttackDefense> defense) const noexcept -> std::optional<bool>;

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

		[[nodiscard]] auto count_deck_zones_card(domain::Player player) const noexcept -> Field::size_type;
		[[nodiscard]] auto count_extra_deck_zones_card(domain::Player player) const noexcept -> Field::size_type;
		[[nodiscard]] auto count_hand_zones_card(domain::Player player) const noexcept -> Field::size_type;
		[[nodiscard]] auto count_graveyard_zones_card(domain::Player player) const noexcept -> Field::size_type;
		[[nodiscard]] auto count_banished_zones_card(domain::Player player) const noexcept -> Field::size_type;
		[[nodiscard]] auto count_monster_zones_card(domain::Player player) const noexcept -> Field::size_type;
		// 只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto count_extra_monster_zones_card() const noexcept -> Field::size_type;
		// 不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto count_extra_monster_zones_card(domain::Player player) const noexcept -> Field::size_type;
		[[nodiscard]] auto count_spell_trap_zones_card(domain::Player player) const noexcept -> Field::size_type;
		[[nodiscard]] auto count_field_spell_zones_card(domain::Player player) const noexcept -> Field::size_type;

		// 获取某个区域所有卡牌实例的数量
		// 如果包含额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto count_zones_card(domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t;

		// 获取某个区域所有卡牌实例的数量
		// 如果包含额外怪兽区,则不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto count_zones_card_ex(domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t;

		// 获取某个区域所有卡牌实例的数量
		// 如果包含额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto count_zones_card(utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t;

		[[nodiscard]] auto collect_deck_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_extra_deck_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_hand_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_graveyard_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_banished_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_monster_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		// 只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto collect_extra_monster_zones_card() const noexcept -> std::vector<CardInstanceId>;
		// 不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto collect_extra_monster_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_spell_trap_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;
		[[nodiscard]] auto collect_field_spell_zones_card(domain::Player player) const noexcept -> std::vector<CardInstanceId>;

		// 获取某个区域所有卡牌实例
		// 如果包含额外怪兽区,则只要在额外怪兽区就算,不论控制者
		[[nodiscard]] auto collect_zones_card(domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>;

		// 获取某个区域所有卡牌实例
		// 如果包含额外怪兽区,则不仅要在额外怪兽区,还要求控制者为指定玩家
		[[nodiscard]] auto collect_zones_card_ex(domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>;

		// 获取某个区域所有卡牌实例
		[[nodiscard]] auto collect_zones_card(utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>;

		// ==================================================================================
		// 指定位置的卡牌实例
		// ==================================================================================

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

		// ==================================================================================
		// 指定位置的卡牌实例
		// ==================================================================================

	private:
		template<typename Placement>
		auto do_place(const CardInstanceId card, const Placement placement) noexcept -> bool
		{
			auto* instance = find_instance_mutable(card);
			if (instance == nullptr)
			{
				return false;
			}

			const auto prev = instance->placement();
			field_.remove(instance->controller(), card, prev);

			instance->set_placement({placement});
			field_.place(instance->controller(), card, placement);

			return true;
		}

	public:
		auto place(CardInstanceId card, domain::DeckPlacement placement) noexcept -> bool;
		auto place(CardInstanceId card, domain::ExtraDeckPlacement placement) noexcept -> bool;
		auto place(CardInstanceId card, domain::HandPlacement placement) noexcept -> bool;
		auto place(CardInstanceId card, domain::GraveyardPlacement placement) noexcept -> bool;
		auto place(CardInstanceId card, domain::BanishedPlacement placement) noexcept -> bool;
		auto place(CardInstanceId card, domain::MonsterPlacement placement) noexcept -> bool;
		auto place(CardInstanceId card, domain::ExtraMonsterPlacement placement) noexcept -> bool;
		auto place(CardInstanceId card, domain::SpellTrapPlacement placement) noexcept -> bool;
		auto place(CardInstanceId card, domain::FieldSpellPlacement placement) noexcept -> bool;

		// 将卡放入其持有者的指定位置(自动从原位置移除) -- 指定实例不存在返回false
		auto place(CardInstanceId card, domain::Placement placement) noexcept -> bool;

	private:
		template<typename PlacementGetter>
			requires std::is_invocable_v<PlacementGetter, domain::Player>
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
		auto move_to_extra_deck(CardInstanceId card, utility::EnumMask<domain::Reason> reason) noexcept -> bool;
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

#pragma once

#include <core/domain/summon.hpp>

#include <core/engine/prototype.hpp>
#include <core/engine/card_state.hpp>
#include <core/engine/turn_state.hpp>
#include <core/engine/collection.hpp>

namespace cg::engine
{
	class Duel;

	class Card
	{
	public:
		class PrototypeHandler;
		class PropertyHandler;
		class StateHandler;
		class SummonHandler;
		class BattleHandler;
		class TargetHandler;
		class XyzHandler;
		class EquipHandler;

	private:
		class Summon
		{
		public:
			// 召唤的类型
			domain::SummonKind kind;
			// 从哪里召唤
			domain::Zone from_zone;
			// 召唤的玩家
			domain::Player player;
			// 召唤的回合
			turn_index_type turn_index;
			// 召唤使用的材料
			Group materials;
		};

		class Battle
		{
		public:
			// 本回合攻击过的卡
			Group attacked_cards;
			// 本回合交战过的卡
			Group battled_cards;
			// 攻击过的次数
			std::size_t attacked_count;
			// 攻击宣言过的次数
			std::size_t attack_announced_count;
			// 上一次攻击的回合号
			turn_index_type attack_turn_index;
			// 上一次攻击被无效的回合号
			turn_index_type attack_canceled_turn_index;
		};

		class Target
		{
		public:
			// 本卡指定为对象的卡
			Group card_targets;
			// 以本卡为对象的卡
			Group owner_targets;
		};

		class Xyz
		{
		public:
			// 超量素材
			// 需要基于CardState::zone_index获取卡,所以容器必须基于插入顺序有序(不能使用Group)
			Sequence materials;
			// 素材所属(如果当前卡是超量素材)
			CardOptional overlay_target;
		};

		class Equip
		{
		public:
			// 装备卡
			// 基于CardState::field_zone获取卡,容器无需基于插入顺序有序
			Group equips;
			// 装备所属(如果当前卡是装备卡)
			CardOptional owner;
		};

		std::reference_wrapper<Duel> duel_;
		card_instance_id_type instance_id_;

		std::reference_wrapper<const Prototype> prototype_;
		CardState state_;
		Summon summon_;
		Battle battle_;
		Target target_;
		Xyz xyz_;
		Equip equip_;

	public:
		// ==================== 原型 ====================

		class PrototypeHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit PrototypeHandler(Card& card) noexcept;

			[[nodiscard]] auto prototype_data() const noexcept -> const Prototype&;

		public:
			[[nodiscard]] auto code() const noexcept -> card_code_type;
			[[nodiscard]] auto code_rule() const noexcept -> card_code_type;
			[[nodiscard]] auto series() const noexcept -> std::array<series_code_type, Prototype::max_series_count>;
			[[nodiscard]] auto card_type() const noexcept -> domain::CardTypeWrapper;
			[[nodiscard]] auto attribute() const noexcept -> domain::AttributeWrapper;
			[[nodiscard]] auto race() const noexcept -> domain::Race;
			[[nodiscard]] auto level() const noexcept -> domain::LevelWrapper;
			[[nodiscard]] auto rank() const noexcept -> domain::Rank;
			[[nodiscard]] auto link_marker() const noexcept -> domain::LinkMarkerWrapper;
			[[nodiscard]] auto attack() const noexcept -> domain::attack_value_type;
			[[nodiscard]] auto defense() const noexcept -> domain::defense_value_type;
			[[nodiscard]] auto left_pendulum() const noexcept -> domain::PendulumScale;
			[[nodiscard]] auto right_pendulum() const noexcept -> domain::PendulumScale;

			[[nodiscard]] auto is_card_type(domain::CardTypeWrapper expected_card_type) const noexcept -> bool;
			[[nodiscard]] auto has_card_type(domain::CardTypeWrapper expected_card_type) const noexcept -> bool;
		};

		// ==================== 属性 ====================

		class PropertyHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit PropertyHandler(Card& card) noexcept;

		public:
			[[nodiscard]] auto card_type() const noexcept -> domain::CardTypeWrapper;
			[[nodiscard]] auto attribute() const noexcept -> domain::AttributeWrapper;
			[[nodiscard]] auto race() const noexcept -> domain::Race;
			[[nodiscard]] auto level() const noexcept -> domain::LevelWrapper;
			[[nodiscard]] auto rank() const noexcept -> domain::Rank;
			[[nodiscard]] auto link_marker() const noexcept -> domain::LinkMarkerWrapper;
			[[nodiscard]] auto attack() const noexcept -> domain::attack_value_type;
			[[nodiscard]] auto defense() const noexcept -> domain::defense_value_type;
			[[nodiscard]] auto left_pendulum() const noexcept -> domain::PendulumScale;
			[[nodiscard]] auto right_pendulum() const noexcept -> domain::PendulumScale;

			[[nodiscard]] auto is_card_type(domain::CardTypeWrapper expected_card_type) const noexcept -> bool;
			[[nodiscard]] auto has_card_type(domain::CardTypeWrapper expected_card_type) const noexcept -> bool;
		};

		// ==================== 运行时状态 ====================

		class StateHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit StateHandler(Card& card) noexcept;

			[[nodiscard]] auto state_data() noexcept -> CardState&;
			[[nodiscard]] auto state_data() const noexcept -> const CardState&;

		public:
			auto set_controller(domain::Player controller) noexcept -> void;
			auto set_zone(domain::Zone zone) noexcept -> void;
			auto set_zone_index(domain::zone_sequence_type zone_index) noexcept -> void;
			auto set_field_zone(domain::FieldZoneSequence field_zone) noexcept -> void;
			auto set_form(domain::FieldZoneForm form) noexcept -> void;
			auto set_reason_player(domain::Player player) noexcept -> void;
			auto set_reason_card(CardOptional card) noexcept -> void;
			auto set_reason_effect(EffectOptional effect) noexcept -> void;
			auto set_reason(domain::ReasonWrapper reason) noexcept -> void;

			[[nodiscard]] auto controller() const noexcept -> domain::Player;
			[[nodiscard]] auto zone() const noexcept -> domain::Zone;
			[[nodiscard]] auto zone_index() const noexcept -> domain::zone_sequence_type;
			[[nodiscard]] auto field_zone() const noexcept -> domain::FieldZoneSequence;
			[[nodiscard]] auto form() const noexcept -> domain::FieldZoneForm;
			[[nodiscard]] auto reason_player() const noexcept -> domain::Player;
			[[nodiscard]] auto reason_card() const noexcept -> CardOptional;
			[[nodiscard]] auto reason_effect() const noexcept -> EffectOptional;
			[[nodiscard]] auto reason() const noexcept -> domain::ReasonWrapper;

			[[nodiscard]] auto is_controller(domain::Player expected_player) const noexcept -> bool;
			[[nodiscard]] auto is_zone(domain::Zone expected_zone) const noexcept -> bool;
			[[nodiscard]] auto is_field_zone(domain::FieldZoneSequence expected_field_zone) const noexcept -> bool;
			[[nodiscard]] auto is_form(domain::FieldZoneForm expected_form) const noexcept -> bool;
			[[nodiscard]] auto is_reason_player(domain::Player expected_player) const noexcept -> bool;
			[[nodiscard]] auto is_reason_card(CardOptional expected_card) const noexcept -> bool;
			[[nodiscard]] auto is_reason_effect(EffectOptional expected_effect) const noexcept -> bool;
			[[nodiscard]] auto is_reason(domain::ReasonWrapper expected_reason) const noexcept -> bool;
			[[nodiscard]] auto has_reason(domain::ReasonWrapper expected_reason) const noexcept -> bool;

			[[nodiscard]] auto is_field_zone() const noexcept -> bool;
			[[nodiscard]] auto is_face_up_form() const noexcept -> bool;
			[[nodiscard]] auto is_face_down_form() const noexcept -> bool;
			[[nodiscard]] auto is_attack_form() const noexcept -> bool;
			[[nodiscard]] auto is_defense_form() const noexcept -> bool;
		};

		// ==================== 召唤信息 ====================

		class SummonHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit SummonHandler(Card& card) noexcept;

			[[nodiscard]] auto summon_data() noexcept -> Summon&;
			[[nodiscard]] auto summon_data() const noexcept -> const Summon&;

		public:
			auto set_kind(domain::SummonKind kind) noexcept -> void;
			auto set_from_zone(domain::Zone from_zone) noexcept -> void;
			auto set_player(domain::Player player) noexcept -> void;
			auto set_turn_index(turn_index_type turn_index) noexcept -> void;
			auto set_materials(Group group) noexcept -> void;

			[[nodiscard]] auto kind() const noexcept -> domain::SummonKind;
			[[nodiscard]] auto from_zone() const noexcept -> domain::Zone;
			[[nodiscard]] auto player() const noexcept -> domain::Player;
			[[nodiscard]] auto turn_index() const noexcept -> turn_index_type;
			[[nodiscard]] auto materials() const noexcept -> View;

			[[nodiscard]] auto is_kind(domain::SummonKind expected_kind) const noexcept -> bool;
			[[nodiscard]] auto is_from_zone(domain::Zone expected_from_zone) const noexcept -> bool;
			[[nodiscard]] auto is_player(domain::Player expected_player) const noexcept -> bool;
			[[nodiscard]] auto is_turn_index(turn_index_type expected_turn_index) const noexcept -> bool;
			[[nodiscard]] auto has_material(const Card& card) const noexcept -> bool;
			[[nodiscard]] auto material_count() const noexcept -> std::size_t;
		};

		// ==================== 战斗记录 ====================

		class BattleHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit BattleHandler(Card& card) noexcept;

			[[nodiscard]] auto battle_data() noexcept -> Battle&;
			[[nodiscard]] auto battle_data() const noexcept -> const Battle&;

		public:
			auto record_attack_announced() noexcept -> void;
			auto record_attack_canceled() noexcept -> void;
			auto record_attacked_card(Card& card) noexcept -> void;

			[[nodiscard]] auto attacked_cards() const noexcept -> View;
			[[nodiscard]] auto attacked_count() const noexcept -> std::size_t;
			[[nodiscard]] auto attack_announced_count() const noexcept -> std::size_t;
			[[nodiscard]] auto attack_turn_index() const noexcept -> turn_index_type;
			[[nodiscard]] auto attack_canceled_turn_index() const noexcept -> turn_index_type;

			[[nodiscard]] auto is_attacked_this_turn() const noexcept -> bool;
			[[nodiscard]] auto is_attack_canceled_this_turn() const noexcept -> bool;
		};

		// ==================== 对象指定 ====================

		class TargetHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit TargetHandler(Card& card) noexcept;

			[[nodiscard]] auto target_data() noexcept -> Target&;
			[[nodiscard]] auto target_data() const noexcept -> const Target&;

			auto set_owner_target(Card& card) noexcept -> bool;
			auto cancel_owner_target(const Card& card) noexcept -> bool;

		public:
			auto set_target(Card& card) noexcept -> bool;
			auto cancel_target(Card& card) noexcept -> bool;

			[[nodiscard]] auto card_targets() const noexcept -> View;
			[[nodiscard]] auto owner_targets() const noexcept -> View;

			[[nodiscard]] auto has_target() const noexcept -> bool;
			[[nodiscard]] auto has_target(const Card& card) const noexcept -> bool;
			[[nodiscard]] auto target_count() const noexcept -> std::size_t;
		};

		// ==================== 超量素材 ====================

		class XyzHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit XyzHandler(Card& card) noexcept;

			[[nodiscard]] auto xyz_data() noexcept -> Xyz&;
			[[nodiscard]] auto xyz_data() const noexcept -> const Xyz&;

		public:
			// 将一张卡作为超量素材叠放到本卡(返回是否叠放成功)(如果已经是本卡超量素材返回true)
			auto add_overlay(Card& material) noexcept -> bool;
			// 将一张作为本卡超量素材的卡移除(返回是否移除成功)(如果不是本卡超量素材返回true)
			auto remove_overlay(Card& material) noexcept -> bool;

			[[nodiscard]] auto materials() const noexcept -> View;
			[[nodiscard]] auto overlay_target() const noexcept -> CardOptional;

			[[nodiscard]] auto has_material() const noexcept -> bool;
			[[nodiscard]] auto has_material(const Card& card) const noexcept -> bool;
			[[nodiscard]] auto material_count() const noexcept -> std::size_t;
		};

		// ==================== 装备 ====================

		class EquipHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit EquipHandler(Card& card) noexcept;

			[[nodiscard]] auto equip_data() noexcept -> Equip&;
			[[nodiscard]] auto equip_data() const noexcept -> const Equip&;

		public:
			// 将一张卡作为装备卡装备到本卡(返回是否装备成功)(如果已经是本卡装备返回true)
			auto add_equip(Card& equip) noexcept -> bool;
			// 将一张作为本卡装备的卡移除(返回是否移除成功)(如果不是本卡装备返回true)
			auto remove_equip(Card& equip) noexcept -> bool;

			[[nodiscard]] auto equips() const noexcept -> View;
			[[nodiscard]] auto owner() const noexcept -> CardOptional;

			[[nodiscard]] auto has_equip() const noexcept -> bool;
			[[nodiscard]] auto has_equip(const Card& card) const noexcept -> bool;
			[[nodiscard]] auto equip_count() const noexcept -> std::size_t;

			// 是否可以是装备卡
			[[nodiscard]] auto equippable() const noexcept -> bool;
		};

		// ==================== | ====================

		Card(Duel& duel, card_instance_id_type instance_id, const Prototype& prototype) noexcept;

		[[nodiscard]] auto instance_id() const noexcept -> card_instance_id_type;

		[[nodiscard]] auto prototype() noexcept -> PrototypeHandler;
		[[nodiscard]] auto property() noexcept -> PropertyHandler;
		[[nodiscard]] auto state() noexcept -> StateHandler;
		[[nodiscard]] auto summon() noexcept -> SummonHandler;
		[[nodiscard]] auto battle() noexcept -> BattleHandler;
		[[nodiscard]] auto target() noexcept -> TargetHandler;
		[[nodiscard]] auto xyz() noexcept -> XyzHandler;
		[[nodiscard]] auto equip() noexcept -> EquipHandler;
	};
}

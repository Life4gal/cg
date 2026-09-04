#pragma once

#include <core/domain/id.hpp>

#include <core/engine/prototype.hpp>
#include <core/engine/state.hpp>
#include <core/engine/summon_info.hpp>
#include <core/engine/battle_info.hpp>
#include <core/engine/target_info.hpp>
#include <core/engine/xyz_info.hpp>
#include <core/engine/equip_info.hpp>

namespace cg::engine
{
	class Prototype;
	class Duel;

	class Card
	{
		friend State;
		friend SummonInfo;
		friend BattleInfo;
		friend TargetInfo;
		friend XyzInfo;
		friend EquipInfo;

	public:
		//

	private:
		// Duel实例引用
		std::reference_wrapper<Duel> duel_;
		// 卡牌实例ID,由Duel生成
		domain::CardInstanceId instance_id_;
		// 卡牌所有者,与控制者不同
		domain::Player owner_;
		// 卡牌原型
		std::reference_wrapper<const Prototype> prototype_;

		// 卡牌状态
		State state_;
		// 卡牌召唤信息
		SummonInfo summon_;
		// 卡牌战斗信息
		BattleInfo battle_;
		// 卡牌对象信息
		TargetInfo target_;
		// 卡牌超量信息
		XyzInfo xyz_;
		// 卡牌装备信息
		EquipInfo equip_;

	public:
		// ==================== 原型 ====================

		class PrototypeHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit PrototypeHandler(Card& card) noexcept;

			[[nodiscard]] auto prototype() const noexcept -> const Prototype&;

		public:
			[[nodiscard]] auto code() const noexcept -> domain::CardCode;
			[[nodiscard]] auto code_rule() const noexcept -> domain::CardCode;
			[[nodiscard]] auto series() const noexcept -> std::array<domain::SeriesCode, Prototype::max_series_count>;
			[[nodiscard]] auto card_type() const noexcept -> domain::CardTypeWrapper;
			[[nodiscard]] auto attribute() const noexcept -> domain::AttributeWrapper;
			[[nodiscard]] auto race() const noexcept -> domain::Race;
			[[nodiscard]] auto level() const noexcept -> domain::LevelWrapper;
			[[nodiscard]] auto rank() const noexcept -> domain::Rank;
			[[nodiscard]] auto link_marker() const noexcept -> domain::LinkMarkerWrapper;
			[[nodiscard]] auto attack() const noexcept -> domain::attack_defense_value_type;
			[[nodiscard]] auto defense() const noexcept -> domain::attack_defense_value_type;
			[[nodiscard]] auto left_pendulum() const noexcept -> domain::PendulumScale;
			[[nodiscard]] auto right_pendulum() const noexcept -> domain::PendulumScale;

			[[nodiscard]] auto is_card_type(domain::CardTypeWrapper expected_card_type) const noexcept -> bool;
			[[nodiscard]] auto is_card_type(domain::CardType expected_card_type) const noexcept -> bool;
			[[nodiscard]] auto has_card_type(domain::CardTypeWrapper expected_card_type) const noexcept -> bool;
			[[nodiscard]] auto has_card_type(domain::CardType expected_card_type) const noexcept -> bool;
		};

		// ==================== 属性 ====================

		class PropertyHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit PropertyHandler(Card& card) noexcept;

			[[nodiscard]] auto prototype() const noexcept -> PrototypeHandler;

		public:
			[[nodiscard]] auto card_type() const noexcept -> domain::CardTypeWrapper;
			[[nodiscard]] auto attribute() const noexcept -> domain::AttributeWrapper;
			[[nodiscard]] auto race() const noexcept -> domain::Race;
			[[nodiscard]] auto level() const noexcept -> domain::LevelWrapper;
			[[nodiscard]] auto rank() const noexcept -> domain::Rank;
			[[nodiscard]] auto link_marker() const noexcept -> domain::LinkMarkerWrapper;
			[[nodiscard]] auto attack() const noexcept -> domain::attack_defense_value_type;
			[[nodiscard]] auto defense() const noexcept -> domain::attack_defense_value_type;
			[[nodiscard]] auto left_pendulum() const noexcept -> domain::PendulumScale;
			[[nodiscard]] auto right_pendulum() const noexcept -> domain::PendulumScale;

			[[nodiscard]] auto is_card_type(domain::CardTypeWrapper expected_card_type) const noexcept -> bool;
			[[nodiscard]] auto is_card_type(domain::CardType expected_card_type) const noexcept -> bool;
			[[nodiscard]] auto has_card_type(domain::CardTypeWrapper expected_card_type) const noexcept -> bool;
			[[nodiscard]] auto has_card_type(domain::CardType expected_card_type) const noexcept -> bool;
		};

		// ==================== 运行时状态 ====================

		class StateHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit StateHandler(Card& card) noexcept;

			// 传播const
			[[nodiscard]] auto state() noexcept -> State&;
			[[nodiscard]] auto state() const noexcept -> const State&;

		public:
			auto set_controller(domain::Player controller) noexcept -> void;
			auto set_zone(domain::Zone zone) noexcept -> void;
			auto set_reason_player(domain::Player reason_player) noexcept -> void;
			auto set_reason_card(CardOptional reason_card) noexcept -> void;
			auto set_reason_effect(EffectOptional reason_effect) noexcept -> void;
			auto set_reason(domain::ReasonWrapper reason) noexcept -> void;

			[[nodiscard]] auto controller() const noexcept -> domain::Player;
			[[nodiscard]] auto zone() const noexcept -> domain::Zone;
			[[nodiscard]] auto reason_player() const noexcept -> domain::Player;
			[[nodiscard]] auto reason_card() const noexcept -> CardOptional;
			[[nodiscard]] auto reason_effect() const noexcept -> EffectOptional;
			[[nodiscard]] auto reason() const noexcept -> domain::ReasonWrapper;

			[[nodiscard]] auto is_controller(domain::Player expected_player) const noexcept -> bool;
			[[nodiscard]] auto is_zone(domain::Zone expected_zone) const noexcept -> bool;
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

		class SummonInfoHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit SummonInfoHandler(Card& card) noexcept;

			// 传播const
			[[nodiscard]] auto summon_info() noexcept -> SummonInfo&;
			[[nodiscard]] auto summon_info() const noexcept -> const SummonInfo&;

		public:
			auto set_kind(domain::SummonKind kind) noexcept -> void;
			auto set_from_zone(domain::Zone from_zone) noexcept -> void;
			auto set_player(domain::Player player) noexcept -> void;
			auto set_turn_id(domain::TurnId turn_id) noexcept -> void;
			auto set_materials(Group materials) noexcept -> void;

			[[nodiscard]] auto kind() const noexcept -> domain::SummonKind;
			[[nodiscard]] auto from_zone() const noexcept -> domain::Zone;
			[[nodiscard]] auto player() const noexcept -> domain::Player;
			[[nodiscard]] auto turn_id() const noexcept -> domain::TurnId;
			[[nodiscard]] auto materials() const noexcept -> const Group&;

			[[nodiscard]] auto is_kind(domain::SummonKind expected_kind) const noexcept -> bool;
			[[nodiscard]] auto is_from_zone(domain::Zone expected_from_zone) const noexcept -> bool;
			[[nodiscard]] auto is_player(domain::Player expected_player) const noexcept -> bool;
			[[nodiscard]] auto is_turn_id(domain::TurnId expected_turn_id) const noexcept -> bool;
			[[nodiscard]] auto has_material(const Card& card) const noexcept -> bool;
			[[nodiscard]] auto material_count() const noexcept -> std::size_t;
		};

		// ==================== 战斗记录 ====================

		class BattleInfoHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit BattleInfoHandler(Card& card) noexcept;

			// 传播const
			[[nodiscard]] auto battle_info() noexcept -> BattleInfo&;
			[[nodiscard]] auto battle_info() const noexcept -> const BattleInfo&;

		public:
			auto record_attack_announced() noexcept -> void;
			auto record_attack_canceled() noexcept -> void;
			auto record_attacked_card(Card& card) noexcept -> void;

			[[nodiscard]] auto attacked_cards() const noexcept -> const Group&;
			[[nodiscard]] auto attacked_count() const noexcept -> BattleInfo::size_type;
			[[nodiscard]] auto attack_announced_count() const noexcept -> BattleInfo::size_type;
			[[nodiscard]] auto attack_turn_id() const noexcept -> domain::TurnId;
			[[nodiscard]] auto attack_canceled_turn_id() const noexcept -> domain::TurnId;

			[[nodiscard]] auto is_attacked_this_turn() const noexcept -> bool;
			[[nodiscard]] auto is_attack_canceled_this_turn() const noexcept -> bool;
		};

		// ==================== 对象指定 ====================

		class TargetInfoHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit TargetInfoHandler(Card& card) noexcept;

			// 传播const
			[[nodiscard]] auto target_info() noexcept -> TargetInfo&;
			[[nodiscard]] auto target_info() const noexcept -> const TargetInfo&;

		public:
			// 将目标卡指定为本卡的对象
			auto target_to(CardReference target) noexcept -> void;
			// 取消目标卡为本卡的对象
			auto cancel_target(CardReference target) noexcept -> void;

			// 本卡的所有对象
			[[nodiscard]] auto targets_to() const noexcept -> const Group&;
			// 所有以本卡为对象的卡
			[[nodiscard]] auto targets_from() const noexcept -> const Group&;

			// 本卡是否有任何对象
			[[nodiscard]] auto has_target() const noexcept -> bool;
			// 本卡是否有指定对象
			[[nodiscard]] auto has_target(const Card& card) const noexcept -> bool;
			// 本卡的对象数量
			[[nodiscard]] auto target_count() const noexcept -> std::size_t;
		};

		// ==================== 超量素材 ====================

		class XyzInfoHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit XyzInfoHandler(Card& card) noexcept;

			// 传播const
			[[nodiscard]] auto xyz_info() noexcept -> XyzInfo&;
			[[nodiscard]] auto xyz_info() const noexcept -> const XyzInfo&;

		public:
			// 将一张卡作为超量素材叠放到本卡 -- 不处理目标卡的超量素材
			auto take(CardReference material) noexcept -> void;
			// 将一张作为本卡超量素材的卡移除
			auto remove(CardReference material) noexcept -> void;
			// 将本卡的所有的超量素材移除
			auto remove_all() noexcept -> void;
			// 本卡是否可以是超量素材
			[[nodiscard]] auto can_overlay() const noexcept -> bool;

			// 当前叠放目标 -- 如果当前是超量素材
			[[nodiscard]] auto overlay_target() const noexcept -> CardOptional;
			// 当前叠放的超量素材 -- 当前必须不是超量素材,超量素材不能叠放超量素材
			[[nodiscard]] auto materials() const noexcept -> const Sequence&;

			// 本卡是否有任何超量素材
			[[nodiscard]] auto has_material() const noexcept -> bool;
			// 本卡是否有指定超量素材
			[[nodiscard]] auto has_material(CardReference material) const noexcept -> bool;
			// 本卡的超量素材数量
			[[nodiscard]] auto material_count() const noexcept -> std::size_t;
		};

		// ==================== 装备 ====================

		class EquipInfoHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit EquipInfoHandler(Card& card) noexcept;

			// 传播const
			[[nodiscard]] auto equip_info() noexcept -> EquipInfo&;
			[[nodiscard]] auto equip_info() const noexcept -> const EquipInfo&;

		public:
			// 将一张卡作为装备卡装备到本卡
			auto equip(CardReference equip) noexcept -> void;
			// 将一张本卡的装备卡移除
			auto unequip(CardReference equip) noexcept -> void;
			// 将本卡的所有装备卡移除
			auto unequip_all() noexcept -> void;
			// 本卡是否可以是装备卡
			[[nodiscard]] auto can_equip() const noexcept -> bool;

			// 当前装备的目标 -- 如果当前卡是装备卡
			[[nodiscard]] auto target() const noexcept -> CardOptional;
			// 当前装备的装备卡牌 -- 当前必须不是装备卡,装备卡不能装备装备卡
			[[nodiscard]] auto equips() const noexcept -> const Group&;

			// 本卡是否有任何装备卡
			[[nodiscard]] auto has_equip() const noexcept -> bool;
			// 本卡是否有指定装备卡
			[[nodiscard]] auto has_equip(CardReference equip) const noexcept -> bool;
			// 本卡的装备卡数量
			[[nodiscard]] auto equip_count() const noexcept -> std::size_t;
		};

		// ==================== | ====================

		Card(Duel& duel, domain::CardInstanceId instance_id, domain::Player owner, const Prototype& prototype) noexcept;

		[[nodiscard]] auto instance_id() const noexcept -> domain::CardInstanceId;

		[[nodiscard]] auto prototype() noexcept -> PrototypeHandler;
		[[nodiscard]] auto property() noexcept -> PropertyHandler;
		[[nodiscard]] auto state() noexcept -> StateHandler;
		[[nodiscard]] auto summon() noexcept -> SummonInfoHandler;
		[[nodiscard]] auto battle() noexcept -> BattleInfoHandler;
		[[nodiscard]] auto target() noexcept -> TargetInfoHandler;
		[[nodiscard]] auto xyz() noexcept -> XyzInfoHandler;
		[[nodiscard]] auto equip() noexcept -> EquipInfoHandler;
	};
}

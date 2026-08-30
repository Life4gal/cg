#pragma once

#include <core/domain/summon.hpp>

#include <core/engine/card_state.hpp>
#include <core/engine/turn_state.hpp>
#include <core/engine/collection.hpp>

namespace cg::engine
{
	class Card
	{
	public:
		class StateHandler;
		class SummonInfoHandler;
		class BattleInfoHandler;
		class TargetInfoHandler;

	private:
		class SummonInfo
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

		class BattleInfo
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

		class TargetInfo
		{
		public:
			// 本卡指定为对象的卡
			Group card_targets;
			// 以本卡为对象的卡
			Group owner_targets;
		};

		card_instance_id_type instance_id_;

		CardState state_;
		SummonInfo summon_info_;
		BattleInfo battle_info_;
		TargetInfo target_info_;

	public:
		// ==================== 运行时状态 ====================

		class StateHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit StateHandler(Card& card) noexcept;

			[[nodiscard]] auto state() noexcept -> CardState&;
			[[nodiscard]] auto state() const noexcept -> const CardState&;

		public:
			auto set_controller(domain::Player controller) noexcept -> void;
			auto set_zone(domain::Zone zone) noexcept -> void;
			auto set_form(domain::FieldZoneForm form) noexcept -> void;
			auto set_reason_player(domain::Player player) noexcept -> void;
			auto set_reason_card(CardOptional card) noexcept -> void;
			auto set_reason_effect(EffectOptional effect) noexcept -> void;
			auto set_reason(domain::ReasonWrapper reason) noexcept -> void;

			[[nodiscard]] auto controller() const noexcept -> domain::Player;
			[[nodiscard]] auto zone() const noexcept -> domain::Zone;
			[[nodiscard]] auto form() const noexcept -> domain::FieldZoneForm;
			[[nodiscard]] auto reason_player() const noexcept -> domain::Player;
			[[nodiscard]] auto reason_card() const noexcept -> CardOptional;
			[[nodiscard]] auto reason_effect() const noexcept -> EffectOptional;
			[[nodiscard]] auto reason() const noexcept -> domain::ReasonWrapper;

			[[nodiscard]] auto is_controller(domain::Player expected_player) const noexcept -> bool;
			[[nodiscard]] auto is_zone(domain::Zone expected_zone) const noexcept -> bool;
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

		class SummonInfoHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit SummonInfoHandler(Card& card) noexcept;

			[[nodiscard]] auto summon_info() noexcept -> SummonInfo&;
			[[nodiscard]] auto summon_info() const noexcept -> const SummonInfo&;

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
			[[nodiscard]] auto materials() const noexcept -> GroupView;

			[[nodiscard]] auto is_kind(domain::SummonKind expected_kind) const noexcept -> bool;
			[[nodiscard]] auto is_from_zone(domain::Zone expected_from_zone) const noexcept -> bool;
			[[nodiscard]] auto is_player(domain::Player expected_player) const noexcept -> bool;
			[[nodiscard]] auto is_turn_index(turn_index_type expected_turn_index) const noexcept -> bool;
			[[nodiscard]] auto has_material(const Card& card) const noexcept -> bool;
			[[nodiscard]] auto material_count() const noexcept -> std::size_t;
		};

		// ==================== 战斗记录 ====================

		class BattleInfoHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit BattleInfoHandler(Card& card) noexcept;

			[[nodiscard]] auto battle_info() noexcept -> BattleInfo&;
			[[nodiscard]] auto battle_info() const noexcept -> const BattleInfo&;

		public:
			auto record_attack_announced() noexcept -> void;
			auto record_attack_canceled() noexcept -> void;
			auto record_attacked_card(Card& card) noexcept -> void;

			[[nodiscard]] auto attacked_cards() const noexcept -> GroupView;
			[[nodiscard]] auto attacked_count() const noexcept -> std::size_t;
			[[nodiscard]] auto attack_announced_count() const noexcept -> std::size_t;
			[[nodiscard]] auto attack_turn_index() const noexcept -> turn_index_type;
			[[nodiscard]] auto attack_canceled_turn_index() const noexcept -> turn_index_type;

			[[nodiscard]] auto is_attacked_this_turn() const noexcept -> bool;
			[[nodiscard]] auto is_attack_canceled_this_turn() const noexcept -> bool;
		};

		// ==================== 对象指定 ====================

		class TargetInfoHandler
		{
			friend Card;

			std::reference_wrapper<Card> card_;

			explicit TargetInfoHandler(Card& card) noexcept;

			[[nodiscard]] auto target_info() noexcept -> TargetInfo&;
			[[nodiscard]] auto target_info() const noexcept -> const TargetInfo&;

			auto set_owner_target(Card& card) noexcept -> bool;
			auto cancel_owner_target(const Card& card) noexcept -> bool;

		public:
			auto set_target(Card& card) noexcept -> bool;
			auto cancel_target(Card& card) noexcept -> bool;

			[[nodiscard]] auto card_targets() const noexcept -> GroupView;
			[[nodiscard]] auto owner_targets() const noexcept -> GroupView;

			[[nodiscard]] auto has_target() const noexcept -> bool;
			[[nodiscard]] auto has_target(const Card& card) const noexcept -> bool;
			[[nodiscard]] auto target_count() const noexcept -> std::size_t;
		};

		[[nodiscard]] auto instance_id() const noexcept -> card_instance_id_type;

		[[nodiscard]] auto state() noexcept -> StateHandler;
		[[nodiscard]] auto summon_info() noexcept -> SummonInfoHandler;
		[[nodiscard]] auto battle_info() noexcept -> BattleInfoHandler;
		[[nodiscard]] auto target_info() noexcept -> TargetInfoHandler;
	};
}

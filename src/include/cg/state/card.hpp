#pragma once

#include <vector>

#include <cg/domain/player.hpp>
#include <cg/domain/phase.hpp>
#include <cg/domain/field.hpp>
#include <cg/domain/reason.hpp>
#include <cg/domain/summon.hpp>

#include <cg/dsl/id.hpp>
#include <cg/dsl/identity.hpp>

#include <cg/state/id.hpp>

namespace cg::state
{
	class Game;

	// 卡牌实例
	class CardInstance
	{
	public:
		class MoveProvenance
		{
		public:
			// 移动前的控制者
			domain::Player controller;
			// 移动原因
			utility::EnumMask<domain::Reason> reason;
			// 移动发生的回合
			domain::TurnId turn_id;
			// 移动前位置
			domain::Placement placement;
			// 导致移动的效果
			dsl::EffectId effect;
		};

		class SummonRecord
		{
		public:
			// 当前控制者
			domain::Player controller;
			// 召唤方式
			domain::SummonMethod method;
			// 召唤发生的回合
			domain::TurnId turn_id;
			// 召唤前的位置
			domain::Placement from;
			// 召唤使用的素材
			std::vector<CardInstanceId> materials;
		};

		class Modifier
		{
			friend Game;

			std::reference_wrapper<CardInstance> instance_;

			explicit Modifier(CardInstance& instance) noexcept;

			[[nodiscard]] auto ref() noexcept -> CardInstance&;

			// 设置卡牌控制者
			auto set_controller(domain::Player controller) noexcept -> void;

			// 设置卡牌当前位置
			auto set_placement(const domain::Placement& placement) noexcept -> void;

			// 添加超量素材
			auto add_overlay(const CardInstance& material) noexcept -> void;

			// 移除超量素材
			auto remove_overlay(const CardInstance& material) noexcept -> bool;

			// 禁止在指定回合攻击
			auto prevent_attack(domain::TurnId turn_id) noexcept -> void;

			// 记录本回合已宣言攻击
			auto mark_attacked(domain::TurnId turn_id) noexcept -> void;

			// 设置移动来源
			auto set_move_provenance(const MoveProvenance& provenance) noexcept -> void;

			// 设置召唤记录
			auto set_summon_record(SummonRecord record) noexcept -> void;
		};

	private:
		// 卡牌定义的引用
		std::reference_wrapper<const dsl::CardIdentity> definition_;

		// 实例ID
		CardInstanceId id_;
		// 拥有者
		domain::Player owner_;
		// 控制者
		domain::Player controller_;
		// 当前位置
		domain::Placement placement_;

		// 当前持有的超量素材
		std::vector<CardInstanceId> overlays_;

		// 表侧存在周期:离场或翻里侧时递增
		// 用于 dsl::UsageKind::ONCE_PER_CARD 的限制;控制权转移不改变该值
		// 类型为 flow::Duel::UsageOncePerCard::generation_type
		std::uint32_t face_up_generation_;

		// 是否曾经完成正规召唤
		bool properly_summoned_;
		// 是否为衍生物
		bool token_;
		// 禁止攻击的回合(离场或重新召唤时清除)
		std::optional<domain::TurnId> cannot_attack_turn_;
		// 本回合已宣言攻击
		std::optional<domain::TurnId> attacked_turn_;

		// 最近一次移动前的快照
		MoveProvenance move_;
		// 最近一次召唤的记录
		SummonRecord summon_;

	public:
		CardInstance(CardInstanceId id, domain::Player owner, const dsl::CardIdentity& definition) noexcept;

		// ================================================================
		// 实例信息
		// ================================================================

		[[nodiscard]] auto id() const noexcept -> CardInstanceId;

		// 卡牌拥有者
		[[nodiscard]] auto owner() const noexcept -> domain::Player;

		// 卡牌控制者
		[[nodiscard]] auto controller() const noexcept -> domain::Player;

		// 卡牌当前位置
		[[nodiscard]] auto placement() const noexcept -> domain::Placement;

		// ================================================================
		// 原本数据(印刷值)
		// ================================================================

		// 卡牌卡密
		[[nodiscard]] auto origin_code() const noexcept -> domain::CardCode;

		// 卡牌类型
		[[nodiscard]] auto origin_kind() const noexcept -> domain::CardType;

		// 是否是指定卡类型
		[[nodiscard]] auto origin_is_card_type(domain::CardType type) const noexcept -> bool;
		[[nodiscard]] auto origin_is_monster_card() const noexcept -> bool;
		[[nodiscard]] auto origin_is_spell_card() const noexcept -> bool;
		[[nodiscard]] auto origin_is_trap_card() const noexcept -> bool;

		// 卡牌子类型
		[[nodiscard]] auto origin_card_subtypes() const noexcept -> utility::EnumMask<domain::CardSubtype>;

		// 是否是指定卡子类型
		[[nodiscard]] auto origin_is_card_subtype(domain::CardSubtype subtype) const noexcept -> bool;
		[[nodiscard]] auto origin_is_card_subtypes(utility::EnumMask<domain::CardSubtype> subtypes) const noexcept -> bool;

		// 属性 -- 假定是怪兽卡
		[[nodiscard]] auto origin_attribute() const noexcept -> domain::Attribute;
		[[nodiscard]] auto origin_is_attribute(domain::Attribute attribute) const noexcept -> bool;

		// 种族 -- 假定是怪兽卡
		[[nodiscard]] auto origin_race() const noexcept -> domain::Race;
		[[nodiscard]] auto origin_is_race(domain::Race race) const noexcept -> bool;

		// 等级/阶级 -- 假定是怪兽卡
		[[nodiscard]] auto origin_level() const noexcept -> std::underlying_type_t<domain::LevelRank>;
		[[nodiscard]] auto origin_is_level(std::underlying_type_t<domain::LevelRank> level) const noexcept -> bool;

		// 攻击力 -- 假定是怪兽卡
		[[nodiscard]] auto origin_attack() const noexcept -> std::underlying_type_t<domain::AttackDefense>;
		[[nodiscard]] auto origin_is_attack(std::underlying_type_t<domain::AttackDefense> attack) const noexcept -> bool;

		// 守备力 -- 假定是怪兽卡
		[[nodiscard]] auto origin_defense() const noexcept -> std::underlying_type_t<domain::AttackDefense>;
		[[nodiscard]] auto origin_is_defense(std::underlying_type_t<domain::AttackDefense> defense) const noexcept -> bool;

		// 灵摆刻度 -- 假定是灵摆怪兽
		[[nodiscard]] auto origin_pendulum_scale() const noexcept -> dsl::CardIdentity::Monster::PendulumScale;
		[[nodiscard]] auto origin_has_pendulum_scale() const noexcept -> bool;
		[[nodiscard]] auto origin_is_pendulum_scale(domain::PendulumSide side, std::underlying_type_t<domain::PendulumScale> scale) const noexcept -> bool;

		// 连接标记 -- 假定是连接怪兽
		[[nodiscard]] auto origin_link_marker() const noexcept -> utility::EnumMask<domain::LinkMarker>;
		[[nodiscard]] auto origin_has_link_marker(domain::LinkMarker mark) const noexcept -> bool;
		[[nodiscard]] auto origin_has_link_marker(utility::EnumMask<domain::LinkMarker> marks) const noexcept -> bool;

		// 是否属于指定系列/字段
		[[nodiscard]] auto origin_has_archetype(dsl::ArchetypeId archetype) const noexcept -> bool;

		// ================================================================
		// 历史/素材
		// ================================================================

		[[nodiscard]] auto overlays() const noexcept -> const std::vector<CardInstanceId>&;

		// 当前表侧存在周期(离场/翻里侧后递增)
		[[nodiscard]] auto face_up_generation() const noexcept -> std::uint32_t;

		[[nodiscard]] auto properly_summoned() const noexcept -> bool;

		// 是否为衍生物
		[[nodiscard]] auto is_token() const noexcept -> bool;

		// 是否因当前回合的持续效果不能攻击
		[[nodiscard]] auto cannot_attack(domain::TurnId turn_id) const noexcept -> bool;

		// 本回合是否已经宣言攻击
		[[nodiscard]] auto has_attacked(domain::TurnId turn_id) const noexcept -> bool;

		[[nodiscard]] auto move_provenance() const noexcept -> const MoveProvenance&;

		[[nodiscard]] auto summon_record() const noexcept -> const SummonRecord&;
	};
}

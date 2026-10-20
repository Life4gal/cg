#pragma once

#include <vector>

#include <cg/domain/player.hpp>
#include <cg/domain/field.hpp>
#include <cg/domain/reason.hpp>
#include <cg/domain/summon.hpp>

#include <cg/dsl/id.hpp>

#include <cg/state/id.hpp>
#include <cg/state/card_definition.hpp>

namespace cg::state
{
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
			// 移动前位置
			domain::Placement placement;
			// 导致移动的效果
			dsl::EffectId effect;
		};

		class SummonRecord
		{
		public:
			// 发起召唤的玩家
			domain::Player player;
			// 召唤方式
			domain::SummonMethod method;
			// 召唤前的位置
			domain::Placement from;
			// 召唤使用的素材
			std::vector<CardInstanceId> materials;
		};

		class Form
		{
		public:
			class Monster
			{
			public:
				// 属性
				domain::Attribute attribute;
				// 种族
				domain::Race race;
				// 等级/阶级
				std::underlying_type_t<domain::LevelRank> level;
				// 攻击力
				std::underlying_type_t<domain::AttackDefense> attack;
				// 守备力
				std::underlying_type_t<domain::AttackDefense> defense;
			};

			// 视为怪兽 -- 如魔陷被视为怪兽
			std::optional<Monster> monster;
			// 视为永续魔法 -- 如怪兽被视为永续魔法
			bool continuous_spell;
			// 是否为衍生物
			bool is_token;
			//
		};

	private:
		// 卡牌定义的引用
		std::reference_wrapper<const CardDefinition> definition_;

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

		// 最近一次移动前的快照
		MoveProvenance move_;
		// 最近一次召唤的记录
		SummonRecord summon_;
		// 当前形态
		Form form_;

	public:
		CardInstance(CardInstanceId id, const CardDefinition& definition) noexcept;

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
		// 定义查询
		// ================================================================

		// 卡牌卡密
		[[nodiscard]] auto code() const noexcept -> domain::CardCode;

		// 是否是怪兽卡
		[[nodiscard]] auto is_monster_card() const noexcept -> bool;

		// 是否是魔法卡
		[[nodiscard]] auto is_spell_card() const noexcept -> bool;

		// 是否是陷阱卡
		[[nodiscard]] auto is_trap_card() const noexcept -> bool;

		// ================================================================
		// 
		// ================================================================
	};
}

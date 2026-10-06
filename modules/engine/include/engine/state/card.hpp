#pragma once

#include <vector>
#include <map>
#include <optional>

#include <domain/player.hpp>
#include <domain/zone.hpp>
#include <domain/card.hpp>
#include <domain/event.hpp>
#include <domain/summon.hpp>

#include <engine/state/id.hpp>
#include <engine/state/zone.hpp>
#include <engine/summon/spec.hpp>

namespace cg::engine
{
	// 卡牌定义
	class CardDefinition
	{
	public:
		// 怪兽数据
		class MonsterStats
		{
		public:
			// 灵摆刻度
			class PendulumScale
			{
			public:
				std::underlying_type_t<domain::PendulumScale> left;
				std::underlying_type_t<domain::PendulumScale> right;
			};

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

			// 灵摆刻度 -- 如果是怪兽卡且是灵摆怪兽
			std::optional<PendulumScale> pendulum_scale;
			// 连接箭头 -- 如果是怪兽卡且是连接怪兽
			std::optional<domain::LinkMarker> link_marker;

			// 召唤手续
			std::vector<SummonInfo> summons;
		};

		// 卡密
		CardCode code;
		// 主类型(怪兽/魔法/陷阱)
		domain::CardType kind;
		// 子类型
		domain::CardType subtypes;
		// 怪兽数据 -- 如果是怪兽卡
		std::optional<MonsterStats> monster;

		// 卡牌卡密
		[[nodiscard]] auto code_of() const noexcept -> CardCode;

		// 是否是怪兽卡
		[[nodiscard]] auto is_monster_card() const noexcept -> bool;

		// 是否是魔法卡
		[[nodiscard]] auto is_spell_card() const noexcept -> bool;

		// 是否是陷阱卡
		[[nodiscard]] auto is_trap_card() const noexcept -> bool;

		// 是否拥有指定子类型
		[[nodiscard]] auto has_subtype(domain::CardType subtype) const noexcept -> bool;

		// 是否是额外卡组怪兽
		// - 是怪兽
		// - 是融合/同调/超量/连接怪兽
		[[nodiscard]] auto is_extra_deck_monster_card() const noexcept -> bool;
	};

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
			domain::Reason reason;
			// 移动前位置
			domain::Position position;
			// 导致移动的效果
			EffectId effect;
		};

		class SummonRecord
		{
		public:
			// 发起召唤的玩家
			domain::Player player;
			// 召唤方式
			domain::SummonMethod method;
			// 召唤前的位置
			domain::Position from;
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

		// 卡牌定义的引用
		std::reference_wrapper<const CardDefinition> definition;

		// 实例ID
		CardInstanceId id;
		// 拥有者
		domain::Player owner;
		// 控制者
		domain::Player controller;
		// 当前位置
		domain::Position position;

		// 当前持有的超量素材
		std::vector<CardInstanceId> overlays;
		// 当前持有的指示物
		std::map<domain::Counter, std::uint8_t> counters;

		// 最近一次移动前的快照
		MoveProvenance move;
		// 最近一次召唤的记录
		SummonRecord summon;
		// 当前形态
		Form form;

		// 是否是怪兽卡
		[[nodiscard]] auto is_monster_card()const noexcept -> bool;

		// 是否是魔法卡
		[[nodiscard]] auto is_spell_card() const noexcept -> bool;

		// 是否是陷阱卡
		[[nodiscard]] auto is_trap_card() const noexcept -> bool;

		// 卡牌卡密
		[[nodiscard]] auto code_of() const noexcept -> CardCode;

		// 卡牌拥有者
		[[nodiscard]] auto owner_of() const noexcept -> domain::Player;

		// 卡牌控制者
		[[nodiscard]] auto controller_of() const noexcept -> domain::Player;

		// 卡牌当前位置
		[[nodiscard]] auto position_of() const noexcept -> domain::Position;

	};

	// ==================================================================================
	// 卡牌实例ID
	// ==================================================================================

	// 是否是怪兽卡
	// 卡牌实例不存在返回null-opt
	[[nodiscard]] auto is_monster_card(const GameState& state, CardInstance card) noexcept -> std::optional<bool>;

	// 是否是魔法卡
	// 卡牌实例不存在返回null-opt
	[[nodiscard]] auto is_spell_card(const GameState& state, CardInstance card) noexcept -> std::optional<bool>;

	// 是否是陷阱卡
	// 卡牌实例不存在返回null-opt
	[[nodiscard]] auto is_trap_card(const GameState& state, CardInstance card) noexcept -> std::optional<bool>;

	// 卡牌卡密
	// 卡牌实例不存在返回null-opt
	[[nodiscard]] auto code_of(const GameState& state, CardInstanceId card) noexcept -> std::optional<CardCode>;

	// 卡牌拥有者
	// 卡牌实例不存在返回null-opt
	[[nodiscard]] auto owner_of(const GameState& state, CardInstanceId card) noexcept -> std::optional<domain::Player>;

	// 卡牌控制者
	// 卡牌实例不存在返回null-opt
	[[nodiscard]] auto controller_of(const GameState& state, CardInstanceId card) noexcept -> std::optional<domain::Player>;

	// 卡牌当前位置
	// 卡牌实例不存在返回null-opt
	[[nodiscard]] auto position_of(const GameState& state, CardInstanceId card) noexcept -> std::optional<domain::Position>;
}

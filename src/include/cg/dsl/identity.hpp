#pragma once

#include <optional>
#include <vector>

#include <cg/utility/enum.hpp>

#include <cg/domain/card.hpp>

#include <cg/dsl/id.hpp>

namespace cg::dsl
{
	// 卡牌定义
	class CardIdentity
	{
	public:
		// 怪兽卡数据
		class Monster
		{
		public:
			class PendulumScale
			{
			public:
				domain::PendulumScale left;
				domain::PendulumScale right;

				[[nodiscard]] constexpr auto operator==(const PendulumScale&) const -> bool = default;
			};

			// 属性
			domain::Attribute attribute;
			// 种族
			domain::Race race;
			// 等级/阶级
			domain::LevelRank level;
			// 攻击力
			domain::AttackDefense attack;
			// 守备力
			domain::AttackDefense defense;

			// 灵摆刻度 -- 如果是怪兽卡且是灵摆怪兽
			std::optional<PendulumScale> pendulum_scale;
			// 连接箭头 -- 如果是怪兽卡且是连接怪兽
			std::optional<utility::EnumMask<domain::LinkMarker>> link_marker;

			[[nodiscard]] constexpr auto operator==(const Monster&) const -> bool = default;
		};

		// 魔法卡数据
		class Spell
		{
		public:
			[[nodiscard]] constexpr auto operator==(const Spell&) const -> bool = default;
		};

		// 陷阱卡数据
		class Trap
		{
		public:
			[[nodiscard]] constexpr auto operator==(const Trap&) const -> bool = default;
		};

		// 卡密
		// TODO: 目前的设计是卡牌脚本文件名即卡密,这意味着解析脚本内容时我们不知道卡牌的卡密,这个值必须由外部设置
		domain::CardCode code;
		// 主类型(怪兽/魔法/陷阱)
		domain::CardType kind;
		// 子类型
		utility::EnumMask<domain::CardSubtype> subtypes;

		// 怪兽数据 -- 如果是怪兽卡
		std::optional<Monster> monster;
		// 魔法卡数据 -- 如果是魔法卡
		std::optional<Spell> spell;
		// 陷阱卡数据 -- 如果是陷阱卡
		std::optional<Trap> trap;

		// 系列/字段
		std::vector<ArchetypeId> archetypes;
		// 追加卡密
		std::vector<domain::CardCode> code_list;

		[[nodiscard]] constexpr auto operator==(const CardIdentity&) const -> bool = default;
	};
}

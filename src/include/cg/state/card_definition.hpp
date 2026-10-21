#pragma once

#include <cg/utility/enum.hpp>

#include <cg/domain/card.hpp>

namespace cg::state
{
	// 卡牌定义
	class CardDefinition
	{
	public:
		// 怪兽卡数据
		class Monster
		{
		public:
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
		};

		// 魔法卡数据
		class Spell
		{
		public:
			//
		};

		// 陷阱卡数据
		class Trap
		{
		public:
			//
		};

		// 卡密
		domain::CardCode code;
		// 主类型(怪兽/魔法/陷阱)
		// 实卡目前仅存在"陷阱怪兽"一种复合类型
		// TODO: 实际上卡定义不需要EnumMask,一定只有一种类型,复合类型由效果产生
		utility::EnumMask<domain::CardType> kind;
		// 子类型
		utility::EnumMask<domain::CardSubtype> subtypes;

		// 怪兽数据 -- 如果是怪兽卡
		std::optional<Monster> monster;
		// 魔法卡数据 -- 如果是魔法卡
		std::optional<Spell> spell;
		// 陷阱卡数据 -- 如果是陷阱卡
		std::optional<Trap> trap;
	};
}

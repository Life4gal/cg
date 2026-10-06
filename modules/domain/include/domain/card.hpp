#pragma once

#include <utility/enum.hpp>

namespace cg::domain
{
	// 卡牌类型
	enum class CardType : std::uint32_t
	{
		// 怪兽/魔法/陷阱 [0~3]
		MONSTER = 1 << 0,
		SPELL = 1 << 1,
		TRAP = 1 << 2,

		// 怪兽-召唤&效果 [4~7]

		// -反转召唤
		FLIP_SUMMON = 1 << 4,
		// -特殊召唤
		SPECIAL_SUMMON = 1 << 5,
		// -通常怪兽
		NORMAL = 1 << 6,
		// -效果怪兽
		EFFECT = 1 << 7,

		// 怪兽-类型 [8~15]

		// -融合怪兽
		FUSION = 1 << 8,
		// -仪式怪兽
		RITUAL = 1 << 9,
		// -同调怪兽
		SYNCHRO = 1 << 10,
		// -超量怪兽
		XYZ = 1 << 11,
		// -灵摆怪兽
		PENDULUM = 1 << 12,
		// -连接怪兽
		LINK = 1 << 13,
		// -陷阱怪兽
		TRAP_MONSTER = 1 << 14,
		// -衍生怪兽
		TOKEN_MONSTER = 1 << 15,

		// 怪兽-能力 [16~23]

		// -灵魂怪兽
		SPIRIT = 1 << 16,
		// -联合怪兽
		UNION = 1 << 17,
		// -二重怪兽
		DUAL = 1 << 18,
		// -调整怪兽
		TUNER = 1 << 19,
		// -卡通怪兽
		CARTOON = 1 << 20,

		// 怪兽/魔法/陷阱-效果类型 [24~31]

		// -永续
		CONTINUOUS = 1 << 24,
		// -速攻
		QUICK_PLAY = 1 << 25,
		// -装备
		EQUIP = 1 << 26,
		// -场地
		FIELD = 1 << 27,
		// -反击
		COUNTER = 1 << 28,
	};

	// 卡牌属性(怪兽卡)
	enum class Attribute : std::uint8_t
	{
		// 地属性
		EARTH = 1 << 0,
		// 水属性
		WATER = 1 << 1,
		// 火属性
		FIRE = 1 << 2,
		// 风属性
		WIND = 1 << 3,
		// 光属性
		LIGHT = 1 << 4,
		// 暗属性
		DARK = 1 << 5,
		// 神属性
		DIVINE = 1 << 6,
	};

	// 卡牌种族(怪兽卡)
	enum class Race : std::uint8_t
	{
		// 战士族
		WARRIOR,
		// 魔法师族
		SPELLCASTER,
		// 龙族
		DRAGON,
		// 天使族
		FAIRY,
		// 恶魔族
		FIEND,
		// 不死族
		ZOMBIE,
		// 机械族
		MACHINE,
		// 水族
		AQUA,
		// 炎族
		PYRO,
		// 岩石族
		ROCK,
		// 鸟兽族
		WINGED_BEAST,
		// 植物族
		PLANT,
		// 昆虫族
		INSECT,
		// 雷族
		THUNDER,
		// 兽族
		BEAST,
		// 兽战士族
		BEAST_WARRIOR,
		// 恐龙族
		DINOSAUR,
		// 鱼族
		FISH,
		// 海龙族
		SEA_SERPENT,
		// 爬虫类族
		REPTILE,
		// 念动力族
		PSYCHIC,
		// 幻神兽族
		DIVINE_BEAST,
		// 电子界族
		CYBERSE,
		// 幻龙族
		WYRM,
		// 幻想魔族
		ILLUSION,
	};

	// 卡牌攻击力/守备力(怪兽卡)
	// 不会为负数,但是使用有符号类型方便计算
	enum class AttackDefense : std::int16_t {};

	// 卡牌等级/阶级(怪兽卡)
	enum class LevelRank : std::uint16_t {};

	// 卡牌灵摆刻度
	enum class PendulumScale : std::uint8_t {};

	// 卡牌连接箭头(怪兽卡)
	enum class LinkMarker : std::uint8_t
	{
		// 上
		TOP = 1 << 0,
		// 右上
		TOP_RIGHT = 1 << 1,
		// 右
		RIGHT = 1 << 2,
		// 右下
		BOTTOM_RIGHT = 1 << 3,
		// 下
		BOTTOM = 1 << 4,
		// 左下
		BOTTOM_LEFT = 1 << 5,
		// 左
		LEFT = 1 << 6,
		// 左上
		TOP_LEFT = 1 << 7,
	};

	// 卡牌属性(怪兽卡)
	enum class Statistic : std::uint8_t
	{
		// 攻击力
		ATTACK,
		// 守备力
		DEFENSE,
		// 等级
		LEVEL,
		// 阶级
		RANK,
		// 灵摆刻度-左
		PENDULUM_SCALE_LEFT,
		// 灵摆刻度-右
		PENDULUM_SCALE_RIGHT,
		// 连接箭头
		LINK_MARKER,
	};

	// 卡牌状态(怪兽卡)
	enum class Status : std::uint8_t
	{
		// 被战斗破坏
		BATTLE_DESTROYED,
		// 表示形式变更过
		FORM_CHANGED,
		// 被正规手续召唤
		PROPERLY_SUMMONED,
	};

	// 指示物
	// = 对应字段名?
	enum class Counter : std::uint32_t {};
}

namespace cg::utility
{
	template<>
	struct is_flag<domain::CardType> : std::true_type {};

	template<>
	struct is_flag<domain::Attribute> : std::true_type {};

	template<>
	struct is_flag<domain::LinkMarker> : std::true_type {};
}

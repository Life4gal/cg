#pragma once

#include <cstdint>

namespace cg::domain
{
	// 卡密(8位数字)
	enum class CardCode : std::uint32_t {};

	// 卡牌主类型
	enum class CardType : std::uint8_t
	{
		MONSTER,
		SPELL,
		TRAP,
	};

	// 卡牌子类型
	enum class CardSubtype : std::uint8_t
	{
		// 怪兽-召唤&效果

		// -翻转
		FLIP,
		// -特殊召唤
		SPECIAL_SUMMON,
		// -通常怪兽
		NORMAL,
		// -效果怪兽
		EFFECT,

		// 怪兽-类型

		// -融合怪兽
		FUSION,
		// -仪式怪兽
		RITUAL,
		// -同调怪兽
		SYNCHRO,
		// -超量怪兽
		XYZ,
		// -灵摆怪兽
		PENDULUM,
		// -连接怪兽
		LINK,
		// -陷阱怪兽
		TRAP_MONSTER,
		// -衍生怪兽
		TOKEN_MONSTER,

		// 怪兽-能力 

		// -灵魂怪兽
		SPIRIT,
		// -联合怪兽
		UNION,
		// -二重怪兽
		GEMINI,
		// -调整怪兽
		TUNER,
		// -卡通怪兽
		// ReSharper disable once IdentifierTypo
		TOON,

		// 怪兽/魔法/陷阱-效果类型

		// -永续
		CONTINUOUS,
		// -速攻
		QUICK_PLAY,
		// -装备
		EQUIP,
		// -场地
		FIELD,
		// -反击
		COUNTER,
	};

	// 卡牌属性(怪兽卡)
	enum class Attribute : std::uint8_t
	{
		// 地属性
		EARTH,
		// 水属性
		WATER,
		// 火属性
		FIRE,
		// 风属性
		WIND,
		// 光属性
		LIGHT,
		// 暗属性
		DARK,
		// 神属性
		DIVINE,
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

	// 灵摆刻度侧
	enum class PendulumSide : std::uint8_t
	{
		LEFT,
		RIGHT,
	};

	// 卡牌连接箭头(怪兽卡)
	enum class LinkMarker : std::uint8_t
	{
		// 上
		TOP,
		// 右上
		TOP_RIGHT,
		// 右
		RIGHT,
		// 右下
		BOTTOM_RIGHT,
		// 下
		BOTTOM,
		// 左下
		BOTTOM_LEFT,
		// 左
		LEFT,
		// 左上
		TOP_LEFT,
	};

	// 数值项
	enum class NumericProperty : std::uint8_t
	{
		// 攻击力
		ATTACK,
		// 守备力
		DEFENSE,
		// 等级 / 阶级
		LEVEL,
		// 灵摆刻度-左
		SCALE_LEFT,
		// 灵摆刻度-右
		SCALE_RIGHT,
	};
}

#pragma once

#include <utility>

namespace cg
{
	// ==================================================
	// 怪兽卡
	// ==================================================

	// 种类
	enum class MonsterCategory : std::uint8_t
	{
		NONE = 0,

		// ====================================

		// 通常怪兽
		NORMAL = 1 << 0,
		// 效果怪兽
		EFFECT = 1 << 1,

		// ====================================

		// 仪式怪兽(主卡组)
		RITUAL = 1 << 2,
		// 融合怪兽(额外卡组)
		FUSION = 1 << 3,
		// 同调怪兽(额外卡组)
		SYNCHRO = 1 << 4,
		// 超量怪兽(额外卡组)
		XYZ = 1 << 5,
		// 链接怪兽(额外卡组)
		LINK = 1 << 6,
		// 灵摆怪兽(主卡组或额外卡组)
		PENDULUM = 1 << 7,

		// ====================================
	};

	// 能力
	enum class MonsterAbility : std::uint8_t
	{
		NONE = 0,

		// ====================================

		// 调整(同调召唤素材)
		TUNER = 1 << 0,

		// ====================================

		// 联合/同盟
		UNION = 1 << 1,
		// 灵魂
		SPIRIT = 1 << 2,
		// 二重
		GEMINI = 1 << 3,
		// 反转
		FLIP = 1 << 4,
		// 卡通
		TOON = 1 << 5,
	};

	// 属性
	enum class MonsterAttribute : std::uint8_t
	{
		NONE = 0,

		// ====================================

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
		DIVINE = 6,
	};

	// 种族
	enum class MonsterRace : std::uint16_t
	{
		NONE = 0,

		// ====================================

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

	// 链接箭头
	enum class MonsterLinkMarker : std::uint8_t
	{
		NONE = 0,

		// ====================================

		TOP_LEFT = 1 << 0,
		TOP = 1 << 1,
		TOP_RIGHT = 1 << 2,

		LEFT = 1 << 3,
		RIGHT = 1 << 4,

		BOTTOM_LEFT = 1 << 5,
		BOTTOM = 1 << 6,
		BOTTOM_RIGHT = 1 << 7,
	};

	// ==================================================
	// 魔法卡
	// ==================================================

	// 种类
	enum class SpellCategory : std::uint8_t
	{
		NONE = 0,

		// ====================================

		// 通常
		NORMAL = 1 << 0,
		// 速攻
		QUICKPLAY = 1 << 1,
		// 装备
		EQUIP = 1 << 2,
		// 场地
		FIELD = 1 << 3,
		// 永续
		CONTINUOUS = 1 << 4,
		// 仪式
		RITUAL = 1 << 5,
	};

	// ==================================================
	// 陷阱卡
	// ==================================================

	// 种类
	enum class TrapCategory : std::uint8_t
	{
		NONE = 0,

		// ====================================

		// 通常
		NORMAL = 1 << 0,
		// 永续陷阱
		CONTINUOUS = 1 << 1,
		// 反击陷阱
		COUNTER = 1 << 2,
	};

	// ==================================================
	// 魔法卡 & 陷阱卡
	// ==================================================

	// 咒文速度(理论上可以不需要)
	enum class SpellSpeed : std::uint8_t
	{
		// 通常魔法/装备魔法/场地魔法/永续魔法/仪式魔法
		S1 = 1,
		// 速攻魔法/通常陷阱/永续陷阱
		S2 = 2,
		// 反击陷阱
		S3 = 3,
	};

	// 咒文状态
	enum class SpellStatus : std::uint8_t
	{
		NONE = 0,

		// ====================================

		// 盖放状态(里侧)
		SET = 1 << 0,
		// 发动中/表侧表示
		ACTIVE = 1 << 1,
		// 效果被无效化
		NEGATED = 1 << 2,
	};
}

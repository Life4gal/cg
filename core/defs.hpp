#pragma once

#include <utility>

namespace cg
{
	// 玩家
	enum class Player : std::uint8_t
	{
		UNKNOWN = 0,

		// ====================================

		PLAYER_1,
		PLAYER_2,
	};

	// 卡片位置
	enum class CardLocation : std::uint16_t
	{
		NONE = 0,

		// ====================================

		// 卡组区域
		INTERNAL_DECK_MAIN = 1 << 0,
		INTERNAL_DECK_EXTRA = 1 << 1,
		// 卡组位置
		INTERNAL_DECK_POSITION_TOP = 1 << 2,
		INTERNAL_DECK_POSITION_BOTTOM = 1 << 3,
		// 怪兽区域
		INTERNAL_MONSTER_MAIN_ZONE = 1 << 4,
		INTERNAL_MONSTER_EXTRA_ZONE = 1 << 5,
		// 魔陷区域
		INTERNAL_SPELL_MAIN_ZONE = 1 << 6,
		INTERNAL_SPELL_FIELD_ZONE = 1 << 7,
		INTERNAL_SPELL_PENDULUM_ZONE = 1 << 8,
		// 卡牌位置
		INTERNAL_POSITION_HAND = 1 << 9,
		INTERNAL_POSITION_GRAVEYARD = 1 << 10,
		INTERNAL_POSITION_REMOVED = 1 << 11,
		INTERNAL_POSITION_OVERLAY = 1 << 12,

		// ====================================

		// 主卡组
		MAIN_DECK = INTERNAL_DECK_MAIN,
		// 额外卡组
		EXTRA_DECK = INTERNAL_DECK_EXTRA,
		// 卡组顶部
		MAIN_DECK_TOP = INTERNAL_DECK_MAIN | INTERNAL_DECK_POSITION_TOP,
		// 卡组底部
		MAIN_DECK_BOTTOM = INTERNAL_DECK_MAIN | INTERNAL_DECK_POSITION_BOTTOM,

		// 怪兽区域
		MONSTER_ZONE = INTERNAL_MONSTER_MAIN_ZONE | INTERNAL_MONSTER_EXTRA_ZONE,
		// 魔陷区域
		SPELL_ZONE = INTERNAL_SPELL_MAIN_ZONE | INTERNAL_SPELL_FIELD_ZONE | INTERNAL_SPELL_PENDULUM_ZONE,
		// 场上
		FIELD = MONSTER_ZONE | SPELL_ZONE,

		// 手牌
		HAND = INTERNAL_POSITION_HAND,
		// 墓地
		GRAVEYARD = INTERNAL_POSITION_GRAVEYARD,
		// 除外区
		REMOVED = INTERNAL_POSITION_REMOVED,
		// 超量素材
		OVERLAY = INTERNAL_POSITION_OVERLAY,
	};

	// 卡片场上位置
	enum class CardZone : std::uint16_t
	{
		NONE = 0,

		// ====================================

		// 从左往右

		// -主怪兽区
		MONSTER_MAIN_ZONE_0 = 1 << 0,
		MONSTER_MAIN_ZONE_1 = 1 << 1,
		MONSTER_MAIN_ZONE_2 = 1 << 2,
		MONSTER_MAIN_ZONE_3 = 1 << 3,
		MONSTER_MAIN_ZONE_4 = 1 << 4,
		MONSTER_MAIN_ZONE = MONSTER_MAIN_ZONE_0 | MONSTER_MAIN_ZONE_1 | MONSTER_MAIN_ZONE_2 | MONSTER_MAIN_ZONE_3 | MONSTER_MAIN_ZONE_4,
		// -额外怪兽区
		MONSTER_EXTRA_ZONE_0 = 1 << 5,
		MONSTER_EXTRA_ZONE_1 = 1 << 6,
		MONSTER_EXTRA_ZONE = MONSTER_EXTRA_ZONE_0 | MONSTER_EXTRA_ZONE_1,
		// -怪兽区
		MONSTER_ZONE = MONSTER_MAIN_ZONE | MONSTER_EXTRA_ZONE,
		// -魔法陷阱区
		SPELL_ZONE_0 = 1 << 7,
		SPELL_ZONE_1 = 1 << 8,
		SPELL_ZONE_2 = 1 << 9,
		SPELL_ZONE_3 = 1 << 10,
		SPELL_ZONE_4 = 1 << 11,
		SPELL_ZONE = SPELL_ZONE_0 | SPELL_ZONE_1 | SPELL_ZONE_2 | SPELL_ZONE_3 | SPELL_ZONE_4,
		// -灵摆区
		SPELL_PENDULUM_ZONE_LEFT = SPELL_ZONE_0,
		SPELL_PENDULUM_ZONE_RIGHT = SPELL_ZONE_4,
		SPELL_PENDULUM_ZONE = SPELL_PENDULUM_ZONE_LEFT | SPELL_PENDULUM_ZONE_RIGHT,
		// -场地魔法区
		SPELL_FIELD_ZONE = 1 << 12,
	};

	// 卡片表示形式
	enum class CardForm : std::uint8_t
	{
		NONE = 0,

		// ====================================

		// 表侧表示
		INTERNAL_FACE_UP = 1 << 0,
		// 里侧表示
		INTERNAL_FACE_DOWN = 1 << 1,
		// 攻击表示
		INTERNAL_ATTACK = 1 << 2,
		// 守备表示
		INTERNAL_DEFENSE = 1 << 3,

		// ====================================

		// 表侧攻击
		FACE_UP_ATTACK = INTERNAL_FACE_UP | INTERNAL_ATTACK,
		// 里侧攻击
		FACE_DOWN_ATTACK = INTERNAL_FACE_DOWN | INTERNAL_ATTACK,
		// 表侧守备
		FACE_UP_DEFENSE = INTERNAL_FACE_UP | INTERNAL_DEFENSE,
		// 里侧守备
		FACE_DOWN_DEFENSE = INTERNAL_FACE_DOWN | INTERNAL_DEFENSE,

		// 表侧攻击或是守备
		FACE_UP = FACE_UP_ATTACK | FACE_UP_DEFENSE,
		// 里侧攻击或是守备
		FACE_DOWN = FACE_DOWN_ATTACK | FACE_DOWN_DEFENSE,
		// 表侧或是里侧攻击
		ATTACK = FACE_UP_ATTACK | FACE_DOWN_ATTACK,
		// 表侧或是里侧守备
		DEFENSE = FACE_UP_DEFENSE | FACE_DOWN_DEFENSE,
	};

	// 卡片状态改变原因
	enum class CardReason : std::uint32_t
	{
		// 抽卡
		INTERNAL_DRAW = 1 << 0,
		// 被破坏
		INTERNAL_DESTROY = 1 << 1,

		// 丢弃(手牌送墓)
		INTERNAL_DISCARD = 1 << 2,
		// 代价
		INTERNAL_COST = 1 << 3,

		INTERNAL_SUMMON_ADVANCE = 1 << 10,
		INTERNAL_SUMMON_SPECIAL = 1 << 11,
		INTERNAL_SUMMON_RITUAL = 1 << 12,
		INTERNAL_SUMMON_FUSION = 1 << 13,
		INTERNAL_SUMMON_SYNCHRO = 1 << 14,
		INTERNAL_SUMMON_XYZ = 1 << 15,
		INTERNAL_SUMMON_LINK = 1 << 16,

		// ====================================

		// TODO

		// 规则
		RULE,

	};

	// 卡片类型
	enum class CardType : std::uint32_t
	{
		// 最基本分类
		INTERNAL_MONSTER = 1 << 0,
		INTERNAL_SPELL = 1 << 1,
		INTERNAL_TRAP = 1 << 2,

		// 怪兽效果
		INTERNAL_MONSTER_NORMAL = 1 << 3,
		INTERNAL_MONSTER_EFFECT = 1 << 4,

		// 怪兽能力
		// -衍生物
		INTERNAL_MONSTER_TOKEN = 1 << 5,
		// -调整(同调召唤素材)
		INTERNAL_MONSTER_TUNER = 1 << 6,
		// -联合/同盟
		INTERNAL_MONSTER_UNION = 1 << 7,
		// -灵魂
		INTERNAL_MONSTER_SPIRIT = 1 << 8,
		// -二重
		INTERNAL_MONSTER_GEMINI = 1 << 9,
		INTERNAL_MONSTER_DUAL = INTERNAL_MONSTER_GEMINI,
		// -反转
		INTERNAL_MONSTER_FLIP = 1 << 10,
		// -卡通
		INTERNAL_MONSTER_TOON = 1 << 11,

		// 怪兽类型
		INTERNAL_MONSTER_RITUAL = 1 << 12,
		INTERNAL_MONSTER_FUSION = 1 << 13,
		INTERNAL_MONSTER_SYNCHRO = 1 << 14,
		INTERNAL_MONSTER_XYZ = 1 << 15,
		INTERNAL_MONSTER_PENDULUM = 1 << 16,
		INTERNAL_MONSTER_LINK = 1 << 17,

		// 魔陷能力
		INTERNAL_SPELL_EQUIP = 1 << 18,
		INTERNAL_SPELL_FIELD = 1 << 19,
		INTERNAL_SPELL_CONTINUOUS = 1 << 20,
		INTERNAL_SPELL_RITUAL = 1 << 21,
		INTERNAL_SPELL_QUICKPLAY = 1 << 22,
		INTERNAL_SPELL_COUNTER = 1 << 23,
		INTERNAL_SPELL_TRAP_MONSTER = 1 << 24,

		// ====================================

		// 通常-衍生物-X
		TOKEN = INTERNAL_MONSTER | INTERNAL_MONSTER_TOKEN,
		// 通常-调整-X
		NORMAL_TUNER = INTERNAL_MONSTER | INTERNAL_MONSTER_NORMAL | INTERNAL_MONSTER_TUNER,
		// 通常-X-仪式
		NORMAL_RITUAL = INTERNAL_MONSTER | INTERNAL_MONSTER_NORMAL | INTERNAL_MONSTER_RITUAL,
		// 通常-X-融合
		NORMAL_FUSION = INTERNAL_MONSTER | INTERNAL_MONSTER_NORMAL | INTERNAL_MONSTER_FUSION,
		// 通常-X-同调
		NORMAL_SYNCHRO = INTERNAL_MONSTER | INTERNAL_MONSTER_NORMAL | INTERNAL_MONSTER_SYNCHRO,
		// 通常-X-超量
		NORMAL_XYZ = INTERNAL_MONSTER | INTERNAL_MONSTER_NORMAL | INTERNAL_MONSTER_XYZ,
		// 通常-X-灵摆
		NORMAL_PENDULUM = INTERNAL_MONSTER | INTERNAL_MONSTER_NORMAL | INTERNAL_MONSTER_PENDULUM,
		// 通常-X-连接
		NORMAL_LINK = INTERNAL_MONSTER | INTERNAL_MONSTER_NORMAL | INTERNAL_MONSTER_LINK,

		// 效果-联合/同盟-X
		EFFECT_UNION = INTERNAL_MONSTER | INTERNAL_MONSTER_EFFECT | INTERNAL_MONSTER_UNION,
		// 效果-灵魂-X
		EFFECT_SPIRIT = INTERNAL_MONSTER | INTERNAL_MONSTER_EFFECT | INTERNAL_MONSTER_SPIRIT,
		// 效果-二重-X
		EFFECT_GEMINI = INTERNAL_MONSTER | INTERNAL_MONSTER_EFFECT | INTERNAL_MONSTER_GEMINI,
		EFFECT_DUAL = EFFECT_GEMINI,
		// 效果-反转-X
		EFFECT_FLIP = INTERNAL_MONSTER | INTERNAL_MONSTER_EFFECT | INTERNAL_MONSTER_FLIP,
		// 效果-卡通-X
		EFFECT_TOON = INTERNAL_MONSTER | INTERNAL_MONSTER_EFFECT | INTERNAL_MONSTER_TOON,

		// 效果-调整-仪式
		EFFECT_TUNER_RITUAL = INTERNAL_MONSTER | INTERNAL_MONSTER_EFFECT | INTERNAL_MONSTER_TUNER | INTERNAL_MONSTER_RITUAL,
		// 效果-调整-融合
		EFFECT_TUNER_FUSION = INTERNAL_MONSTER | INTERNAL_MONSTER_EFFECT | INTERNAL_MONSTER_TUNER | INTERNAL_MONSTER_FUSION,
		// 效果-调整-同调
		EFFECT_TUNER_SYNCHRO = INTERNAL_MONSTER | INTERNAL_MONSTER_EFFECT | INTERNAL_MONSTER_TUNER | INTERNAL_MONSTER_SYNCHRO,
		// 效果-调整-超量
		EFFECT_TUNER_XYZ = INTERNAL_MONSTER | INTERNAL_MONSTER_EFFECT | INTERNAL_MONSTER_TUNER | INTERNAL_MONSTER_XYZ,
		// 效果-调整-灵摆
		EFFECT_TUNER_PENDULUM = INTERNAL_MONSTER | INTERNAL_MONSTER_EFFECT | INTERNAL_MONSTER_TUNER | INTERNAL_MONSTER_PENDULUM,
		// 效果-调整-连接
		EFFECT_TUNER_LINK = INTERNAL_MONSTER | INTERNAL_MONSTER_EFFECT | INTERNAL_MONSTER_TUNER | INTERNAL_MONSTER_LINK,

		// 额外卡组的卡牌
		EXTRA_DECK = INTERNAL_MONSTER_FUSION | INTERNAL_MONSTER_SYNCHRO | INTERNAL_MONSTER_XYZ | INTERNAL_MONSTER_LINK,
	};

	// 卡片字段
	class CardArchetype
	{
	public:
		using value_type = std::uint16_t;

		// 主字段,例如[HERO]
		value_type main : 12;
		// 子字段,例如[D-HERO],[E-HERO]等
		value_type sub : 4;

		constexpr CardArchetype() noexcept
			: main{0},
			  sub{0} {}

		constexpr explicit CardArchetype(const value_type value) noexcept
			: main{static_cast<value_type>(value & 0x0FFF)},
			  sub{static_cast<value_type>((value >> 12) & 0x000F)} {}

		[[nodiscard]] constexpr auto operator==(const CardArchetype& other) const noexcept -> bool
		{
			return main == other.main && sub == other.sub;
		}

		[[nodiscard]] constexpr auto match(const CardArchetype& required) const noexcept -> bool
		{
			// 主字段必须完全匹配
			if (main != required.main)
			{
				return false;
			}

			// 子字段必须完全包含
			if ((sub & required.sub) != required.sub)
			{
				return false;
			}

			return true;
		}
	};

	// 无效字段
	constexpr CardArchetype InvalidArchetype{};

	// 怪兽卡属性
	enum class MonsterAttribute : std::uint8_t
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
		DIVINE = 6,
	};

	// 怪兽卡种族
	enum class MonsterRace : std::uint8_t
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

	// 怪兽卡等级(等级可能被改变,我们允许更大的值)
	enum class MonsterLevel : std::uint16_t
	{
		L0 = 0,
		L1 = 1,
		L2 = 2,
		L3 = 3,
		L4 = 4,
		L5 = 5,
		L6 = 6,
		L7 = 7,
		L8 = 8,
		L9 = 9,
		L10 = 10,
		L11 = 11,
		L12 = 12,
		L13 = 13,

		L_MAX = 0xFFFF,
	};

	// 怪兽卡阶级
	enum class MonsterRank : std::uint8_t
	{
		R0 = 0,
		R1 = 1,
		R2 = 2,
		R3 = 3,
		R4 = 4,
		R5 = 5,
		R6 = 6,
		R7 = 7,
		R8 = 8,
		R9 = 9,
		R10 = 10,
		R11 = 11,
		R12 = 12,
		R13 = 13,
	};

	// 怪兽卡灵摆刻度
	enum class MonsterPendulum : std::uint8_t
	{
		P0 = 0,
		P1 = 1,
		P2 = 2,
		P3 = 3,
		P4 = 4,
		P5 = 5,
		P6 = 6,
		P7 = 7,
		P8 = 8,
		P9 = 9,
		P10 = 10,
		P11 = 11,
		P12 = 12,
		P13 = 13,
	};

	// 怪兽卡链接箭头
	enum class MonsterLinkMarker : std::uint8_t
	{
		TOP_LEFT = 1 << 0,
		TOP = 1 << 1,
		TOP_RIGHT = 1 << 2,

		LEFT = 1 << 3,
		RIGHT = 1 << 4,

		BOTTOM_LEFT = 1 << 5,
		BOTTOM = 1 << 6,
		BOTTOM_RIGHT = 1 << 7,
	};

	// 怪兽卡召唤类型
	enum class MonsterSummonType : std::uint16_t
	{
		// 通常召唤
		INTERNAL_NORMAL = 1 << 0,
		// 上级召唤
		INTERNAL_ADVANCE = 1 << 1,
		// 二重召唤
		INTERNAL_GEMINI = 1 << 2,
		INTERNAL_DUAL = INTERNAL_GEMINI,

		// 反转召唤
		INTERNAL_FLIP = 1 << 3,

		// 特殊召唤
		INTERNAL_SPECIAL = 1 << 4,
		// 仪式召唤
		INTERNAL_RITUAL = 1 << 5,
		// 融合召唤
		INTERNAL_FUSION = 1 << 6,
		// 同调召唤
		INTERNAL_SYNCHRO = 1 << 7,
		// 超量召唤
		INTERNAL_XYZ = 1 << 8,
		// 灵摆召唤
		INTERNAL_PENDULUM = 1 << 9,
		// 连接召唤
		INTERNAL_LINK = 1 << 10,

		// ====================================

		ADVANCE = INTERNAL_NORMAL | INTERNAL_ADVANCE,
		GEMINI = INTERNAL_NORMAL | INTERNAL_GEMINI,
		DUAL = INTERNAL_NORMAL | INTERNAL_DUAL,

		FLIP = INTERNAL_FLIP,

		RITUAL = INTERNAL_SPECIAL | INTERNAL_RITUAL,
		FUSION = INTERNAL_SPECIAL | INTERNAL_FUSION,
		SYNCHRO = INTERNAL_SPECIAL | INTERNAL_SYNCHRO,
		XYZ = INTERNAL_SPECIAL | INTERNAL_XYZ,
		PENDULUM = INTERNAL_SPECIAL | INTERNAL_PENDULUM,
		LINK = INTERNAL_SPECIAL | INTERNAL_LINK,
	};
}

#pragma once

#include <array>
#include <variant>

namespace cg::core
{
	// 怪兽卡类型
	class MonsterCategory
	{
	public:
		enum class Effect : std::uint8_t
		{
			// 通常怪兽
			NORMAL,
			// 效果怪兽
			EFFECT,
		};

		enum class Type : std::uint8_t
		{
			// 通常
			NORMAL = 0,
			// 仪式
			RITUAL = 1 << 0,
			// 融合
			FUSION = 1 << 1,
			// 同调
			SYNCHRO = 1 << 2,
			// 超量
			XYZ = 1 << 3,
			// 灵摆
			PENDULUM = 1 << 4,
			// 链接
			LINK = 1 << 5,
		};

		enum class Ability : std::uint8_t
		{
			// 无能力
			NONE = 0,
			// 调整
			TUNER = 1 << 0,
			// 联合/同盟
			UNION = 1 << 1,
			// 灵魂
			SPIRIT = 1 << 2,
			// 二重
			GEMINI = 1 << 3,
			DUAL = GEMINI,
			// 反转
			FLIP = 1 << 4,
			// 卡通
			TOON = 1 << 5,
		};

		Effect effect;
		Type type;
		Ability ability;

		std::uint8_t pad;
	};

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

	// 魔法卡类型
	enum class SpellCategory : std::uint8_t
	{
		// 普通
		NORMAL = 0,
		// 装备
		EQUIP,
		// 仪式
		RITUAL,
		// 速攻
		QUICKPLAY,
		// 永续
		CONTINUOUS,
		// 场地
		FIELD,
	};

	// 陷阱卡类型
	enum class TrapCategory : std::uint8_t
	{
		// 普通
		NORMAL = 0,
		// 永续
		CONTINUOUS,
		// 反击
		COUNTER,
	};

	// 卡片字段
	class Archetype
	{
	public:
		using value_type = std::uint16_t;

		// 主字段,例如[HERO]
		value_type main : 12;
		// 子字段,例如[D-HERO],[E-HERO]等
		value_type sub : 4;

		[[nodiscard]] constexpr static auto none() noexcept -> Archetype
		{
			return Archetype{0};
		}

		constexpr explicit Archetype(const value_type value) noexcept
			: main{static_cast<value_type>(value & 0x0FFF)},
			  sub{static_cast<value_type>((value >> 12) & 0x000F)} {}

		[[nodiscard]] constexpr auto operator==(const Archetype& other) const noexcept -> bool
		{
			return main == other.main && sub == other.sub;
		}

		[[nodiscard]] constexpr auto match(const Archetype& required) const noexcept -> bool
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

	// 无字段
	constexpr auto NoneArchetype = Archetype::none();

	// 卡片数据
	class Data
	{
	public:
		using code_type = std::uint32_t;

		// 无效卡密
		constexpr static code_type invalid_code = 0;
		// 一张卡片默认情况下其数据最多存有多少个字段
		constexpr static std::size_t max_archetype_count = 16;

		class Uninitialized {};

		class Monster
		{
		public:
			using property_value_type = std::uint32_t;

			// 类型
			MonsterCategory category;

			// 属性
			MonsterAttribute attribute;
			// 种族
			MonsterRace race;

			// 等级
			MonsterLevel level;
			// 阶级
			MonsterRank rank;
			// 左侧灵摆刻度
			MonsterPendulum left_pendulum;
			// 右侧灵摆刻度
			MonsterPendulum right_pendulum;
			// 链接箭头
			MonsterLinkMarker link_marker;

			// 攻击力
			property_value_type attack;
			// 守备力
			property_value_type defense;
		};

		class Spell
		{
		public:
			// 类型
			SpellCategory category;
		};

		class Trap
		{
		public:
			// 类型
			TrapCategory category;
		};

		using data_type = std::variant<Uninitialized, Monster, Spell, Trap>;
		using archetypes_type = std::array<Archetype, max_archetype_count>;

		// 8位卡片密码(如青眼白龙是89631139)
		code_type code;
		// 别名(视为另一张卡,用于不同卡图版本)
		code_type alias_code;
		// 规则用替代密码
		code_type rule_code;

		// 卡片类型+数据
		data_type data;
		// 卡片字段
		archetypes_type archetypes;

	private:
		[[nodiscard]] constexpr static auto default_archetypes() noexcept -> archetypes_type
		{
			return []<std::size_t... Index>(std::index_sequence<Index...>) noexcept -> archetypes_type
			{
				return {(std::ignore = Index, NoneArchetype)...}; // NOLINT(clang-diagnostic-comma)
			}(std::make_index_sequence<max_archetype_count>{});
		}

	public:
		constexpr Data() noexcept
			: code{invalid_code},
			  alias_code{invalid_code},
			  rule_code{invalid_code},
			  data{Uninitialized{}},
			  archetypes{default_archetypes()} {}
	};
}

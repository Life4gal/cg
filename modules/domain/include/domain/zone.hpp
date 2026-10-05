#pragma once

#include <variant>

namespace cg::domain
{
	class Zone
	{
	public:
		// 各个区域的卡数量不会超过255张
		using size_type = std::uint8_t;

		// =======================================================================

		enum class MonsterPosition : size_type
		{
			MAIN_1 = 0,
			MAIN_2 = 1,
			MAIN_3 = 2,
			MAIN_4 = 3,
			MAIN_5 = 4,
		};

		enum class ExtraMonsterPosition : size_type
		{
			EXTRA_1 = 0,
			EXTRA_2 = 1,
		};

		enum class MonsterForm : size_type
		{
			FACE_UP_ATTACK,
			FACE_DOWN_ATTACK,
			FACE_UP_DEFENSE,
			FACE_DOWN_DEFENSE,
		};

		enum class SpellTrapPosition : size_type
		{
			SPELL_TRAP_1 = 0,
			SPELL_TRAP_2 = 1,
			SPELL_TRAP_3 = 2,
			SPELL_TRAP_4 = 3,
			SPELL_TRAP_5 = 4,
			FIELD_SPELL = 5,
			PENDULUM_LEFT = SPELL_TRAP_1,
			PENDULUM_RIGHT = SPELL_TRAP_5,
		};

		enum class SpellTrapForm : size_type
		{
			FACE_UP,
			FACE_DOWN,
		};

		constexpr static auto monster_main_count = static_cast<std::size_t>(5);
		constexpr static auto monster_extra_count = static_cast<std::size_t>(2);
		constexpr static auto spell_trap_count = static_cast<std::size_t>(6);

		// =======================================================================

		// 卡组
		class Deck
		{
		public:
			size_type index;

			[[nodiscard]] constexpr auto operator==(const Deck& other) const noexcept -> bool = default;
		};

		// 额外卡组
		class ExtraDeck
		{
		public:
			size_type index;

			[[nodiscard]] constexpr auto operator==(const ExtraDeck& other) const noexcept -> bool = default;
		};

		// 手牌
		class Hand
		{
		public:
			size_type index;

			[[nodiscard]] constexpr auto operator==(const Hand& other) const noexcept -> bool = default;
		};

		// 墓地
		class Graveyard
		{
		public:
			size_type index;

			[[nodiscard]] constexpr auto operator==(const Graveyard& other) const noexcept -> bool = default;
		};

		// 除外区
		class Removed
		{
		public:
			size_type index;

			[[nodiscard]] constexpr auto operator==(const Removed& other) const noexcept -> bool = default;
		};

		// 超量素材
		class Overlay
		{
		public:
			size_type index;

			[[nodiscard]] constexpr auto operator==(const Overlay& other) const noexcept -> bool = default;
		};

		// 怪兽区域
		class Monster
		{
		public:
			MonsterPosition position;
			MonsterForm form;

			[[nodiscard]] constexpr auto operator==(const Monster& other) const noexcept -> bool = default;
		};

		// 额外怪兽区
		class ExtraMonster
		{
		public:
			ExtraMonsterPosition position;
			MonsterForm form;

			[[nodiscard]] constexpr auto operator==(const ExtraMonster& other) const noexcept -> bool = default;
		};

		// 魔陷区域
		class SpellTrap
		{
		public:
			SpellTrapPosition position;
			SpellTrapForm form;

			[[nodiscard]] constexpr auto operator==(const SpellTrap& other) const noexcept -> bool = default;
		};

		// 不存在区域
		class NotExist
		{
		public:
			[[nodiscard]] constexpr auto operator==(const NotExist& other) const noexcept -> bool = default;
		};

		using zone_type = std::variant<
			// 卡组
			Deck,
			// 额外卡组
			ExtraDeck,
			// 手牌
			Hand,
			// 墓地
			Graveyard,
			// 除外区
			Removed,
			// 超量素材
			Overlay,
			// 怪兽区域
			Monster,
			// 额外怪兽区
			ExtraMonster,
			// 魔陷区域
			SpellTrap,
			// 不存在区域
			NotExist
			//
		>;

		constexpr static NotExist not_exist{};

		zone_type zone;
	};

	// 自动区域 -- 只设置目标区域,不设置具体位置(例如将卡牌移动到卡组/额外卡组/手牌/墓地/除外区,自动设置其所属位置)
	enum class AutoZone : std::uint8_t
	{
		DECK,
		EXTRA_DECK,
		HAND,
		GRAVEYARD,
		REMOVED,
	};

	// 场地区域 -- 如怪兽区/额外怪兽区/魔法陷阱区
	enum class FieldZone : std::uint8_t
	{
		MONSTER,
		EXTRA_MONSTER,
		SPELL_TRAP
	};

	template<auto>
	struct zone_of;

	template<>
	struct zone_of<AutoZone::DECK>
	{
		using type = Zone::Deck;
	};

	template<>
	struct zone_of<AutoZone::EXTRA_DECK>
	{
		using type = Zone::ExtraDeck;
	};

	template<>
	struct zone_of<AutoZone::HAND>
	{
		using type = Zone::Hand;
	};

	template<>
	struct zone_of<AutoZone::GRAVEYARD>
	{
		using type = Zone::Graveyard;
	};

	template<>
	struct zone_of<AutoZone::REMOVED>
	{
		using type = Zone::Removed;
	};

	template<>
	struct zone_of<FieldZone::MONSTER>
	{
		using type = Zone::Monster;
	};

	template<>
	struct zone_of<FieldZone::EXTRA_MONSTER>
	{
		using type = Zone::ExtraMonster;
	};

	template<>
	struct zone_of<FieldZone::SPELL_TRAP>
	{
		using type = Zone::SpellTrap;
	};

	template<AutoZone Z>
	using zone_of_t = typename zone_of<Z>::type;

	// 选择区域 -- 某些效果适用区域不止一个
	enum class SelectZone : std::uint16_t
	{
		NONE = 0,

		// 卡组
		DECK = 1 << 0,
		// 手牌
		HAND = 1 << 1,
		// 怪兽区域
		MONSTER = 1 << 2,
		// 额外怪兽区
		EXTRA_MONSTER = 1 << 3,
		// 魔陷区域
		SPELL_TRAP = 1 << 4,
		// 墓地
		GRAVEYARD = 1 << 5,
		// 除外区
		REMOVED = 1 << 6,
		// 额外卡组
		EXTRA_DECK = 1 << 7,
		// 超量素材
		OVERLAY = 1 << 8,
	};
}

#pragma once

#include <array>
#include <variant>

#include <utility/enum.hpp>

namespace cg::domain
{
	// ==============================================================
	// ZONE
	// ==============================================================

	// 区域
	enum class Zone : std::uint8_t
	{
		// 不存在区域
		NOT_EXIST = 0,
		// 卡组
		DECK = 1 << 0,
		// 额外卡组
		EXTRA_DECK = 1 << 1,
		// 手牌
		HAND = 1 << 2,
		// 墓地
		GRAVEYARD = 1 << 3,
		// 除外区
		BANISHED = 1 << 4,
		// 怪兽区
		MONSTER = 1 << 5,
		// 额外怪兽区
		EXTRA_MONSTER = 1 << 6,
		// 魔陷区
		SPELL_TRAP = 1 << 7,

		// ================================================

		// 场上所有区域
		ON_FIELD = MONSTER | EXTRA_MONSTER | SPELL_TRAP,
	};

	// ==============================================================
	// LOCATION
	// ==============================================================

	// 不存在区域
	class NotExistLocation
	{
	public:
		[[nodiscard]] constexpr auto operator==(const NotExistLocation& other) const noexcept -> bool = default;
	};

	// 卡组
	class DeckLocation
	{
	public:
		// 卡数量不会超过255张
		using size_type = std::uint8_t;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const DeckLocation& other) const noexcept -> bool = default;
	};

	// 额外卡组
	class ExtraDeckLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const ExtraDeckLocation& other) const noexcept -> bool = default;
	};

	// 手牌
	class HandLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const HandLocation& other) const noexcept -> bool = default;
	};

	// 墓地
	class GraveyardLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const GraveyardLocation& other) const noexcept -> bool = default;
	};

	// 除外区
	class BanishedLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const BanishedLocation& other) const noexcept -> bool = default;
	};

	// 怪兽区域
	class MonsterLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		enum class Slot : size_type
		{
			MAIN_1 = 0,
			MAIN_2 = 1,
			MAIN_3 = 2,
			MAIN_4 = 3,
			MAIN_5 = 4,
		};

		constexpr static size_type count = 5;
		constexpr static std::array<Slot, count> slots{Slot::MAIN_1, Slot::MAIN_2, Slot::MAIN_3, Slot::MAIN_4, Slot::MAIN_5};

		Slot slot;

		[[nodiscard]] constexpr auto operator==(const MonsterLocation& other) const noexcept -> bool = default;
	};

	// 额外怪兽区
	class ExtraMonsterLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		enum class Slot : size_type
		{
			EXTRA_1 = 0,
			EXTRA_2 = 1,
		};

		constexpr static size_type count = 2;
		constexpr static std::array<Slot, count> slots{Slot::EXTRA_1, Slot::EXTRA_2};

		Slot slot;

		[[nodiscard]] constexpr auto operator==(const ExtraMonsterLocation& other) const noexcept -> bool = default;
	};

	// 魔陷区域
	class SpellTrapLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		enum class Slot : size_type
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

		constexpr static size_type count = 6;
		constexpr static std::array<Slot, count> slots{Slot::SPELL_TRAP_1, Slot::SPELL_TRAP_2, Slot::SPELL_TRAP_3, Slot::SPELL_TRAP_4, Slot::SPELL_TRAP_5, Slot::FIELD_SPELL};

		// 仅主要魔陷区,不包括场地魔法
		constexpr static size_type main_count = 5;
		constexpr static std::array<Slot, main_count> main_slots{Slot::SPELL_TRAP_1, Slot::SPELL_TRAP_2, Slot::SPELL_TRAP_3, Slot::SPELL_TRAP_4, Slot::SPELL_TRAP_5};

		// 仅灵摆区
		constexpr static size_type pendulum_count = 2;
		constexpr static std::array<Slot, pendulum_count> pendulum_slots{Slot::PENDULUM_LEFT, Slot::PENDULUM_RIGHT};

		Slot slot;
	};

	class Location : public std::variant<
				// 不存在区域
				NotExistLocation,
				// 卡组
				DeckLocation,
				// 额外卡组
				ExtraDeckLocation,
				// 手牌
				HandLocation,
				// 墓地
				GraveyardLocation,
				// 除外区
				BanishedLocation,
				// 怪兽区域
				MonsterLocation,
				// 额外怪兽区
				ExtraMonsterLocation,
				// 魔陷区域
				SpellTrapLocation
				//
			> {};

	// ==============================================================
	// POSITION
	// ==============================================================

	// 不存在区域
	class NotExistPosition
	{
	public:
		[[nodiscard]] constexpr auto operator==(const NotExistPosition& other) const noexcept -> bool = default;
	};

	// 卡组
	class DeckPosition
	{
	public:
		using size_type = DeckLocation::size_type;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const DeckPosition& other) const noexcept -> bool = default;
	};

	// 额外卡组
	class ExtraDeckPosition
	{
	public:
		using size_type = DeckPosition::size_type;

		enum class Form : size_type
		{
			FACE_UP,
			FACE_DOWN,
		};

		size_type index;
		Form form;

		[[nodiscard]] constexpr auto operator==(const ExtraDeckPosition& other) const noexcept -> bool = default;
	};

	// 手牌
	class HandPosition
	{
	public:
		using size_type = DeckPosition::size_type;

		enum class Form : size_type
		{
			FACE_UP,
			FACE_DOWN,
		};

		size_type index;
		Form form;

		[[nodiscard]] constexpr auto operator==(const HandPosition& other) const noexcept -> bool = default;
	};

	// 墓地
	class GraveyardPosition
	{
	public:
		using size_type = DeckPosition::size_type;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const GraveyardPosition& other) const noexcept -> bool = default;
	};

	// 除外区
	class BanishedPosition
	{
	public:
		using size_type = DeckPosition::size_type;

		enum class Form : size_type
		{
			FACE_UP,
			FACE_DOWN,
		};

		size_type index;
		Form form;

		[[nodiscard]] constexpr auto operator==(const BanishedPosition& other) const noexcept -> bool = default;
	};

	// 怪兽区域
	class MonsterPosition
	{
	public:
		using size_type = DeckPosition::size_type;

		enum class Slot : size_type
		{
			MAIN_1 = 0,
			MAIN_2 = 1,
			MAIN_3 = 2,
			MAIN_4 = 3,
			MAIN_5 = 4,
		};

		enum class Form : size_type
		{
			FACE_UP_ATTACK,
			FACE_DOWN_ATTACK,
			FACE_UP_DEFENSE,
			FACE_DOWN_DEFENSE,
		};

		Slot slot;
		Form form;

		[[nodiscard]] constexpr auto operator==(const MonsterPosition& other) const noexcept -> bool = default;
	};

	// 额外怪兽区
	class ExtraMonsterPosition
	{
	public:
		using size_type = MonsterPosition::size_type;

		enum class Slot : size_type
		{
			EXTRA_1 = 0,
			EXTRA_2 = 1,
		};

		Slot slot;
		MonsterPosition::Form form;

		[[nodiscard]] constexpr auto operator==(const ExtraMonsterPosition& other) const noexcept -> bool = default;
	};

	// 魔陷区域
	class SpellTrapPosition
	{
	public:
		using size_type = DeckPosition::size_type;

		enum class Slot : size_type
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

		enum class Form : size_type
		{
			FACE_UP,
			FACE_DOWN,
		};

		Slot slot;
		Form form;

		[[nodiscard]] constexpr auto operator==(const SpellTrapPosition& other) const noexcept -> bool = default;
	};

	class Position : public std::variant<
				// 不存在区域
				NotExistPosition,
				// 卡组
				DeckPosition,
				// 额外卡组
				ExtraDeckPosition,
				// 手牌
				HandPosition,
				// 墓地
				GraveyardPosition,
				// 除外区
				BanishedPosition,
				// 怪兽区域
				MonsterPosition,
				// 额外怪兽区
				ExtraMonsterPosition,
				// 魔陷区域
				SpellTrapPosition
				//
			> {};
}

template<>
struct cg::utility::is_flag<cg::domain::Zone> : std::true_type {};

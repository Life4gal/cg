#pragma once

#include <core/utility/enum.hpp>

namespace cg::domain
{
	// 卡密代码
	enum class CardCode : std::uint32_t
	{
		ORIGIN = 0,
		INVALID = static_cast<std::uint32_t>(-1)
	};

	// 字段代码
	enum class SeriesCode : std::uint32_t
	{
		INVALID = static_cast<std::uint32_t>(-1)
	};

	// 卡片类型
	enum class CardType : std::uint32_t
	{
		NONE = 0,

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

	class CardTypeWrapper : public utility::Enum<
				CardType,
				// ==, !=
				utility::EnumSupport::EQUALITY,
				// &, &=, |, |=, ^, ^=, ~
				utility::EnumSupport::BIT_OPERATION
			> {};

	// 是怪兽卡
	[[nodiscard]] constexpr auto is_monster_card(const CardTypeWrapper card) noexcept -> bool
	{
		return card.any_of(CardType::MONSTER);
	}

	// 是额外卡组怪兽卡
	[[nodiscard]] constexpr auto is_extra_deck_monster_card(const CardTypeWrapper card) noexcept -> bool
	{
		constexpr static auto extra = []
		{
			CardTypeWrapper w{CardType::NONE};

			// FUSION
			w |= CardType::FUSION;
			// SYNCHRO
			w |= CardType::SYNCHRO;
			// XYZ
			w |= CardType::XYZ;
			// LINK
			w |= CardType::LINK;

			return w;
		}();

		return card.any_of(extra);
	}

	// 是魔法卡
	[[nodiscard]] constexpr auto is_spell_card(const CardTypeWrapper card) noexcept -> bool
	{
		return card.any_of(CardType::SPELL);
	}

	// 是陷阱卡
	[[nodiscard]] constexpr auto is_trap_card(const CardTypeWrapper card) noexcept -> bool
	{
		return card.any_of(CardType::TRAP);
	}

	// 是魔法或者陷阱卡
	[[nodiscard]] constexpr auto is_spell_or_trap_card(const CardTypeWrapper card) noexcept -> bool
	{
		constexpr static auto spell_or_trap = []
		{
			CardTypeWrapper w{CardType::NONE};

			// SPELL
			w |= CardType::SPELL;
			// TRAP
			w |= CardType::TRAP;

			return w;
		}();

		return card.any_of(spell_or_trap);
	}
}

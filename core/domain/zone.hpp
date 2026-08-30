#pragma once

#include <core/utility/enum.hpp>

namespace cg::domain
{
	// 卡牌所在区域
	enum class Zone : std::uint8_t
	{
		// 卡组
		DECK,
		// 手牌
		HAND,
		// 怪兽区域
		MONSTER,
		// 额外怪兽区
		EXTRA_MONSTER,
		// 魔陷区域
		SPELL_TRAP,
		// 墓地
		GRAVEYARD,
		// 除外区
		REMOVED,
		// 额外卡组
		EXTRA_DECK,
		// 超量素材
		OVERLAY,
		// 灵摆区域
		// 移除该定义,使用Zone+FieldZoneSequence判断
		// PENDULUM,
		// 场地魔法区域
		// 移除该定义,使用Zone+FieldZoneSequence判断
		// FIELD_SPELL,
	};

	// 场上区域
	[[nodiscard]] constexpr auto is_field_zone(const Zone zone) noexcept -> bool
	{
		// return zone == Zone::MONSTER || zone == Zone::EXTRA_MONSTER || zone == Zone::SPELL_TRAP || zone == Zone::PENDULUM || zone == Zone::FIELD_SPELL;
		return zone == Zone::MONSTER || zone == Zone::EXTRA_MONSTER || zone == Zone::SPELL_TRAP;
	}

	// 选卡区域
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
		// // 灵摆区域
		// PENDULUM = 1 << 9,
		// // 场地区域
		// FIELD_SPELL = 1 << 10,
	};

	class SelectZoneWrapper : public utility::Enum<
				SelectZone,
				// ==, !=
				utility::EnumSupport::EQUALITY,
				// &, &=, |, |=, ^, ^=, ~
				utility::EnumSupport::BIT_OPERATION
			> {};

	// 场上区域
	[[nodiscard]] constexpr auto is_field_zone(const SelectZoneWrapper zone) noexcept -> bool
	{
		constexpr static auto field = []
		{
			SelectZoneWrapper w{SelectZone::NONE};

			// 怪兽区域
			w |= SelectZone::MONSTER;
			// 额外怪兽区
			w |= SelectZone::EXTRA_MONSTER;
			// 魔陷区域
			w |= SelectZone::SPELL_TRAP;
			// // 灵摆区域(共用魔陷区域)
			// w |= SelectZone::PENDULUM;
			// // 场地魔法区域
			// w |= SelectZone::FIELD_SPELL;

			return w;
		}();

		return zone.any_of(field);
	}

	// 返回卡组时返回的位置
	enum class DeckPlacement : std::uint8_t
	{
		// 放在卡组顶端
		TOP,
		// 放在卡组底端
		BOTTOM,
		// 加入卡组后洗切
		SHUFFLE,
	};

	// 非场上区域的位置序号
	using zone_sequence_type = std::uint8_t;

	enum class FieldZoneSequence : zone_sequence_type
	{
		// 怪兽区域
		MONSTER_MAIN_1 = 0,
		MONSTER_MAIN_2 = 1,
		MONSTER_MAIN_3 = 2,
		MONSTER_MAIN_4 = 3,
		MONSTER_MAIN_5 = 4,
		MONSTER_EXTRA_1 = 0,
		MONSTER_EXTRA_2 = 1,

		// 魔陷区域
		SPELL_TRAP_1 = 0,
		SPELL_TRAP_2 = 1,
		SPELL_TRAP_3 = 2,
		SPELL_TRAP_4 = 3,
		SPELL_TRAP_5 = 4,
		SPELL_TRAP_FIELD_SPELL = 5,
		PENDULUM_LEFT = SPELL_TRAP_1,
		PENDULUM_RIGHT = SPELL_TRAP_5,
	};

	constexpr auto field_monster_main_count = static_cast<std::size_t>(5);
	constexpr auto field_monster_extra_count = static_cast<std::size_t>(2);
	constexpr auto field_spell_trap_count = static_cast<std::size_t>(6);

	// 场上区域的表示形式
	enum class FieldZoneForm : std::uint8_t
	{
		FACE_UP_ATTACK,
		FACE_DOWN_ATTACK,
		FACE_UP_DEFENSE,
		FACE_DOWN_DEFENSE,
	};

	[[nodiscard]] constexpr auto is_face_up_form(const FieldZoneForm form) noexcept -> bool
	{
		return form == FieldZoneForm::FACE_UP_ATTACK || form == FieldZoneForm::FACE_UP_DEFENSE;
	}

	[[nodiscard]] constexpr auto is_face_down_form(const FieldZoneForm form) noexcept -> bool
	{
		return form == FieldZoneForm::FACE_DOWN_ATTACK || form == FieldZoneForm::FACE_DOWN_DEFENSE;
	}

	[[nodiscard]] constexpr auto is_attack_form(const FieldZoneForm form) noexcept -> bool
	{
		return form == FieldZoneForm::FACE_UP_ATTACK || form == FieldZoneForm::FACE_DOWN_ATTACK;
	}

	[[nodiscard]] constexpr auto is_defense_form(const FieldZoneForm form) noexcept -> bool
	{
		return form == FieldZoneForm::FACE_UP_DEFENSE || form == FieldZoneForm::FACE_DOWN_DEFENSE;
	}
}

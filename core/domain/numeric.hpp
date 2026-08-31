#pragma once

#include <core/utility/enum.hpp>

namespace cg::domain
{
	// 等级
	enum class Level : std::uint8_t
	{
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
	};

	class LevelWrapper : public utility::Enum<
				Level,
				// ==, !=
				utility::EnumSupport::EQUALITY,
				// >, >=, <, <=
				utility::EnumSupport::COMPARISON,
				// +, +=, -, -=
				utility::EnumSupport::ARITHMETIC
			> {};

	// 阶级
	enum class Rank : std::uint8_t
	{
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

	// 连接箭头
	enum class LinkMarker : std::uint8_t
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

	class LinkMarkerWrapper : public utility::Enum<
				LinkMarker,
				// &, &=, |, |=, ^, ^=, ~
				utility::EnumSupport::BIT_OPERATION
			> {};

	// 灵摆刻度
	enum class PendulumScale : std::uint8_t
	{
		PS0 = 0,
		PS1 = 1,
		PS2 = 2,
		PS3 = 3,
		PS4 = 4,
		PS5 = 5,
		PS6 = 6,
		PS7 = 7,
		PS8 = 8,
		PS9 = 9,
		PS10 = 10,
		PS11 = 11,
		PS12 = 12,
		PS13 = 13,
	};

	// 生命值(不允许负数,但是设置为有符号类型可以简化计算)
	using life_point_type = std::uint32_t;
	// 攻击力(不允许负数,但是设置为有符号类型可以简化计算)
	using attack_value_type = std::int32_t;
	// 防御力(不允许负数,但是设置为有符号类型可以简化计算)
	using defense_value_type = std::int32_t;

	// 未知攻击力(?)
	constexpr attack_value_type unknown_attack_value = -1;
	// 位置防御力(?)
	constexpr defense_value_type unknown_defense_value = -1;
}

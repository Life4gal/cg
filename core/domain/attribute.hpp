#pragma once

#include<core/utility/enum.hpp>

namespace cg::domain
{
	// 属性允许同时有多个
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

	class AttributeWrapper : public utility::Enum<
				Attribute,
				// ==, !=
				utility::EnumSupport::EQUALITY,
				// &, &=, |, |=, ^, ^=, ~
				utility::EnumSupport::BIT_OPERATION
			> {};
}

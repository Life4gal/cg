#pragma once

#include <cstdint>

namespace cg::domain
{
	enum class WinReason : std::uint8_t
	{
		// LP归零
		LIFE_POINTS,
		// 卡组耗尽
		DECK_OUT,
		// 特殊胜利
		EFFECT,
		// 投降
		SURRENDER,
	};
}

#pragma once

#include <cstdint>

namespace cg::domain
{
	// 玩家代号
	enum class Player : std::uint8_t
	{
		// 先攻玩家
		FIRST = 0,
		// 后攻玩家
		SECOND = 1,
	};

	constexpr std::size_t player_count = 2;

	// 玩家指代
	enum class RelativePlayer : std::uint8_t
	{
		// 自己
		SELF,
		// 对方
		OPPONENT,
	};
}

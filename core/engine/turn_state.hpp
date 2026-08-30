#pragma once

#include <core/domain/player.hpp>
#include <core/domain/turn.hpp>

namespace cg::engine
{
	using turn_index_type = std::uint32_t;

	class TurnState
	{
	public:
		// 当前回合玩家
		domain::Player player;
		// 当前回合阶段
		domain::Phase phase;
		// 当前回合数
		turn_index_type turn_index;
	};
}

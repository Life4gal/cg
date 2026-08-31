#pragma once

#include <core/domain/player.hpp>
#include <core/domain/turn.hpp>
#include <core/domain/id.hpp>

namespace cg::engine
{
	class TurnState
	{
	public:
		// 当前回合玩家
		domain::Player player;
		// 当前回合阶段
		domain::Phase phase;
		// 当前回合ID
		domain::TurnId turn_id;
	};
}

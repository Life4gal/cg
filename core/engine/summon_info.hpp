#pragma once

#include <core/domain/summon.hpp>
#include <core/domain/zone.hpp>
#include <core/domain/player.hpp>
#include <core/domain/turn.hpp>

#include <core/engine/collection.hpp>

namespace cg::engine
{
	class SummonInfo
	{
	public:
		// 召唤的类型
		domain::SummonKind kind;
		// 从哪里召唤
		domain::Zone from_zone;
		// 召唤的玩家
		domain::Player player;
		// 召唤的回合
		domain::TurnId turn_id;
		// 召唤使用的材料
		Group materials;

		SummonInfo() noexcept;
	};
}

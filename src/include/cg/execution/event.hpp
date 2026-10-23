#pragma once

#include <optional>
#include <vector>

#include <cg/domain/event.hpp>
#include <cg/domain/player.hpp>

#include <cg/dsl/trigger.hpp>

#include <cg/state/id.hpp>

namespace cg::execution
{
	// 结算过程中产生的事件,由流程层在连锁结算后统一开时点
	class PendingEvent
	{
	public:
		domain::EventCode code;
		state::CardInstanceId card;
	};

	class PendingEventList : public std::vector<PendingEvent> {};

	// 事件快照:source/card是触发身份,player/amount是可选事件载荷
	// - 不是"仅触发键":载荷需随触发链保留到结算
	// - 同一时点可能同时有多个事件(如一次效果同时破坏两张卡)
	class EventWindow
	{
	public:
		dsl::TriggerSource source;
		std::optional<state::CardInstanceId> card;

		// 事件关联玩家(如 BATTLE_DAMAGE 的受伤玩家;绝对玩家)
		std::optional<domain::Player> player;
		// 事件数值(如 BATTLE_DAMAGE 的伤害)
		std::optional<std::int32_t> amount;
	};

	class EventWindowList : public std::vector<EventWindow> {};
}

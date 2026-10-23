#pragma once

#include <optional>
#include <vector>

#include <cg/domain/event.hpp>

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

	// 一个事件时点:触发源 + 事件关联卡
	// - 同一时点可能同时发生多个事件(如一次效果同时破坏两张卡)
	// - 流程层把同一批事件一起交给触发收集,组成同一组连锁
	class EventWindow
	{
	public:
		dsl::TriggerSource source;
		std::optional<state::CardInstanceId> card;
	};

	class EventWindowList : public std::vector<EventWindow> {};
}

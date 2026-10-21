#pragma once

#include <vector>

#include <cg/domain/event.hpp>

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
}

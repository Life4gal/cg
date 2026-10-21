#pragma once

#include <optional>

#include <cg/dsl/trigger.hpp>

#include <cg/state/id.hpp>

namespace cg::flow
{
	class Window
	{
	public:
		dsl::TriggerSource source;
		std::optional<state::CardInstanceId> card;
	};
}

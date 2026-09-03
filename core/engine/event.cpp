#include <core/engine/event.hpp>

#include <algorithm>

namespace cg::engine
{
	Event::Event() noexcept
		: type{static_cast<domain::EventType>(0)},
		  reason_player{domain::PlayerReference::FIRST},
		  // reason_card{},
		  // reason_effect{},
		  reason{domain::Reason::NORMAL_SUMMON}
	{
		//
	}

	Events::Events() noexcept = default;
	// 	: events_{}
	// {
	// 	//
	// }

	auto Events::raise(const Event& event) noexcept -> void
	{
		events_.push_back(event);
	}

	auto Events::triggered(const domain::EventType type) const noexcept -> bool
	{
		return std::ranges::contains(events_, type, &Event::type);
	}
}

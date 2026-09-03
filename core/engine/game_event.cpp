#include <core/engine/game_event.hpp>

namespace cg::engine
{
	GameEvents::GameEvents() noexcept = default;
	// 	: events_{}
	// {
	// 	//
	// }

	auto GameEvents::emit(domain::GameEvent event) noexcept -> void
	{
		events_.push_back(std::move(event));
	}
}

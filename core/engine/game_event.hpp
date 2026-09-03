#pragma once

#include <vector>

#include <core/domain/game_event.hpp>

namespace cg::engine
{
	class GameEvents
	{
	public:
		using events_type = std::vector<domain::GameEvent>;

	private:
		events_type events_;

	public:
		GameEvents() noexcept;

		auto emit(domain::GameEvent event) noexcept -> void;
	};
}

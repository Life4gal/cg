#include <core/engine/state.hpp>

namespace cg::engine
{
	State::State() noexcept
		: controller{domain::Player::FIRST},
		  // zone{},
		  reason_player{domain::Player::FIRST},
		  // reason_card{},
		  // reason_effect{},
		  reason{domain::Reason::NORMAL_SUMMON}
	{
		//
	}
}

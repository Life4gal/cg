#include <core/engine/summon_info.hpp>

namespace cg::engine
{
	SummonInfo::SummonInfo() noexcept
		: kind{domain::SummonKind::NORMAL},
		  // from_zone{},
		  player{domain::Player::FIRST},
		  turn_id{static_cast<domain::TurnId>(0)}
	// materials{}
	{
		//	
	}
}

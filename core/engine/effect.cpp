#include <core/engine/effect.hpp>

namespace cg::engine
{
	Effect::Effect(Duel& duel, const domain::EffectInstanceId instance_id, Card& owner) noexcept
		: duel_{duel},
		  instance_id_{instance_id},
		  owner_{owner}
	{
		//
	}

	auto Effect::instance_id() const noexcept -> domain::EffectInstanceId
	{
		return instance_id_;
	}
}

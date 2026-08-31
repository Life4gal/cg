#include <core/engine/effect.hpp>

namespace cg::engine
{
	auto Effect::instance_id() const noexcept -> domain::EffectInstanceId
	{
		return instance_id_;
	}
}

#include <core/engine/effect.hpp>

namespace cg::engine
{
	auto Effect::instance_id() const noexcept -> effect_instance_id_type
	{
		return instance_id_;
	}
}

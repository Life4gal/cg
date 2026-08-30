#pragma once

#include <core/engine/effect_reference.hpp>

namespace cg::engine
{
	class Effect
	{
	public:
		//

	private:
		effect_instance_id_type instance_id_;

		//

	public:
		[[nodiscard]] auto instance_id() const noexcept -> effect_instance_id_type;
	};
}

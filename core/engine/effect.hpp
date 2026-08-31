#pragma once

#include <core/domain/id.hpp>

#include <core/engine/effect_reference.hpp>

namespace cg::engine
{
	class Effect
	{
	public:
		//

	private:
		domain::EffectInstanceId instance_id_;

		//

	public:
		[[nodiscard]] auto instance_id() const noexcept -> domain::EffectInstanceId;
	};
}

#include <core/engine/api.hpp>

#include <core/engine/duel.hpp>

namespace cg::engine
{
	auto Api::duel() const noexcept -> Duel&
	{
		return duel_.get();
	}

	Api::Api(Duel& duel) noexcept
		: duel_{duel} {}
}

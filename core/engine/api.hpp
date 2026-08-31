#pragma once

#include <functional>

namespace cg::engine
{
	class Duel;

	class Api
	{
	public:
		//

	private:
		std::reference_wrapper<Duel> duel_;

		[[nodiscard]] auto duel() const noexcept -> Duel&;

	public:
		explicit Api(Duel& duel) noexcept;
	};
}

#pragma once

#include <core/engine/card_reference.hpp>

namespace cg::engine
{
	// card_a < card_b
	class CardOrder
	{
	public:
		[[nodiscard]] static auto operator()(const Card& lhs, const Card& rhs) noexcept -> bool;

		[[nodiscard]] static auto operator()(const CardReference& lhs, const CardReference& rhs) noexcept -> bool;
	};
}

#include <core/engine/card_order.hpp>

#include <core/engine/card.hpp>

namespace cg::engine
{
	auto CardOrder::operator()(const Card& lhs, const Card& rhs) noexcept -> bool
	{
		return lhs.instance_id() < rhs.instance_id();
	}

	auto CardOrder::operator()(const CardReference& lhs, const CardReference& rhs) noexcept -> bool
	{
		return operator()(lhs.get(), rhs.get());
	}
}

#include <core/card_view.hpp>

#include <core/card.hpp>

namespace cg
{
	auto CardView::Sorter::operator()(const Card& lhs, const Card& rhs) noexcept -> bool
	{
		return lhs.instance_id < rhs.instance_id;
	}
}

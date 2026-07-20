#include <core/view.hpp>

#include <core/card.hpp>

namespace cg::core
{
	auto View::Sorter::operator()(const Card& lhs, const Card& rhs) noexcept -> bool
	{
		return lhs.instance_id < rhs.instance_id;
	}
}

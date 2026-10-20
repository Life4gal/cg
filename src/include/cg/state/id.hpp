#pragma once

#include <limits>

#include <cg/utility/opacity.hpp>

namespace cg::state
{
	// 卡牌实例ID
	class CardInstanceId : public utility::Opacity<class TagCardInstance, std::uint32_t, std::numeric_limits<std::uint32_t>::max()>
	{
	public:
		using Opacity::Opacity;
	};
}

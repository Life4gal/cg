#pragma once

#include <cg/utility/opacity.hpp>

namespace cg::state
{
	// 卡牌实例ID
	class CardInstanceId : public utility::Opacity<class TagCardInstance>
	{
	public:
		using Opacity::Opacity;
	};
}

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

	// 效果ID
	// TODO: 已移动到dsl/ids.hpp
	class EffectId : public utility::Opacity<class TagEffect>
	{
	public:
		using Opacity::Opacity;
	};
}

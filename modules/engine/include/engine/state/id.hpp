#pragma once

#include <engine/opacity.hpp>

namespace cg::engine
{
	// 卡密
	class CardCode : public Opacity<class CardTag>
	{
	public:
		using Opacity::Opacity;
	};

	// 卡牌实例ID
	class CardInstanceId : public Opacity<class CardInstanceTag>
	{
	public:
		using Opacity::Opacity;
	};

	// 效果ID
	class EffectId : public Opacity<class EffectTag>
	{
	public:
		using Opacity::Opacity;
	};
}

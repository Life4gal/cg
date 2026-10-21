#pragma once

#include <cg/execution/context.hpp>
#include <cg/execution/effect.hpp>

namespace cg::execution
{
	// 一条连锁(激活时产生的绑定保存在context中)
	class ChainLink
	{
	public:
		TriggeredEffect effect;
		Context context;
	};

	// 连锁;下标 0 = Chain Link 1,结算时逆序
	class Chain : public std::vector<ChainLink> {};
}

#pragma once

#include <functional>
#include <map>
#include <vector>

#include <cg/domain/player.hpp>

#include <cg/dsl/id.hpp>

#include <cg/state/id.hpp>

#include <cg/execution/value.hpp>

namespace cg::state
{
	class Game;
}

namespace cg::execution
{
	class Context
	{
	public:
		// 选择回调:从候选中选出 [min, max] 张;缺省取前 min 张
		using selector_type = std::function<auto(const Group& candidates, std::size_t min, std::size_t max) /*noexcept*/ -> Group>;

		std::reference_wrapper<state::Game> game;

		// 效果控制者
		domain::Player controller;
		// 效果来源卡
		state::CardInstanceId source;
		// 效果帧绑定
		std::map<dsl::BindingId, Value> bindings;
		// 元素作用域栈(`$`)
		std::vector<state::CardInstanceId> element_scopes;
		// 选择回调(测试/UI 注入)
		selector_type selector;
	};
}

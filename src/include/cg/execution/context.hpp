#pragma once

#include <functional>
#include <map>
#include <optional>
#include <vector>

#include <cg/domain/player.hpp>

#include <cg/dsl/id.hpp>
#include <cg/dsl/trigger.hpp>

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

		// ================================================================
		// 触发上下文
		// ================================================================

		// 当前事件/期间
		std::optional<dsl::TriggerSource> trigger_source;
		// 事件关联卡
		std::optional<state::CardInstanceId> event_card;
		// 事件玩家
		std::optional<domain::Player> event_player;
	};
}

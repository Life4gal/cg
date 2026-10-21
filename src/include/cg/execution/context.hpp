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
		using bindings_type = std::map<dsl::BindingId, Value>;
		using scopes_type = std::vector<state::CardInstanceId>;
		// 选择回调:从候选中选出 [min, max] 张;缺省取前 min 张
		using selector_type = std::function<auto(const Group& candidates, std::size_t min, std::size_t max) /*noexcept*/ -> Group>;

		std::reference_wrapper<state::Game> game;

		// 效果控制者
		domain::Player controller;
		// 效果来源卡
		state::CardInstanceId source;
		// 效果帧绑定
		bindings_type bindings;
		// 元素作用域栈(`$`)
		scopes_type element_scopes;
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

		constexpr Context(
			state::Game& game,
			const domain::Player controller,
			const state::CardInstanceId source = state::CardInstanceId{state::CardInstanceId::invalid_id},
			bindings_type bindings = {},
			scopes_type element_scopes = {},
			selector_type selector = {}
		) noexcept
			: game{game},
			  controller{controller},
			  source{source},
			  bindings{std::move(bindings)},
			  element_scopes{std::move(element_scopes)},
			  selector{std::move(selector)},
			  trigger_source{std::nullopt},
			  event_card{std::nullopt},
			  event_player{std::nullopt} {}
	};
}

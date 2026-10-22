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
	class CardStateResolver;

	// 一个效果发动/结算(连锁link)的执行帧
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
		// 元素作用域栈($)
		scopes_type element_scopes;

		// 选择回调(测试/UI注入)
		selector_type selector;

		// 有效值查询(为空时回退 CardInstance::origin_*)
		CardStateResolver* card_state;

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
			selector_type selector = {}
		) noexcept
			: game{game},
			  controller{controller},
			  source{source},
			  bindings{},
			  element_scopes{},
			  selector{std::move(selector)},
			  card_state{nullptr},
			  trigger_source{std::nullopt},
			  event_card{std::nullopt},
			  event_player{std::nullopt} {}

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;

		Context(Context&&) noexcept = default;
		Context& operator=(Context&&) noexcept = default;

		~Context() noexcept = default;

		// 为新效果/新 link 创建子帧:共享环境,局部绑定与作用域为空
		[[nodiscard]] auto make_child(
			const domain::Player child_controller,
			const state::CardInstanceId child_source
		) const noexcept -> Context
		{
			Context child{game.get(), child_controller, child_source};

			child.selector = selector;
			child.card_state = card_state;
			child.trigger_source = trigger_source;
			child.event_card = event_card;
			child.event_player = event_player;

			return child;
		}
	};
}

#pragma once

#include <cstddef>
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
	class Chain;

	// 一个效果发动/结算(连锁link)的执行帧
	// - 基础帧:game / controller / source / frame
	// - 环境:card_state / selector(可空)
	// - 可选子上下文:trigger(触发时点) / chain(连锁结算位置)
	class Context
	{
	public:
		// 选择回调:从候选中选出 [min, max] 张
		using selector_type = std::function<auto(const Group& candidates, std::size_t min, std::size_t max) /*noexcept*/ -> Group>;

		// 执行帧局部状态
		class Frame
		{
		public:
			using bindings_type = std::map<dsl::BindingId, Value>;
			using scopes_type = std::vector<state::CardInstanceId>;

			// 效果帧绑定
			bindings_type bindings;
			// 元素作用域栈($)
			scopes_type element_scopes;
		};

		// 触发时点(仅触发收集/结算时存在)
		class TriggerInfo
		{
		public:
			// 当前事件/期间
			dsl::TriggerSource source;
			// 事件关联卡
			std::optional<state::CardInstanceId> event_card;
		};

		// 连锁结算位置(仅 Interpreter::resolve(Chain&) 时存在)
		class ChainCursor
		{
		public:
			Chain* chain;
			// 当前 link 在 chain 中的下标
			std::size_t link_index;
		};

		std::reference_wrapper<state::Game> game;

		// 效果控制者
		domain::Player controller;
		// 效果来源卡
		state::CardInstanceId source;

		// 执行帧(绑定/作用域)
		Frame frame;

		// 有效值查询(为空时回退CardInstance::origin_*)
		CardStateResolver* card_state;
		// 选择回调;为空 = 未注入,选择类原语失败
		const selector_type* selector;

		// 触发时点;空 = 非触发上下文
		std::optional<TriggerInfo> trigger;
		// 连锁结算位置;空 = 非结算上下文
		std::optional<ChainCursor> chain;

		constexpr Context(
			state::Game& game,
			const domain::Player controller,
			const state::CardInstanceId source = state::CardInstanceId{state::CardInstanceId::invalid_id}
		) noexcept
			: game{game},
			  controller{controller},
			  source{source},
			  frame{},
			  card_state{nullptr},
			  selector{nullptr},
			  trigger{std::nullopt},
			  chain{std::nullopt} {}

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;

		Context(Context&&) noexcept = default;
		Context& operator=(Context&&) noexcept = default;

		~Context() noexcept = default;

		// 为新效果/新 link 创建子帧:共享环境,复制触发时点,局部帧为空
		[[nodiscard]] auto make_child(
			const domain::Player child_controller,
			const state::CardInstanceId child_source
		) const noexcept -> Context
		{
			Context child{game.get(), child_controller, child_source};

			child.card_state = card_state;
			child.selector = selector;
			child.trigger = trigger;

			return child;
		}
	};
}

#pragma once

#include <functional>
#include <optional>

#include <cg/domain/player.hpp>

#include <cg/execution/event.hpp>
#include <cg/execution/frame.hpp>

namespace cg::state
{
	class Game;
}

namespace cg::execution
{
	class CardStateResolver;
	class Chain;
	class Selector;

	// 一个效果发动/结算(连锁link)的执行帧
	// - 基础帧:game / controller / source
	// - 求值帧:frame(绑定/作用域)
	// - 环境:card_state / selector(必选)
	// - 可选子上下文:event(触发时点) / chain(连锁结算位置)
	class Context
	{
	public:
		// 连锁结算位置(仅 Interpreter::resolve(Chain&) 时存在)
		class ChainCursor
		{
		public:
			std::reference_wrapper<Chain> chain;
			// 当前 link 在 chain 中的下标
			std::size_t link_index;
		};

		std::reference_wrapper<state::Game> game;
		// 选择器(由 Duel/驱动持有并保证生命周期)
		std::reference_wrapper<Selector> selector;

		// 效果控制者
		domain::Player controller;
		// 效果来源卡
		state::CardInstanceId source;

		// 执行帧(绑定/作用域);仅执行层内部使用
		Frame frame;

		// 有效值查询(为空时回退CardInstance::origin_*;测试路径可能为空)
		CardStateResolver* card_state;

		// 事件快照;condition / activation / resolution 均可查询 event_*
		std::optional<EventWindow> event;
		// 连锁结算位置;仅 Interpreter::resolve(Chain&) 设置
		std::optional<ChainCursor> chain;

		constexpr Context(
			state::Game& game,
			Selector& selector,
			const domain::Player controller,
			const state::CardInstanceId source = state::CardInstanceId{state::CardInstanceId::invalid_id}
		) noexcept
			: game{game},
			  selector{selector},
			  controller{controller},
			  source{source},
			  frame{},
			  card_state{nullptr},
			  event{std::nullopt},
			  chain{std::nullopt} {}

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;

		Context(Context&&) noexcept = default;
		Context& operator=(Context&&) noexcept = default;

		~Context() noexcept = default;

		// 为新效果/新 link 创建子帧:共享环境,复制事件时点,局部求值帧为空
		[[nodiscard]] auto make_child(
			const domain::Player child_controller,
			const state::CardInstanceId child_source
		) const noexcept -> Context
		{
			Context child{game.get(), selector.get(), child_controller, child_source};

			child.card_state = card_state;
			child.event = event;

			return child;
		}
	};
}

#pragma once

#include <functional>
#include <optional>

#include <cg/domain/player.hpp>

#include <cg/execution/event.hpp>
#include <cg/execution/frame.hpp>

namespace cg::execution
{
	class Chain;

	// 一个效果发动/结算(连锁link)的执行帧
	// - 基础帧:controller / source
	// - 求值帧:frame(绑定/作用域)
	// - 可选子上下文:event(触发时点)/chain(连锁结算位置)
	class Context
	{
	public:
		// 连锁位置
		// - 激活时:chain指向已有连锁,link_index = 追加位置(chain.size())
		// - 结算时:Interpreter::resolve设置当前link的真实下标
		class ChainCursor
		{
		public:
			std::reference_wrapper<Chain> chain;
			// 激活时是追加位置;结算时是当前link下标
			std::size_t link_index;
		};

		// 效果控制者
		domain::Player controller;
		// 效果来源卡
		state::CardInstanceId source;

		// 执行帧(绑定/作用域);仅执行层内部使用
		Frame frame;

		// 事件快照;condition / activation / resolution 均可查询 event_*
		std::optional<EventWindow> event;

		// 连锁位置;Duel在激活前设置,Interpreter::resolve在结算时覆盖为真实下标
		std::optional<ChainCursor> chain;

		Context(
			const domain::Player controller,
			const state::CardInstanceId source
		) noexcept
			: controller{controller},
			  source{source},
			  event{std::nullopt},
			  chain{std::nullopt} {}

		Context(
			const domain::Player controller,
			const state::CardInstanceId source,
			const EventWindow& event
		) noexcept
			: controller{controller},
			  source{source},
			  event{event},
			  chain{std::nullopt} {}

		Context(
			const domain::Player controller,
			const state::CardInstanceId source,
			const ChainCursor& chain
		) noexcept
			: controller{controller},
			  source{source},
			  event{std::nullopt},
			  chain{chain} {}
	};
}

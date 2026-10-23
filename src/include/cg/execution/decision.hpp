#pragma once

#include <span>

#include <cg/domain/player.hpp>

#include <cg/dsl/id.hpp>

#include <cg/execution/event.hpp>
#include <cg/execution/value.hpp>

#include <cg/state/id.hpp>

namespace cg::execution
{
	// 一个可选择的触发效果
	class TriggerChoice
	{
	public:
		// 触发者
		domain::Player controller;
		// 效果来源卡
		state::CardInstanceId source;
		// 效果稳定标识
		dsl::EffectId effect;
		// 命中的事件时点(触发源 + 事件关联卡)
		EventWindow window;
		// 选发/必发
		bool optional;
	};

	// 当前需要外部驱动决策的一组触发(SEGOC的同一优先级组)
	class TriggerDecision
	{
	public:
		// 需要做出选择的玩家
		domain::Player player;
		// true = 选发组(可选子集);false = 必发组(必须全选,仅决定顺序)
		bool optional;
		// 候选;驱动返回的索引指向此列表,顺序即发动顺序
		// - 仅在本次回调期间有效,驱动不得保存 span
		std::span<const TriggerChoice> candidates;
	};

	class Selector
	{
	public:
		Selector() noexcept = default;

		Selector(const Selector&) noexcept = delete;
		auto operator=(const Selector&) noexcept -> Selector& = delete;
		Selector(Selector&&) noexcept = default;
		auto operator=(Selector&&) noexcept -> Selector& = default;

		virtual ~Selector() noexcept = default;


		[[nodiscard]] virtual auto select_cards(domain::Player player, const Group& candidates, std::size_t min, std::size_t max) noexcept -> Group = 0;

		// 必发组必须返回全部索引;选发组可返回空
		[[nodiscard]] virtual auto choose_triggers(const TriggerDecision& decision) noexcept -> std::vector<std::size_t> = 0;
	};
}

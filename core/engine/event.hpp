#pragma once

#include <core/domain/event.hpp>
#include <core/domain/player.hpp>
#include <core/domain/reason.hpp>

#include <core/engine/card_reference.hpp>
#include <core/engine/effect_reference.hpp>

namespace cg::engine
{
	class Event
	{
	public:
		domain::EventType type;

		// 触发事件的原因玩家
		domain::PlayerReference reason_player;
		// 触发事件的原因卡
		CardOptional reason_card;
		// 触发事件的原因效果
		EffectOptional reason_effect;
		// 触发事件的原因
		domain::ReasonWrapper reason;

		// todo: 额外成员

		Event() noexcept;
	};

	class Events
	{
	public:
		using events_type = std::vector<Event>;

	private:
		events_type events_;

	public:
		Events() noexcept;

		// 触发事件
		auto raise(const Event& event) noexcept -> void;

		// 目标类型事件是否被触发
		[[nodiscard]] auto triggered(domain::EventType type) const noexcept -> bool;
	};
}

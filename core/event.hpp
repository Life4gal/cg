#pragma once

#include <compare>

#include <core/state.hpp>

namespace cg::core
{
	class View;

	enum class EventType : std::uint32_t
	{
		// 被破坏
		DESTROY,
		// 被返回手牌
		TO_HAND,
		// 被返回卡组
		TO_DECK,
		// 被送墓
		TO_GRAVE,
		// 被除外
		REMOVED,
		// 被解放
		RELEASE,
		// 被(从手牌)丢弃
		DISCARD,
	};

	class Event
	{
	public:
		// 触发此事件的玩家
		Player player;
		// 触发此事件的原因
		Reason reason;
		// 事件类型
		EventType type;

		// 触发此事件的卡片
		std::reference_wrapper<const Card> trigger_card;
		// 事件涉及的所有卡片
		std::reference_wrapper<const View> cards;
		// 引发此事件的原因效果
		std::reference_wrapper<const Effect> reason_effect;

		[[nodiscard]] constexpr auto operator<=>(const Event& other) const noexcept -> std::strong_ordering
		{
			return std::to_underlying(type) <=> std::to_underlying(other.type);
		}

		[[nodiscard]] constexpr auto operator==(const Event& other) const noexcept -> bool
		{
			// 同一个原因
			if (reason != other.reason)
			{
				return false;
			}

			// 同一个事件类型
			if (type != other.type)
			{
				return false;
			}

			// 同一个触发者
			if (&trigger_card.get() != &other.trigger_card.get())
			{
				return false;
			}

			// 涉及相同的卡片
			if (&cards.get() != &other.cards.get())
			{
				return false;
			}

			return true;
		}
	};
}

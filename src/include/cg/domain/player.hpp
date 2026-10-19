#pragma once

#include <utility>

namespace cg::domain
{
	// 玩家绝对代号
	enum class Player : std::uint8_t
	{
		// 先攻玩家
		FIRST = 0,
		// 后攻玩家
		SECOND = 1,
	};

	[[nodiscard]] constexpr auto opponent_of(const Player player) noexcept -> Player
	{
		return static_cast<Player>(1 - std::to_underlying(player));
	}

	static_assert(opponent_of(Player::FIRST) == Player::SECOND);
	static_assert(opponent_of(Player::SECOND) == Player::FIRST);

	// 玩家指代代号
	enum class RelativePlayer : std::uint8_t
	{
		// 自己
		SELF,
		// 对方
		OPPONENT,
	};

	// 玩家选择子(效果脚本中的玩家引用;相对效果控制者)
	enum class PlayerSelector : std::uint8_t
	{
		// 效果控制者
		SELF,
		// 效果控制者的对手
		OPPONENT,
		// 事件玩家(事件的发生者)
		EVENT_PLAYER,
		// 原因玩家(移动/变化的原因来源方)
		REASON_PLAYER,
		// 触发玩家
		TRIGGERING_PLAYER,
	};
}

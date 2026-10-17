#pragma once

#include <cstdint>

namespace cg::domain
{
	// 操作/行为被拒绝的理由
	enum class RejectReason : std::uint8_t
	{
		// 不在你的回合
		NOT_YOUR_TURN,
		// 不正确的阶段
		WRONG_PHASE,
		// 怪兽/魔陷区被占用
		ZONE_OCCUPIED,
		// 怪兽/魔陷区被禁用
		ZONE_LOCKED,
		// 错误的表示形式(例如连接怪兽选择守备表示)
		WRONG_POSITION,
		// 错误的目标
		INVALID_TARGET,
		// 无法发动
		CANNOT_ACTIVATE,
		// 被取消
		CANCELLED,
		// 超时
		TIMEOUT,
	};
}

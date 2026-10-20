#pragma once

#include <variant>
#include <vector>

#include <cg/domain/event.hpp>
#include <cg/domain/phase.hpp>

namespace cg::dsl
{
	// ================================================================
	// 触发源:事件 | 期间
	// - 事件:`domain::EventCode`
	// - 期间:`domain::Phase` / `BattleStep` / `DamageStep`
	// ================================================================

	// 期间引用(阶段/战斗步骤/伤害步骤)
	class PeriodRef : public std::variant<domain::Phase, domain::BattleStep, domain::DamageStep> {};

	// 触发源 = 事件 | 期间
	class TriggerSource : public std::variant<domain::EventCode, PeriodRef> {};

	// 触发源列表(或关系;至少一个;如「召唤·特殊召唤成功时」)
	class TriggerSourceList : public std::vector<TriggerSource> {};

	enum class TriggerTiming : std::uint8_t
	{
		// 触发动作须为该时点最后发生的事,否则错过时点(即"xxx时")
		WHEN,
		// 不受此限(即"xxx场合")
		IF,
	};
}

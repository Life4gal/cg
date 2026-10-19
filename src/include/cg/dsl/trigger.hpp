#pragma once

#include <variant>

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
}

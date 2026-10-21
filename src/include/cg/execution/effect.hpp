#pragma once

#include <functional>
#include <vector>

#include <cg/dsl/effect.hpp>

#include <cg/state/id.hpp>

namespace cg::execution
{
	// 一条已匹配的触发效果
	class TriggeredEffect
	{
	public:
		state::CardInstanceId card;
		domain::Player controller;
		dsl::EffectId effect;
		std::reference_wrapper<const dsl::ActivatedEffect> activated;
		std::reference_wrapper<const dsl::Trigger> trigger;
	};

	class TriggeredEffectList : public std::vector<TriggeredEffect> {};
}

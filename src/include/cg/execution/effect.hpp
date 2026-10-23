#pragma once

#include <functional>
#include <vector>

#include <cg/domain/player.hpp>

#include <cg/execution/event.hpp>

namespace cg
{
	namespace state
	{
		class CardInstance;
	}

	namespace dsl
	{
		class Effect;
		class ActivatedEffect;
		class Trigger;
	}
}

namespace cg::execution
{
	// 已触发的效果候选:由Interpreter生成,Duel使用
	class TriggeredEffect
	{
	public:
		domain::Player controller;
		std::reference_wrapper<const state::CardInstance> instance;

		// dsl::ActivatedEffect + dsl::Trigger
		std::reference_wrapper<const dsl::Effect> effect;
		// 可以从effect读取,不过保存也无妨
		std::reference_wrapper<const dsl::ActivatedEffect> activated;
		std::reference_wrapper<const dsl::Trigger> trigger;

		// 触发事件快照(保留到结算,供event_* 查询)
		EventWindow window;
	};

	class TriggeredEffectList : public std::vector<TriggeredEffect> {};
}

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
	// 一条已匹配的触发效果
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

		// 命中的事件时点(同一效果可能被多个同时事件满足,取首个)
		EventWindow window;
	};

	class TriggeredEffectList : public std::vector<TriggeredEffect> {};
}

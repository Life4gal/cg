#pragma once

#include <map>

#include <cg/domain/player.hpp>

#include <cg/dsl/id.hpp>

#include <cg/state/id.hpp>

#include <cg/execution/value.hpp>

namespace cg::state
{
	class Game;
}

namespace cg::execution
{
	class Context
	{
	public:
		std::reference_wrapper<state::Game> game;

		// 效果控制者
		domain::Player controller;
		// 效果来源卡
		state::CardInstanceId source;
		// 效果帧绑定
		std::map<dsl::BindingId, Value> bindings;
	};
}

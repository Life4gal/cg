#pragma once

#include <functional>

#include <cg/domain/card.hpp>

namespace cg::state
{
	class CardInstance;
	class Game;
}

namespace cg::execution
{
	class Interpreter;
	class ScriptRegistry;
}

namespace cg::flow
{
	// 派生数值:印刷值 + 持续效果修正(后续可在此加缓存/分层)
	class StatResolver
	{
	public:
		//

	private:
		std::reference_wrapper<state::Game> game_;
		std::reference_wrapper<const execution::ScriptRegistry> scripts_;
		std::reference_wrapper<execution::Interpreter> interpreter_;

	public:
		StatResolver(state::Game& game, const execution::ScriptRegistry& scripts, execution::Interpreter& interpreter) noexcept;

		[[nodiscard]] auto attack(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>;
		[[nodiscard]] auto defense(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>;
	};
}

#pragma once

#include <functional>

#include <cg/domain/card.hpp>
#include <cg/domain/effect.hpp>

#include <cg/utility/enum.hpp>

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
		[[nodiscard]] auto protections(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::Protection>;
	};
}

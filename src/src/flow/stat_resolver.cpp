#include <cg/flow/stat_resolver.hpp>

#include <cg/state/card.hpp>
#include <cg/execution/interpreter.hpp>
#include <cg/execution/script_registry.hpp>

namespace cg::flow
{
	StatResolver::StatResolver(
		state::Game& game,
		const execution::ScriptRegistry& scripts,
		execution::Interpreter& interpreter
	) noexcept
		: game_{game},
		  scripts_{scripts},
		  interpreter_{interpreter} {}

	auto StatResolver::attack(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		const auto base = card.attack();

		execution::Context context{game_.get(), card.controller()};
		const auto bonus = interpreter_.get().static_numeric_bonus(scripts_.get(), card.id(), domain::NumericProperty::ATTACK, context);

		return static_cast<std::underlying_type_t<domain::AttackDefense>>(base + bonus);
	}

	auto StatResolver::defense(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		const auto base = card.defense();

		execution::Context context{game_.get(), card.controller()};
		const auto bonus = interpreter_.get().static_numeric_bonus(scripts_.get(), card.id(), domain::NumericProperty::DEFENSE, context);

		return static_cast<std::underlying_type_t<domain::AttackDefense>>(base + bonus);
	}

	auto StatResolver::protections(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::Protection>
	{
		execution::Context context{game_.get(), card.controller()};

		return interpreter_.get().static_protections(scripts_.get(), card.id(), context);
	}
}

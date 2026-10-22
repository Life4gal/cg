#include <cg/execution/card_state_resolver.hpp>

#include <cg/state/game.hpp>
#include <cg/execution/interpreter.hpp>

namespace cg::execution
{
	CardStateResolver::CardStateResolver(
		state::Game& game,
		const ScriptRegistry& scripts,
		Interpreter& interpreter
	) noexcept
		: game_{game},
		  scripts_{scripts},
		  interpreter_{interpreter} {}

	auto CardStateResolver::effective_code(const state::CardInstance& card) const noexcept -> domain::CardCode
	{
		std::ignore = this;

		return card.origin_code();
	}

	auto CardStateResolver::is_card_type(const state::CardInstance& card, const domain::CardType type) const noexcept -> bool
	{
		std::ignore = this;

		return card.origin_is_card_type(type);
	}

	auto CardStateResolver::is_card_subtype(const state::CardInstance& card, const domain::CardSubtype subtype) const noexcept -> bool
	{
		std::ignore = this;

		return card.origin_is_card_subtype(subtype);
	}

	auto CardStateResolver::is_card_subtype(const state::CardInstance& card, const utility::EnumMask<domain::CardSubtype> subtypes) const noexcept -> bool
	{
		std::ignore = this;

		return card.origin_is_card_subtype(subtypes);
	}

	auto CardStateResolver::attack(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		Context context{game_.get(), card.controller()};
		const auto bonus = interpreter_.get().static_numeric_bonus(scripts_.get(), card.id(), domain::NumericProperty::ATTACK, context);

		return static_cast<std::underlying_type_t<domain::AttackDefense>>(card.origin_attack() + bonus);
	}

	auto CardStateResolver::defense(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		Context context{game_.get(), card.controller()};
		const auto bonus = interpreter_.get().static_numeric_bonus(scripts_.get(), card.id(), domain::NumericProperty::DEFENSE, context);

		return static_cast<std::underlying_type_t<domain::AttackDefense>>(card.origin_defense() + bonus);
	}

	auto CardStateResolver::attributes(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::Attribute>
	{
		std::ignore = this;

		return {card.origin_attribute()};
	}

	auto CardStateResolver::race(const state::CardInstance& card) const noexcept -> domain::Race
	{
		std::ignore = this;

		return card.origin_race();
	}

	auto CardStateResolver::level(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::LevelRank>
	{
		std::ignore = this;

		return card.origin_level();
	}

	auto CardStateResolver::protections(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::Protection>
	{
		Context context{game_.get(), card.controller()};

		return interpreter_.get().static_protections(scripts_.get(), card.id(), context);
	}
}

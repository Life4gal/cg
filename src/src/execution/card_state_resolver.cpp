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
		Context context{game_.get(), card.controller()};

		return interpreter_.get().static_code(scripts_.get(), card.id(), context).value_or(card.origin_code());
	}

	auto CardStateResolver::is_card_type(const state::CardInstance& card, const domain::CardType type) const noexcept -> bool
	{
		if (card.origin_is_card_type(type))
		{
			return true;
		}

		Context context{game_.get(), card.controller()};
		return interpreter_.get().static_card_types(scripts_.get(), card.id(), context).contains(type);
	}

	auto CardStateResolver::is_card_subtype(const state::CardInstance& card, const domain::CardSubtype subtype) const noexcept -> bool
	{
		if (card.origin_is_card_subtype(subtype))
		{
			return true;
		}

		Context context{game_.get(), card.controller()};
		return interpreter_.get().static_card_subtypes(scripts_.get(), card.id(), context).contains(subtype);
	}

	auto CardStateResolver::is_card_subtype(const state::CardInstance& card, const utility::EnumMask<domain::CardSubtype> subtypes) const noexcept -> bool
	{
		if (card.origin_is_card_subtype(subtypes))
		{
			return true;
		}

		Context context{game_.get(), card.controller()};
		const auto extra = interpreter_.get().static_card_subtypes(scripts_.get(), card.id(), context);

		return (extra.value() & subtypes.value()) != 0;
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
		Context context{game_.get(), card.controller()};
		auto result = interpreter_.get().static_attributes(scripts_.get(), card.id(), context);
		result.add_with(card.origin_attribute());

		return result;
	}

	auto CardStateResolver::race(const state::CardInstance& card) const noexcept -> domain::Race
	{
		std::ignore = this;

		return card.origin_race();
	}

	auto CardStateResolver::level(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::LevelRank>
	{
		Context context{game_.get(), card.controller()};
		const auto bonus = interpreter_.get().static_level_bonus(scripts_.get(), card.id(), context);

		return static_cast<std::underlying_type_t<domain::LevelRank>>(card.origin_level() + bonus);
	}

	auto CardStateResolver::protections(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::Protection>
	{
		Context context{game_.get(), card.controller()};

		return interpreter_.get().static_protections(scripts_.get(), card.id(), context);
	}
}

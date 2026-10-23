#include <cg/execution/card_state_resolver.hpp>

#include <algorithm>
#include <ranges>

#include <cg/state/card.hpp>
#include <cg/state/game.hpp>
#include <cg/dsl/effect.hpp>
#include <cg/dsl/vocabulary.hpp>
#include <cg/execution/context.hpp>
#include <cg/execution/interpreter.hpp>
#include <cg/execution/script_registry.hpp>

namespace cg::execution
{
	auto CardStateResolver::collect_static_modifiers(
		const state::CardInstance& card,
		Context& context
	) const noexcept -> std::vector<std::reference_wrapper<const dsl::StaticModifier>>
	{
		const Interpreter::Evaluator evaluator{interpreter_.get()};

		std::vector<std::reference_wrapper<const dsl::StaticModifier>> result{};
		for (const auto& game = game_.get();
		     const auto& instance: game.instances())
		{
			if (const auto zone = game.zone_of(instance.id());
				zone != domain::Zone::MONSTER && zone != domain::Zone::EXTRA_MONSTER && zone != domain::Zone::SPELL_TRAP && zone != domain::Zone::FIELD_SPELL)
			{
				continue;
			}

			const auto* script = scripts_.get().find(instance.origin_code());
			if (script == nullptr)
			{
				continue;
			}

			for (const auto& effect: script->effects)
			{
				const auto* static_effect = std::get_if<dsl::StaticEffect>(&effect.body);
				if (static_effect == nullptr)
				{
					continue;
				}

				const auto applies = evaluator.evaluate_scoped(static_effect->apply_to.expression, card.id(), context);
				if (!applies.has_value())
				{
					continue;
				}

				if (const auto* condition = std::get_if<Boolean>(&*applies);
					condition == nullptr || !static_cast<bool>(*condition))
				{
					continue;
				}

				if (static_effect->condition.has_value())
				{
					const auto value = evaluator.evaluate(*static_effect->condition, context);
					if (!value.has_value())
					{
						continue;
					}

					if (const auto* condition = std::get_if<Boolean>(&*value);
						condition == nullptr || !static_cast<bool>(*condition))
					{
						continue;
					}
				}

				for (const auto& modifier: static_effect->modifiers)
				{
					result.emplace_back(modifier);
				}
			}
		}

		return result;
	}

	auto CardStateResolver::static_code(const state::CardInstance& card, Context& context) const noexcept -> std::optional<domain::CardCode>
	{
		const Interpreter::Evaluator evaluator{interpreter_.get()};

		for (const auto& modifier: collect_static_modifiers(card, context) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary_.get().find(modifier.primitive);
			if (definition == nullptr || definition->name != "change_name")
			{
				continue;
			}

			const auto args = evaluator.evaluate_list(modifier.args, context);
			if (!args.has_value() || args->size() != 1)
			{
				continue;
			}

			const auto* value = std::get_if<Card>(&args->front());
			if (value == nullptr)
			{
				continue;
			}

			const auto* instance = game_.get().find_instance(*value);
			if (instance == nullptr)
			{
				continue;
			}

			return instance->origin_code();
		}

		return std::nullopt;
	}

	auto CardStateResolver::static_card_types(const state::CardInstance& card, Context& context) const noexcept -> utility::EnumMask<domain::CardType>
	{
		const Interpreter::Evaluator evaluator{interpreter_.get()};

		utility::EnumMask<domain::CardType> result{};
		for (const auto& modifier: collect_static_modifiers(card, context) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary_.get().find(modifier.primitive);
			if (definition == nullptr || definition->name != "add_card_type")
			{
				continue;
			}

			const auto args = evaluator.evaluate_list(modifier.args, context);
			if (!args.has_value() || args->size() != 1)
			{
				continue;
			}

			if (const auto* value = std::get_if<Integer>(&args->front());
				value != nullptr)
			{
				const auto type = static_cast<std::underlying_type_t<domain::CardType>>(static_cast<Integer::value_type>(*value));
				result.add_with(static_cast<domain::CardType>(type));
			}
		}

		return result;
	}

	auto CardStateResolver::static_card_subtypes(const state::CardInstance& card, Context& context) const noexcept -> utility::EnumMask<domain::CardSubtype>
	{
		const Interpreter::Evaluator evaluator{interpreter_.get()};

		utility::EnumMask<domain::CardSubtype> result{};
		for (const auto& modifier: collect_static_modifiers(card, context) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary_.get().find(modifier.primitive);
			if (definition == nullptr || definition->name != "add_card_subtype")
			{
				continue;
			}

			const auto args = evaluator.evaluate_list(modifier.args, context);
			if (!args.has_value() || args->size() != 1)
			{
				continue;
			}

			if (const auto* value = std::get_if<Integer>(&args->front());
				value != nullptr)
			{
				const auto subtype = static_cast<std::underlying_type_t<domain::CardSubtype>>(static_cast<Integer::value_type>(*value));
				result.add_with(static_cast<domain::CardSubtype>(subtype));
			}
		}

		return result;
	}

	auto CardStateResolver::static_attributes(const state::CardInstance& card, Context& context) const noexcept -> utility::EnumMask<domain::Attribute>
	{
		const Interpreter::Evaluator evaluator{interpreter_.get()};

		utility::EnumMask<domain::Attribute> result{};
		for (const auto& modifier: collect_static_modifiers(card, context) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary_.get().find(modifier.primitive);
			if (definition == nullptr || definition->name != "add_attribute")
			{
				continue;
			}

			const auto args = evaluator.evaluate_list(modifier.args, context);
			if (!args.has_value() || args->size() != 1)
			{
				continue;
			}

			if (const auto* value = std::get_if<Integer>(&args->front());
				value != nullptr)
			{
				const auto attribute = static_cast<std::underlying_type_t<domain::Attribute>>(static_cast<Integer::value_type>(*value));
				result.add_with(static_cast<domain::Attribute>(attribute));
			}
		}

		return result;
	}

	auto CardStateResolver::static_level_bonus(const state::CardInstance& card, Context& context) const noexcept -> Integer::value_type
	{
		const Interpreter::Evaluator evaluator{interpreter_.get()};

		Integer::value_type total = 0;
		for (const auto& modifier: collect_static_modifiers(card, context) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary_.get().find(modifier.primitive);
			if (definition == nullptr || definition->name != "change_level")
			{
				continue;
			}

			const auto args = evaluator.evaluate_list(modifier.args, context);
			if (!args.has_value() || args->size() != 1)
			{
				continue;
			}

			if (const auto* value = std::get_if<Integer>(&args->front());
				value != nullptr)
			{
				total += static_cast<Integer::value_type>(*value);
			}
		}

		return total;
	}

	auto CardStateResolver::static_numeric_bonus(
		const state::CardInstance& card,
		const domain::NumericProperty property,
		Context& context
	) const noexcept -> Integer::value_type
	{
		const Interpreter::Evaluator evaluator{interpreter_.get()};

		Integer::value_type total = 0;
		for (const auto& modifier: collect_static_modifiers(card, context) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary_.get().find(modifier.primitive);
			if (definition == nullptr || definition->name != "add_numeric_property")
			{
				continue;
			}

			const auto args = evaluator.evaluate_list(modifier.args, context);
			if (!args.has_value() || args->size() != 2)
			{
				continue;
			}

			const auto* property_value = std::get_if<Integer>(&args->operator[](0));
			const auto* amount = std::get_if<Integer>(&args->operator[](1));
			if (property_value == nullptr || amount == nullptr)
			{
				continue;
			}

			if (const auto p = static_cast<std::underlying_type_t<domain::NumericProperty>>(static_cast<Integer::value_type>(*property_value));
				static_cast<domain::NumericProperty>(p) == property)
			{
				total += static_cast<Integer::value_type>(*amount);
			}
		}

		return total;
	}

	auto CardStateResolver::static_protections(const state::CardInstance& card, Context& context) const noexcept -> utility::EnumMask<domain::Protection>
	{
		const Interpreter::Evaluator evaluator{interpreter_.get()};

		utility::EnumMask<domain::Protection> result{};
		for (const auto& modifier: collect_static_modifiers(card, context) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary_.get().find(modifier.primitive);
			if (definition == nullptr || definition->name != "add_protection")
			{
				continue;
			}

			const auto args = evaluator.evaluate_list(modifier.args, context);
			if (!args.has_value() || args->size() != 1)
			{
				continue;
			}

			if (const auto* value = std::get_if<Integer>(&args->front());
				value != nullptr)
			{
				const auto p = static_cast<std::underlying_type_t<domain::Protection>>(static_cast<Integer::value_type>(*value));
				result.add_with(static_cast<domain::Protection>(p));
			}
		}

		return result;
	}

	CardStateResolver::CardStateResolver(
		state::Game& game,
		const ScriptRegistry& scripts,
		const dsl::Vocabulary& vocabulary,
		Interpreter& interpreter
	) noexcept
		: game_{game},
		  scripts_{scripts},
		  vocabulary_{vocabulary},
		  interpreter_{interpreter} {}

	auto CardStateResolver::effective_code(const state::CardInstance& card) const noexcept -> domain::CardCode
	{
		Context context{game_.get(), card.controller()};

		return static_code(card, context).value_or(card.origin_code());
	}

	auto CardStateResolver::card_types(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::CardType>
	{
		Context context{game_.get(), card.controller()};
		auto result = static_card_types(card, context);
		result.unite_with(card.origin_kind());

		return result;
	}

	auto CardStateResolver::card_subtypes(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::CardSubtype>
	{
		Context context{game_.get(), card.controller()};
		auto result = static_card_subtypes(card, context);
		result.unite_with(card.origin_subtypes());

		return result;
	}

	auto CardStateResolver::is_card_type(const state::CardInstance& card, const domain::CardType type) const noexcept -> bool
	{
		if (card.origin_is_card_type(type))
		{
			return true;
		}

		Context context{game_.get(), card.controller()};
		return static_card_types(card, context).contains(type);
	}

	auto CardStateResolver::is_card_subtype(const state::CardInstance& card, const domain::CardSubtype subtype) const noexcept -> bool
	{
		if (card.origin_is_card_subtype(subtype))
		{
			return true;
		}

		Context context{game_.get(), card.controller()};
		return static_card_subtypes(card, context).contains(subtype);
	}

	auto CardStateResolver::is_card_subtype(const state::CardInstance& card, const utility::EnumMask<domain::CardSubtype> subtypes) const noexcept -> bool
	{
		if (card.origin_is_card_subtype(subtypes))
		{
			return true;
		}

		Context context{game_.get(), card.controller()};
		const auto extra = static_card_subtypes(card, context);

		return (extra.value() & subtypes.value()) != 0;
	}

	auto CardStateResolver::attack(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		Context context{game_.get(), card.controller()};
		const auto bonus = static_numeric_bonus(card, domain::NumericProperty::ATTACK, context);

		return static_cast<std::underlying_type_t<domain::AttackDefense>>(card.origin_attack() + bonus);
	}

	auto CardStateResolver::defense(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		Context context{game_.get(), card.controller()};
		const auto bonus = static_numeric_bonus(card, domain::NumericProperty::DEFENSE, context);

		return static_cast<std::underlying_type_t<domain::AttackDefense>>(card.origin_defense() + bonus);
	}

	auto CardStateResolver::attributes(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::Attribute>
	{
		Context context{game_.get(), card.controller()};
		auto result = static_attributes(card, context);
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
		const auto bonus = static_level_bonus(card, context);

		return static_cast<std::underlying_type_t<domain::LevelRank>>(card.origin_level() + bonus);
	}

	auto CardStateResolver::protections(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::Protection>
	{
		Context context{game_.get(), card.controller()};

		return static_protections(card, context);
	}
}

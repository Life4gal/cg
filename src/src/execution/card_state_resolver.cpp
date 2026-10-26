#include <cg/execution/card_state_resolver.hpp>

#include <algorithm>
#include <ranges>

#include <cg/dsl/effect.hpp>
#include <cg/dsl/vocabulary.hpp>
#include <cg/execution/context.hpp>
#include <cg/execution/interpreter.hpp>
#include <cg/execution/script_registry.hpp>
#include <cg/state/card.hpp>
#include <cg/state/game.hpp>

namespace cg::execution
{
	auto CardStateResolver::collect_static_modifiers(const state::CardInstance& card) const noexcept -> std::vector<std::reference_wrapper<const dsl::StaticModifier>>
	{
		class ActiveEffectGuard
		{
		public:
			ActiveEffectGuard(std::vector<const dsl::Effect*>& stack, const dsl::Effect* effect) noexcept
				: stack_{stack}
			{
				stack_.push_back(effect);
			}

			ActiveEffectGuard(const ActiveEffectGuard&) noexcept = delete;
			auto operator=(const ActiveEffectGuard&) noexcept -> ActiveEffectGuard& = delete;
			ActiveEffectGuard(ActiveEffectGuard&&) noexcept = delete;
			auto operator=(ActiveEffectGuard&&) noexcept -> ActiveEffectGuard& = delete;

			~ActiveEffectGuard() noexcept
			{
				stack_.pop_back();
			}

		private:
			std::vector<const dsl::Effect*>& stack_;
		};

		const auto& game = game_.get();
		auto& interpreter = interpreter_.get();

		std::vector<std::reference_wrapper<const dsl::StaticModifier>> result{};
		for (const auto& instance: game.instances())
		{
			const auto zone = game.zone_of(instance.id());

			const auto* script = scripts_.get().find(instance.origin_code());
			if (script == nullptr)
			{
				continue;
			}

			for (const auto& effect: script->effects)
			{
				const auto* static_effect = std::get_if<dsl::StaticEffect>(&effect.body);
				if (
					static_effect == nullptr ||
					!static_effect->zones.contains(zone) ||
					std::ranges::find(active_effects_, &effect) != active_effects_.end()
				)
				{
					continue;
				}

				const auto applies = [&]() noexcept -> bool
				{
					const ActiveEffectGuard guard{active_effects_, &effect};
					Context static_context{instance.controller(), instance.id()};

					const auto value = interpreter.evaluate_scoped(static_effect->apply_to.expression, card.id(), static_context);
					if (!value.has_value())
					{
						return false;
					}

					if (const auto* condition = std::get_if<Boolean>(&*value);
						condition == nullptr || !static_cast<bool>(*condition))
					{
						return false;
					}

					if (!static_effect->condition.has_value())
					{
						return true;
					}

					const auto condition = interpreter.evaluate_scoped(*static_effect->condition, card.id(), static_context);
					if (!condition.has_value())
					{
						return false;
					}

					const auto* condition_value = std::get_if<Boolean>(&*condition);
					return condition_value != nullptr && static_cast<bool>(*condition_value);
				}();

				if (!applies)
				{
					continue;
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
		const auto& vocabulary = vocabulary_.get();
		auto& interpreter = interpreter_.get();

		for (const auto& modifier: collect_static_modifiers(card) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary.find(modifier.primitive);
			if (definition == nullptr || definition->name != "change_name")
			{
				continue;
			}

			const auto args = interpreter.evaluate_list(modifier.args, context);
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
		const auto& vocabulary = vocabulary_.get();
		auto& interpreter = interpreter_.get();

		utility::EnumMask<domain::CardType> result{};
		for (const auto& modifier: collect_static_modifiers(card) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary.find(modifier.primitive);
			if (definition == nullptr || definition->name != "add_card_type")
			{
				continue;
			}

			const auto args = interpreter.evaluate_list(modifier.args, context);
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
		const auto& vocabulary = vocabulary_.get();
		auto& interpreter = interpreter_.get();

		utility::EnumMask<domain::CardSubtype> result{};
		for (const auto& modifier: collect_static_modifiers(card) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary.find(modifier.primitive);
			if (definition == nullptr || definition->name != "add_card_subtype")
			{
				continue;
			}

			const auto args = interpreter.evaluate_list(modifier.args, context);
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

	auto CardStateResolver::static_removed_card_subtypes(const state::CardInstance& card, Context& context) const noexcept -> utility::EnumMask<domain::CardSubtype>
	{
		const auto& vocabulary = vocabulary_.get();
		auto& interpreter = interpreter_.get();

		utility::EnumMask<domain::CardSubtype> result{};
		for (const auto& modifier: collect_static_modifiers(card) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary.find(modifier.primitive);
			if (definition == nullptr || definition->name != "remove_card_subtype")
			{
				continue;
			}

			const auto args = interpreter.evaluate_list(modifier.args, context);
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
		const auto& vocabulary = vocabulary_.get();
		auto& interpreter = interpreter_.get();

		utility::EnumMask<domain::Attribute> result{};
		for (const auto& modifier: collect_static_modifiers(card) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary.find(modifier.primitive);
			if (definition == nullptr || definition->name != "add_attribute")
			{
				continue;
			}

			const auto args = interpreter.evaluate_list(modifier.args, context);
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
		const auto& vocabulary = vocabulary_.get();
		auto& interpreter = interpreter_.get();

		Integer::value_type total = 0;
		for (const auto& modifier: collect_static_modifiers(card) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary.find(modifier.primitive);
			if (definition == nullptr || definition->name != "change_level")
			{
				continue;
			}

			const auto args = interpreter.evaluate_list(modifier.args, context);
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
		const auto& vocabulary = vocabulary_.get();
		auto& interpreter = interpreter_.get();

		Integer::value_type total = 0;
		for (const auto& modifier: collect_static_modifiers(card) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary.find(modifier.primitive);
			if (definition == nullptr || definition->name != "add_numeric_property")
			{
				continue;
			}

			const auto args = interpreter.evaluate_list(modifier.args, context);
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
		const auto& vocabulary = vocabulary_.get();
		auto& interpreter = interpreter_.get();

		utility::EnumMask<domain::Protection> result{};
		for (const auto& modifier: collect_static_modifiers(card) | std::views::transform(&std::reference_wrapper<const dsl::StaticModifier>::get))
		{
			const auto* definition = vocabulary.find(modifier.primitive);
			if (definition == nullptr || definition->name != "add_protection")
			{
				continue;
			}

			const auto args = interpreter.evaluate_list(modifier.args, context);
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
		const dsl::Vocabulary& vocabulary,
		Interpreter& interpreter,
		const ScriptRegistry& scripts
	) noexcept
		: game_{game},
		  vocabulary_{vocabulary},
		  interpreter_{interpreter},
		  scripts_{scripts} {}

	auto CardStateResolver::effective_code(const state::CardInstance& card) const noexcept -> domain::CardCode
	{
		Context context{card.controller(), card.id()};

		const auto code = static_code(card, context);
		return code.value_or(card.origin_code());
	}

	auto CardStateResolver::has_code(const state::CardInstance& card, const domain::CardCode code) const noexcept -> bool
	{
		if (effective_code(card) == code)
		{
			return true;
		}

		const auto* script = scripts_.get().find(card.origin_code());
		return script != nullptr && std::ranges::contains(script->identity.code_list, code);
	}

	auto CardStateResolver::card_types(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::CardType>
	{
		Context context{card.controller(), card.id()};

		auto result = static_card_types(card, context);
		result.add_with(card.origin_kind());

		return result;
	}

	auto CardStateResolver::contains_card_type(const state::CardInstance& card, const domain::CardType type) const noexcept -> bool
	{
		const auto result = card_types(card);
		return result.contains(type);
	}

	auto CardStateResolver::contains_card_types(const state::CardInstance& card, const utility::EnumMask<domain::CardType> types) const noexcept -> bool
	{
		const auto result = card_types(card);
		return !result.intersect(types).empty();
	}

	auto CardStateResolver::card_subtypes(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::CardSubtype>
	{
		Context context{card.controller(), card.id()};

		auto result = static_card_subtypes(card, context);
		result.unite_with(card.origin_subtypes());
		result.subtract_with(static_removed_card_subtypes(card, context));

		return result;
	}

	auto CardStateResolver::contains_card_subtype(const state::CardInstance& card, const domain::CardSubtype subtype) const noexcept -> bool
	{
		const auto result = card_subtypes(card);
		return result.contains(subtype);
	}

	auto CardStateResolver::contains_card_subtypes(const state::CardInstance& card, const utility::EnumMask<domain::CardSubtype> subtypes) const noexcept -> bool
	{
		const auto result = card_subtypes(card);
		return !result.intersect(subtypes).empty();
	}

	auto CardStateResolver::attributes(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::Attribute>
	{
		Context context{card.controller(), card.id()};

		auto result = static_attributes(card, context);
		result.add_with(card.origin_attribute());

		return result;
	}

	auto CardStateResolver::contains_attribute(const state::CardInstance& card, const domain::Attribute attribute) const noexcept -> bool
	{
		const auto result = attributes(card);
		return result.contains(attribute);
	}

	auto CardStateResolver::race(const state::CardInstance& card) const noexcept -> domain::Race
	{
		std::ignore = this;

		return card.origin_race();
	}

	auto CardStateResolver::is_race(const state::CardInstance& card, const domain::Race race) const noexcept -> bool
	{
		std::ignore = this;

		return card.origin_is_race(race);
	}

	auto CardStateResolver::level(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::LevelRank>
	{
		Context context{card.controller(), card.id()};

		const auto bonus = static_level_bonus(card, context);
		return static_cast<std::underlying_type_t<domain::LevelRank>>(card.origin_level() + bonus);
	}

	auto CardStateResolver::attack(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		Context context{card.controller(), card.id()};

		const auto bonus = static_numeric_bonus(card, domain::NumericProperty::ATTACK, context);
		return static_cast<std::underlying_type_t<domain::AttackDefense>>(card.origin_attack() + bonus);
	}

	auto CardStateResolver::defense(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		Context context{card.controller(), card.id()};

		const auto bonus = static_numeric_bonus(card, domain::NumericProperty::DEFENSE, context);
		return static_cast<std::underlying_type_t<domain::AttackDefense>>(card.origin_defense() + bonus);
	}

	auto CardStateResolver::protections(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::Protection>
	{
		Context context{card.controller(), card.id()};

		return static_protections(card, context);
	}

	auto CardStateResolver::contains_protection(const state::CardInstance& card, const domain::Protection protection) const noexcept -> bool
	{
		const auto result = protections(card);
		return result.contains(protection);
	}

	auto CardStateResolver::contains_protections(const state::CardInstance& card, const utility::EnumMask<domain::Protection> protections) const noexcept -> bool
	{
		const auto result = this->protections(card);
		return !result.intersect(protections).empty();
	}
}

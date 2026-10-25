#include <cg/execution/interpreter.hpp>

#include <algorithm>
#include <ranges>

#include <cg/utility/functional.hpp>
#include <cg/dsl/expression.hpp>
#include <cg/dsl/statement.hpp>
#include <cg/dsl/types.hpp>
#include <cg/dsl/vocabulary.hpp>
#include <cg/state/game.hpp>
#include <cg/execution/card_state_resolver.hpp>
#include <cg/execution/decision.hpp>
#include <cg/execution/chain.hpp>
#include <cg/execution/script_registry.hpp>

namespace cg::execution
{
	namespace
	{
		template<dsl::ValueClass Class>
		[[nodiscard]] auto as(const Value& value) noexcept -> const auto*
		{
			if constexpr (Class == dsl::ValueClass::VOID)
			{
				std::unreachable();
			}
			else if constexpr (Class == dsl::ValueClass::BOOL)
			{
				return std::get_if<Boolean>(&value);
			}
			else if constexpr (Class == dsl::ValueClass::INT)
			{
				return std::get_if<Integer>(&value);
			}
			else if constexpr (Class == dsl::ValueClass::CARD)
			{
				return std::get_if<Card>(&value);
			}
			else if constexpr (Class == dsl::ValueClass::GROUP)
			{
				return std::get_if<Group>(&value);
			}
			else if constexpr (Class == dsl::ValueClass::ENUM || Class == dsl::ValueClass::ENUM_SET)
			{
				return std::get_if<EnumMask>(&value);
			}
			else
			{
				std::unreachable();
			}
		}

		[[nodiscard]] auto argument(const std::vector<std::optional<Value>>& args, const std::size_t index) noexcept -> const Value*
		{
			if (index >= args.size() || !args[index].has_value())
			{
				return nullptr;
			}

			return &*args[index];
		}

		template<dsl::ValueClass Class>
		[[nodiscard]] auto as_argument(const std::vector<std::optional<Value>>& args, const std::size_t index) noexcept -> const auto*
		{
			const auto* value = argument(args, index);
			if (value == nullptr)
			{
				return static_cast<decltype(as<Class>(*value))>(nullptr);
			}

			return as<Class>(*value);
		}

		[[nodiscard]] auto integer_compare(const dsl::CompareOperator op, const Integer lhs, const Integer rhs) noexcept -> bool
		{
			switch (op)
			{
				case dsl::CompareOperator::EQUAL:
				{
					return lhs == rhs;
				}
				case dsl::CompareOperator::NOT_EQUAL:
				{
					return lhs != rhs;
				}
				case dsl::CompareOperator::LESS_THAN:
				{
					return lhs < rhs;
				}
				case dsl::CompareOperator::LESS_EQUAL:
				{
					return lhs <= rhs;
				}
				case dsl::CompareOperator::GREATER_THAN:
				{
					return lhs > rhs;
				}
				case dsl::CompareOperator::GREATER_EQUAL:
				{
					return lhs >= rhs;
				}
			}

			return false;
		}

		[[nodiscard]] auto integer_arithmetic(const dsl::ArithmeticOperator op, const Integer lhs, const Integer rhs) noexcept -> std::optional<Integer::value_type>
		{
			switch (op)
			{
				case dsl::ArithmeticOperator::ADD:
				{
					return lhs + rhs;
				}
				case dsl::ArithmeticOperator::SUB:
				{
					return lhs - rhs;
				}
				case dsl::ArithmeticOperator::MUL:
				{
					return lhs * rhs;
				}
				case dsl::ArithmeticOperator::DIV:
				{
					if (rhs == 0)
					{
						return std::nullopt;
					}
					return lhs / rhs;
				}
				case dsl::ArithmeticOperator::MIN:
				{
					return std::ranges::min(lhs, rhs);
				}
				case dsl::ArithmeticOperator::MAX:
				{
					return std::ranges::max(lhs, rhs);
				}
			}

			return std::nullopt;
		}

		template<typename T>
		[[nodiscard]] auto to_enum_mask(const EnumMask mask) noexcept -> utility::EnumMask<T>
		{
			// read_enum_set_constant_expression应该保证dsl::EnumSetConstant::mask就是我们需要的值
			return utility::EnumMask<T>{static_cast<typename utility::EnumMask<T>::value_type>(mask)};
		}
	}

	auto Interpreter::players_of(const Context& context, const EnumMask mask) const noexcept -> std::vector<domain::Player>
	{
		std::ignore = this;

		const auto selectors = to_enum_mask<domain::PlayerSelector>(mask);

		std::vector<domain::Player> players{};
		if (selectors.contains(domain::PlayerSelector::SELF))
		{
			players.push_back(context.controller);
		}
		if (selectors.contains(domain::PlayerSelector::OPPONENT))
		{
			players.push_back(domain::opponent_of(context.controller));
		}

		// TODO: EVENT_PLAYER / REASON_PLAYER / TRIGGERING_PLAYER
		return players;
	}

	auto Interpreter::collect(const std::span<const domain::Player> players, const utility::EnumMask<domain::Zone> zones) const noexcept -> Group
	{
		const auto& game = game_.get();

		Group result{};

		for (const auto player: players)
		{
			auto cards = game.collect_zones_card(player, zones);
			result.append_range(std::move(cards));
		}

		return result;
	}

	auto Interpreter::select(const Context& context, const Group& candidates, const std::size_t min, const std::size_t max) const noexcept -> std::optional<Group>
	{
		if (min > max || candidates.size() < min)
		{
			return std::nullopt;
		}

		// 引擎不自动决策:必须由外部提供选择
		auto selected = selector_.get().select_cards(context.controller, candidates, min, max);
		if (selected.size() < min || selected.size() > max)
		{
			return std::nullopt;
		}

		Group result{};
		result.reserve(selected.size());
		for (const auto card: selected)
		{
			if (!std::ranges::contains(candidates, card) || std::ranges::contains(result, card))
			{
				return std::nullopt;
			}

			result.push_back(card);
		}

		return result;
	}

	auto Interpreter::evaluate_arguments(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<std::vector<std::optional<Value>>>
	{
		std::vector<std::optional<Value>> values{};
		values.reserve(call.args.size());

		for (std::size_t i = 0; i < call.args.size(); ++i)
		{
			// 作用域参数由原语按候选逐个求值
			if (i < definition.params.size() && std::holds_alternative<dsl::ParamDef::Scoped>(definition.params[i].kind))
			{
				values.emplace_back(std::nullopt);
				continue;
			}

			auto value = evaluate(call.args[i], context);
			if (!value.has_value())
			{
				return std::nullopt;
			}

			values.emplace_back(*std::move(value));
		}

		return values;
	}

	auto Interpreter::invoke(const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto* definition = vocabulary_.get().find(call.primitive);
		if (definition == nullptr)
		{
			return std::nullopt;
		}

		if (definition->name == "find_cards")
		{
			return find_cards(*definition, call, context);
		}
		if (definition->name == "exists_matching")
		{
			return exists_matching(*definition, call, context);
		}
		if (definition->name == "pick_cost" || definition->name == "pick_target" || definition->name == "pick_resolution")
		{
			return pick(*definition, call, context);
		}
		if (definition->name == "destroy")
		{
			return destroy(*definition, call, context);
		}
		if (definition->name == "discard")
		{
			return discard(*definition, call, context);
		}
		if (definition->name == "is_code")
		{
			return is_code(*definition, call, context);
		}
		if (definition->name == "is_card_type")
		{
			return is_card_type(*definition, call, context);
		}
		if (definition->name == "is_card_subtype")
		{
			return is_card_subtype(*definition, call, context);
		}
		if (definition->name == "is_attribute")
		{
			return is_attribute(*definition, call, context);
		}
		if (definition->name == "is_race")
		{
			return is_race(*definition, call, context);
		}
		if (definition->name == "is_in_zone")
		{
			return is_in_zone(*definition, call, context);
		}
		if (definition->name == "is_face_up")
		{
			return is_face_up(*definition, call, context);
		}
		if (definition->name == "attack")
		{
			return attack(*definition, call, context);
		}
		if (definition->name == "defense")
		{
			return defense(*definition, call, context);
		}
		if (definition->name == "level")
		{
			return level(*definition, call, context);
		}
		if (definition->name == "this_card")
		{
			return this_card(*definition, call, context);
		}
		if (definition->name == "$")
		{
			return candidate_card(*definition, call, context);
		}
		if (definition->name == "chain_exists")
		{
			return chain_exists(*definition, call, context);
		}
		if (definition->name == "event_card")
		{
			return event_card(*definition, call, context);
		}
		if (definition->name == "event_amount")
		{
			return event_amount(*definition, call, context);
		}
		if (definition->name == "event_player")
		{
			return event_player(*definition, call, context);
		}
		if (definition->name == "sole_card")
		{
			return sole_card(*definition, call, context);
		}
		if (definition->name == "draw")
		{
			return draw(*definition, call, context);
		}
		if (definition->name == "add_to_hand")
		{
			return add_to_hand(*definition, call, context);
		}
		if (definition->name == "banish")
		{
			return banish(*definition, call, context);
		}
		if (definition->name == "effect_damage")
		{
			return effect_damage(*definition, call, context);
		}
		if (definition->name == "recover_life")
		{
			return recover_life(*definition, call, context);
		}
		if (definition->name == "pay_half_life")
		{
			return pay_half_life(*definition, call, context);
		}
		if (definition->name == "change_position")
		{
			return change_position(*definition, call, context);
		}
		if (definition->name == "special_summon")
		{
			return special_summon(*definition, call, context);
		}
		if (definition->name == "ritual_summon")
		{
			return ritual_summon(*definition, call, context);
		}
		if (definition->name == "negate_activation")
		{
			return negate_activation(*definition, call, context);
		}
		if (definition->name == "negate_activation_and_destroy")
		{
			return negate_activation_and_destroy(*definition, call, context);
		}
		if (definition->name == "negate_effect")
		{
			return negate_effect(*definition, call, context);
		}

		// TODO: 其余原语
		return std::nullopt;
	}

	auto Interpreter::execute_action(const dsl::ActionCall& call, Context& context) noexcept -> bool
	{
		const dsl::PrimitiveCall primitive{.primitive = call.primitive, .args = call.args};
		auto value = invoke(primitive, context);
		if (!value.has_value())
		{
			return false;
		}

		if (call.binding.has_value() && !std::holds_alternative<Void>(*value))
		{
			context.frame.bindings.insert_or_assign(*call.binding, *std::move(value));
		}

		return true;
	}

	auto Interpreter::execute_statements(const dsl::StatementList& statements, Context& context) noexcept -> bool
	{
		for (const auto& statement: statements)
		{
			const auto result = std::visit(
				utility::Overloaded
				{
						[&](const dsl::ActionCall& call) noexcept -> std::optional<bool>
						{
							if (!execute_action(call, context))
							{
								return false;
							}

							return std::nullopt;
						},
						[&](const dsl::Guard& guard) noexcept -> std::optional<bool>
						{
							const auto value = evaluate(guard.condition, context);
							if (!value.has_value())
							{
								return false;
							}

							const auto condition = as<dsl::ValueClass::BOOL>(*value);
							if (condition == nullptr)
							{
								return false;
							}

							if (!*condition)
							{
								// 中止剩余处理
								return true;
							}

							return std::nullopt;
						},
						[&](const dsl::If& i) noexcept -> std::optional<bool>
						{
							const auto value = evaluate(i.condition, context);
							if (!value.has_value())
							{
								return false;
							}

							const auto condition = as<dsl::ValueClass::BOOL>(*value);
							if (condition == nullptr)
							{
								return false;
							}

							const auto& branch = *condition ? i.then_body : i.else_body;
							return execute_statements(branch, context) ? std::nullopt : std::optional{false};
						},
						[&](const dsl::ForEach& for_each) noexcept -> std::optional<bool>
						{
							const auto value = evaluate(for_each.group, context);
							if (!value.has_value())
							{
								return false;
							}

							const auto group = as<dsl::ValueClass::GROUP>(*value);
							if (group == nullptr)
							{
								return false;
							}

							for (const auto card: *group)
							{
								context.frame.bindings.insert_or_assign(for_each.binding, Value{Card{card}});
								if (!execute_statements(for_each.body, context))
								{
									return false;
								}
							}

							return std::nullopt;
						},
				},
				statement.body
			);

			if (result.has_value())
			{
				return *result;
			}
		}

		return true;
	}

	auto Interpreter::resolve(const dsl::ActivatedEffect& effect, Context& context) noexcept -> bool
	{
		return execute_statements(effect.effect, context);
	}

	auto Interpreter::find_cards(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* players_mask = as_argument<dsl::ValueClass::ENUM_SET>(*args, 0);
		const auto* zones_mask = as_argument<dsl::ValueClass::ENUM_SET>(*args, 1);
		if (players_mask == nullptr || zones_mask == nullptr)
		{
			return std::nullopt;
		}

		const auto players = players_of(context, *players_mask);
		const auto zones = to_enum_mask<domain::Zone>(*zones_mask);
		auto candidates = collect(players, zones);

		if (call.args.size() > 2)
		{
			auto filtered = filter(call.args[2], candidates, context);
			if (!filtered.has_value())
			{
				return std::nullopt;
			}

			candidates = *std::move(filtered);
		}

		return Value{Group{std::move(candidates)}};
	}

	auto Interpreter::exists_matching(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* players_mask = as_argument<dsl::ValueClass::ENUM_SET>(*args, 0);
		const auto* zones_mask = as_argument<dsl::ValueClass::ENUM_SET>(*args, 1);
		const auto* min = as_argument<dsl::ValueClass::INT>(*args, 2);
		if (players_mask == nullptr || zones_mask == nullptr || min == nullptr)
		{
			return std::nullopt;
		}

		const auto players = players_of(context, *players_mask);
		const auto zones = to_enum_mask<domain::Zone>(*zones_mask);
		auto candidates = collect(players, zones);

		if (call.args.size() > 3)
		{
			auto filtered = filter(call.args[3], candidates, context);
			if (!filtered.has_value())
			{
				return std::nullopt;
			}

			candidates = *std::move(filtered);
		}

		return Value{Boolean{std::cmp_greater_equal(candidates.size(), static_cast<Group::size_type>(*min))}};
	}

	auto Interpreter::pick(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* players_mask = as_argument<dsl::ValueClass::ENUM_SET>(*args, 0);
		const auto* zones_mask = as_argument<dsl::ValueClass::ENUM_SET>(*args, 1);
		const auto* min = as_argument<dsl::ValueClass::INT>(*args, 2);
		const auto* max = as_argument<dsl::ValueClass::INT>(*args, 3);
		if (players_mask == nullptr || zones_mask == nullptr || min == nullptr || max == nullptr)
		{
			return std::nullopt;
		}

		const auto players = players_of(context, *players_mask);
		const auto zones = to_enum_mask<domain::Zone>(*zones_mask);
		auto candidates = collect(players, zones);

		if (call.args.size() > 4)
		{
			auto filtered = filter(call.args[4], candidates, context);
			if (!filtered.has_value())
			{
				return std::nullopt;
			}

			candidates = *std::move(filtered);
		}

		auto selected = select(context, candidates, static_cast<std::size_t>(*min), static_cast<std::size_t>(*max));
		if (!selected.has_value())
		{
			return std::nullopt;
		}

		return Value{Group{std::move(*selected)}};
	}

	auto Interpreter::destroy(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* cards = as_argument<dsl::ValueClass::GROUP>(*args, 0);
		const auto* reason = as_argument<dsl::ValueClass::INT>(*args, 1);
		if (cards == nullptr || reason == nullptr)
		{
			return std::nullopt;
		}

		for (const utility::EnumMask reasons{static_cast<domain::Reason>(static_cast<std::underlying_type_t<domain::Reason>>(*reason))};
		     const auto card: *cards)
		{
			auto* instance = game.find_instance_mutable(card);
			if (instance == nullptr)
			{
				return std::nullopt;
			}

			game.move_to_graveyard(*instance, reasons);

			pending_events_.emplace_back(domain::EventCode::DESTROYED, card);
			pending_events_.emplace_back(domain::EventCode::EFFECT_DESTROYED, card);
		}

		return Value{Void{}};
	}

	auto Interpreter::discard(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* cards = as_argument<dsl::ValueClass::GROUP>(*args, 0);
		if (cards == nullptr)
		{
			return std::nullopt;
		}

		for (const auto card: *cards)
		{
			auto* instance = game.find_instance_mutable(card);
			if (instance == nullptr)
			{
				return std::nullopt;
			}

			game.move_to_graveyard(*instance, {domain::Reason::DISCARD});
		}

		return Value{Void{}};
	}

	auto Interpreter::is_card_type(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* card = as_argument<dsl::ValueClass::CARD>(*args, 0);
		const auto* kind = as_argument<dsl::ValueClass::INT>(*args, 1);
		if (card == nullptr || kind == nullptr)
		{
			return std::nullopt;
		}

		const auto* instance = game.find_instance(*card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		const auto value = static_cast<domain::CardType>(static_cast<std::underlying_type_t<domain::CardType>>(*kind));
		const auto result = state_resolver_.contains_card_type(*instance, value);
		return Value{Boolean{result}};
	}

	auto Interpreter::is_code(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* card = as_argument<dsl::ValueClass::CARD>(*args, 0);
		const auto* code = as_argument<dsl::ValueClass::INT>(*args, 1);
		if (card == nullptr || code == nullptr)
		{
			return std::nullopt;
		}

		const auto* instance = game.find_instance(*card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		const auto expected = static_cast<domain::CardCode>(
			static_cast<std::underlying_type_t<domain::CardCode>>(*code)
		);
		return Value{Boolean{state_resolver_.has_code(*instance, expected)}};
	}

	auto Interpreter::is_card_subtype(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* card = as_argument<dsl::ValueClass::CARD>(*args, 0);
		const auto* subtype = as_argument<dsl::ValueClass::INT>(*args, 1);
		if (card == nullptr || subtype == nullptr)
		{
			return std::nullopt;
		}

		const auto* instance = game.find_instance(*card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		const auto value = static_cast<domain::CardSubtype>(static_cast<std::underlying_type_t<domain::CardSubtype>>(*subtype));
		const auto result = state_resolver_.contains_card_subtype(*instance, value);
		return Value{Boolean{result}};
	}

	auto Interpreter::is_attribute(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* card = as_argument<dsl::ValueClass::CARD>(*args, 0);
		const auto* attribute = as_argument<dsl::ValueClass::INT>(*args, 1);
		if (card == nullptr || attribute == nullptr)
		{
			return std::nullopt;
		}

		const auto* instance = game.find_instance(*card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		if (!state_resolver_.contains_card_type(*instance, domain::CardType::MONSTER))
		{
			return std::nullopt;
		}

		const auto value = static_cast<domain::Attribute>(static_cast<std::underlying_type_t<domain::Attribute>>(*attribute));
		const auto result = state_resolver_.contains_attribute(*instance, value);
		return Value{Boolean{result}};
	}

	auto Interpreter::is_race(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* card = as_argument<dsl::ValueClass::CARD>(*args, 0);
		const auto* race = as_argument<dsl::ValueClass::INT>(*args, 1);
		if (card == nullptr || race == nullptr)
		{
			return std::nullopt;
		}

		const auto* instance = game.find_instance(*card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		if (!state_resolver_.contains_card_type(*instance, domain::CardType::MONSTER))
		{
			return std::nullopt;
		}

		const auto value = static_cast<domain::Race>(static_cast<std::underlying_type_t<domain::Race>>(*race));
		const auto result = state_resolver_.is_race(*instance, value);
		return Value{Boolean{result}};
	}

	auto Interpreter::is_in_zone(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* card = as_argument<dsl::ValueClass::CARD>(*args, 0);
		const auto* zones = as_argument<dsl::ValueClass::ENUM_SET>(*args, 1);
		if (card == nullptr || zones == nullptr)
		{
			return std::nullopt;
		}

		const auto zones_mask = to_enum_mask<domain::Zone>(*zones);
		const auto zone = game.zone_of(*card);

		return Value{Boolean{zones_mask.contains(zone)}};
	}

	auto Interpreter::is_face_up(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* card = as_argument<dsl::ValueClass::CARD>(*args, 0);
		if (card == nullptr)
		{
			return std::nullopt;
		}

		const auto* instance = game.find_instance(*card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		const auto placement = instance->placement();

		if (const auto position = domain::position_of(placement);
			position.has_value())
		{
			return Value{Boolean{*position == domain::Position::FACE_UP_ATTACK || *position == domain::Position::FACE_UP_DEFENSE}};
		}

		// 墓地是表侧表示
		return Value{Boolean{std::holds_alternative<domain::GraveyardPlacement>(placement)}};
	}

	auto Interpreter::level(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* card = as_argument<dsl::ValueClass::CARD>(*args, 0);
		if (card == nullptr)
		{
			return std::nullopt;
		}

		const auto* instance = game.find_instance(*card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		if (!state_resolver_.contains_card_type(*instance, domain::CardType::MONSTER))
		{
			return std::nullopt;
		}

		return Value{Integer{state_resolver_.level(*instance)}};
	}

	auto Interpreter::attack(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* card = as_argument<dsl::ValueClass::CARD>(*args, 0);
		if (card == nullptr)
		{
			return std::nullopt;
		}

		const auto* instance = game.find_instance(*card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		if (!state_resolver_.contains_card_type(*instance, domain::CardType::MONSTER))
		{
			return std::nullopt;
		}

		return Value{Integer{state_resolver_.attack(*instance)}};
	}

	auto Interpreter::defense(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* card = as_argument<dsl::ValueClass::CARD>(*args, 0);
		if (card == nullptr)
		{
			return std::nullopt;
		}

		const auto* instance = game.find_instance(*card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		if (!state_resolver_.contains_card_type(*instance, domain::CardType::MONSTER))
		{
			return std::nullopt;
		}

		return Value{Integer{state_resolver_.defense(*instance)}};
	}

	auto Interpreter::this_card(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		if (const auto args = evaluate_arguments(definition, call, context);
			!args.has_value())
		{
			return std::nullopt;
		}

		return Value{Card{context.source}};
	}

	auto Interpreter::candidate_card(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		if (const auto args = evaluate_arguments(definition, call, context);
			!args.has_value())
		{
			return std::nullopt;
		}

		if (context.frame.element_scopes.empty())
		{
			return std::nullopt;
		}

		return Value{Card{context.frame.element_scopes.back()}};
	}

	auto Interpreter::chain_exists(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		if (const auto args = evaluate_arguments(definition, call, context);
			!args.has_value())
		{
			return std::nullopt;
		}

		if (!context.chain.has_value())
		{
			return Value{Boolean{false}};
		}

		return Value{Boolean{!context.chain->chain.get().empty()}};
	}

	auto Interpreter::event_card(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		if (const auto args = evaluate_arguments(definition, call, context);
			!args.has_value())
		{
			return std::nullopt;
		}

		if (!context.event.has_value() || !context.event->card.has_value())
		{
			return std::nullopt;
		}

		return Value{Card{*context.event->card}};
	}

	auto Interpreter::event_amount(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		if (const auto args = evaluate_arguments(definition, call, context);
			!args.has_value())
		{
			return std::nullopt;
		}

		if (!context.event.has_value() || !context.event->amount.has_value())
		{
			return std::nullopt;
		}

		return Value{Integer{*context.event->amount}};
	}

	auto Interpreter::event_player(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		if (const auto args = evaluate_arguments(definition, call, context);
			!args.has_value())
		{
			return std::nullopt;
		}

		if (!context.event.has_value() || !context.event->player.has_value())
		{
			return std::nullopt;
		}

		const auto relative = *context.event->player == context.controller ? 0 : 1;
		return Value{Integer{relative}};
	}

	auto Interpreter::sole_card(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* group = as_argument<dsl::ValueClass::GROUP>(*args, 0);
		if (group == nullptr || group->size() != 1)
		{
			return std::nullopt;
		}

		return Value{Card{*group->begin()}};
	}

	auto Interpreter::draw(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* players_mask = as_argument<dsl::ValueClass::ENUM_SET>(*args, 0);
		const auto* count = as_argument<dsl::ValueClass::INT>(*args, 1);
		if (players_mask == nullptr || count == nullptr)
		{
			return std::nullopt;
		}

		for (const auto players = players_of(context, *players_mask);
		     const auto player: players)
		{
			for (Integer::value_type i = 0; i < *count; ++i)
			{
				const auto deck = game.collect_zone_card(player, domain::Zone::DECK);
				if (deck.empty())
				{
					break;
				}

				const auto card = deck.front();
				auto* instance = game.find_instance_mutable(card);
				if (instance == nullptr)
				{
					return std::nullopt;
				}

				game.move_to_hand(*instance, {domain::Reason::DRAW}, domain::Position::FACE_DOWN_DEFENSE);
			}
		}

		return Value{Void{}};
	}

	auto Interpreter::add_to_hand(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* cards = as_argument<dsl::ValueClass::GROUP>(*args, 0);
		if (cards == nullptr)
		{
			return std::nullopt;
		}

		for (const auto card: *cards)
		{
			auto* instance = game.find_instance_mutable(card);
			if (instance == nullptr)
			{
				return std::nullopt;
			}

			game.move_to_hand(*instance, {domain::Reason::EFFECT}, domain::Position::FACE_DOWN_DEFENSE);
		}

		return Value{Void{}};
	}

	auto Interpreter::banish(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* cards = as_argument<dsl::ValueClass::GROUP>(*args, 0);
		const auto* face_down = as_argument<dsl::ValueClass::BOOL>(*args, 1);
		if (cards == nullptr || face_down == nullptr)
		{
			return std::nullopt;
		}

		const auto position = *face_down ? domain::Position::FACE_DOWN_DEFENSE : domain::Position::FACE_UP_ATTACK;

		for (const auto card: *cards)
		{
			auto* instance = game.find_instance_mutable(card);
			if (instance == nullptr)
			{
				return std::nullopt;
			}

			game.move_to_banished(*instance, {domain::Reason::EFFECT}, position);
		}

		return Value{Void{}};
	}

	auto Interpreter::effect_damage(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* players_mask = as_argument<dsl::ValueClass::ENUM_SET>(*args, 0);
		const auto* amount = as_argument<dsl::ValueClass::INT>(*args, 1);
		if (players_mask == nullptr || amount == nullptr)
		{
			return std::nullopt;
		}

		const auto players = players_of(context, *players_mask);
		const auto value = static_cast<std::underlying_type_t<domain::AttackDefense>>(*amount);

		for (const auto player: players)
		{
			game.change_life_points(player, -value);
		}

		return Value{Void{}};
	}

	auto Interpreter::recover_life(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* players_mask = as_argument<dsl::ValueClass::ENUM_SET>(*args, 0);
		const auto* amount = as_argument<dsl::ValueClass::INT>(*args, 1);
		if (players_mask == nullptr || amount == nullptr)
		{
			return std::nullopt;
		}

		const auto players = players_of(context, *players_mask);
		const auto value = static_cast<std::underlying_type_t<domain::AttackDefense>>(*amount);

		for (const auto player: players)
		{
			game.change_life_points(player, value);
		}

		return Value{Void{}};
	}

	auto Interpreter::pay_half_life(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		auto& game = game_.get();

		if (const auto args = evaluate_arguments(definition, call, context);
			!args.has_value())
		{
			return std::nullopt;
		}

		const auto life = game.life_points(context.controller);
		// 规则:一半生命值,出现小数时向上取整
		const auto amount = (life + 1) / 2;
		game.change_life_points(context.controller, -amount);

		return Value{Void{}};
	}

	auto Interpreter::change_position(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* card = as_argument<dsl::ValueClass::CARD>(*args, 0);
		const auto* position = as_argument<dsl::ValueClass::INT>(*args, 1);
		if (card == nullptr || position == nullptr)
		{
			return std::nullopt;
		}

		auto* instance = game.find_instance_mutable(*card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		const auto placement = instance->placement();
		const auto value = static_cast<domain::Position>(static_cast<std::underlying_type_t<domain::Position>>(*position));

		const auto updated = std::visit(
			utility::Overloaded
			{
					[&](const domain::MonsterPlacement p) noexcept -> std::optional<domain::Placement>
					{
						return domain::Placement{domain::MonsterPlacement{.column = p.column, .position = value}};
					},
					[&](const domain::ExtraMonsterPlacement p) noexcept -> std::optional<domain::Placement>
					{
						return domain::Placement{domain::ExtraMonsterPlacement{.column = p.column, .position = value}};
					},
					[&](const domain::SpellTrapPlacement p) noexcept -> std::optional<domain::Placement>
					{
						return domain::Placement{domain::SpellTrapPlacement{.column = p.column, .position = value}};
					},
					[&](const domain::FieldSpellPlacement) noexcept -> std::optional<domain::Placement>
					{
						return domain::Placement{domain::FieldSpellPlacement{.position = value}};
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> std::optional<domain::Placement>
					{
						return std::nullopt;
					},
			},
			placement
		);

		if (!updated.has_value())
		{
			return std::nullopt;
		}

		game.place(*instance, *updated);

		return Value{Void{}};
	}

	auto Interpreter::special_summon(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* card = as_argument<dsl::ValueClass::CARD>(*args, 0);
		const auto* method = as_argument<dsl::ValueClass::INT>(*args, 1);
		const auto* position = as_argument<dsl::ValueClass::INT>(*args, 2);

		if (card == nullptr || method == nullptr || position == nullptr)
		{
			return std::nullopt;
		}

		auto* instance = game.find_instance_mutable(*card);
		if (instance == nullptr || !state_resolver_.contains_card_type(*instance, domain::CardType::MONSTER))
		{
			return std::nullopt;
		}

		const auto value = static_cast<domain::Position>(static_cast<std::underlying_type_t<domain::Position>>(*position));
		if (value != domain::Position::FACE_UP_ATTACK && value != domain::Position::FACE_UP_DEFENSE)
		{
			return std::nullopt;
		}

		const auto method_value = static_cast<domain::SummonMethod>(static_cast<std::underlying_type_t<domain::SummonMethod>>(*method));
		const auto from = instance->placement();
		const auto source_zone = game.zone_of(*card);
		const auto requires_proper_summon = state_resolver_.contains_card_subtypes(
			*instance,
			utility::EnumMask
			{
					domain::CardSubtype::FUSION,
					domain::CardSubtype::SYNCHRO,
					domain::CardSubtype::XYZ,
					domain::CardSubtype::LINK,
					domain::CardSubtype::RITUAL,
			}
		);

		if (
			source_zone != domain::Zone::MONSTER &&
			source_zone != domain::Zone::EXTRA_MONSTER &&
			requires_proper_summon &&
			!instance->properly_summoned()
		)
		{
			return std::nullopt;
		}

		auto candidates = game.collect_monster_empty_zone(context.controller);

		const auto location = selector_.get().select_monster_location(context.controller, *card, candidates);
		if (!location.has_value() || !std::ranges::contains(candidates, *location))
		{
			return std::nullopt;
		}

		game.move_to_monster(*instance, context.controller, {domain::Reason::EFFECT}, domain::MonsterPlacement{.column = location->column, .position = value});
		game.set_summon_record(
			*instance,
			state::CardInstance::SummonRecord
			{
					.player = context.controller,
					.method = method_value,
					.from = from,
					.materials = {},
			}
		);

		pending_events_.emplace_back(domain::EventCode::SPECIAL_SUMMON_SUCCESS, *card);

		return Value{Void{}};
	}

	auto Interpreter::ritual_summon(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* position_value = as_argument<dsl::ValueClass::INT>(*args, 0);
		if (position_value == nullptr)
		{
			return std::nullopt;
		}

		const auto position = static_cast<domain::Position>(static_cast<std::underlying_type_t<domain::Position>>(*position_value));
		if (position != domain::Position::FACE_UP_ATTACK && position != domain::Position::FACE_UP_DEFENSE)
		{
			return std::nullopt;
		}

		// 仪式怪兽:自己手牌、仪式怪兽、脚本含 RitualProcedure。
		auto monster_candidates = collect({&context.controller, 1}, utility::EnumMask{domain::Zone::HAND});
		std::erase_if(
			monster_candidates.operator Group::value_type&(),
			[&](const state::CardInstanceId card) noexcept -> bool
			{
				const auto* instance = game.find_instance(card);
				if (instance == nullptr)
				{
					return true;
				}

				if (
					!state_resolver_.contains_card_type(*instance, domain::CardType::MONSTER) ||
					!state_resolver_.contains_card_subtype(*instance, domain::CardSubtype::RITUAL)
				)
				{
					return true;
				}

				const auto* script = find_script(instance->origin_code());
				if (script == nullptr)
				{
					return true;
				}

				return std::ranges::none_of(
					script->summons,
					[](const dsl::SummonProcedure& summon) noexcept -> bool
					{
						return std::holds_alternative<dsl::RitualProcedure>(summon);
					}
				);
			}
		);

		if (call.args.size() > 1)
		{
			auto filtered = filter(call.args[1], monster_candidates, context);
			if (!filtered.has_value())
			{
				return std::nullopt;
			}

			monster_candidates = *std::move(filtered);
		}

		const auto selected_monster = select(context, monster_candidates, 1, 1);
		if (!selected_monster.has_value() || selected_monster->empty())
		{
			return std::nullopt;
		}

		const auto monster_id = *selected_monster->begin();
		auto* monster = game.find_instance_mutable(monster_id);
		if (monster == nullptr)
		{
			return std::nullopt;
		}

		const auto* script = find_script(monster->origin_code());
		if (script == nullptr)
		{
			return std::nullopt;
		}

		const auto* procedure = [&]() noexcept -> const dsl::RitualProcedure*
		{
			for (const auto& summon: script->summons)
			{
				if (const auto* ritual = std::get_if<dsl::RitualProcedure>(&summon);
					ritual != nullptr)
				{
					return ritual;
				}
			}
			return nullptr;
		}();
		if (procedure == nullptr)
		{
			return std::nullopt;
		}

		Context monster_context{monster->controller(), monster->id()};
		const auto target = evaluate(procedure->level, monster_context);
		if (!target.has_value())
		{
			return std::nullopt;
		}

		const auto* target_level = std::get_if<Integer>(&*target);
		if (target_level == nullptr || *target_level < 0)
		{
			return std::nullopt;
		}

		// 素材:自己手牌/场上表侧怪兽,排除仪式怪兽自身,再按 procedure 谓词过滤
		auto material_candidates = collect(
			{&context.controller, 1},
			utility::EnumMask{domain::Zone::HAND, domain::Zone::MONSTER, domain::Zone::EXTRA_MONSTER}
		);
		std::erase_if(
			material_candidates.operator Group::value_type&(),
			[&](const state::CardInstanceId card) noexcept -> bool
			{
				if (card == monster_id)
				{
					return true;
				}

				const auto* instance = game.find_instance(card);
				if (instance == nullptr || instance->controller() != context.controller)
				{
					return true;
				}

				const auto zone = game.zone_of(card);
				if (zone != domain::Zone::MONSTER && zone != domain::Zone::EXTRA_MONSTER)
				{
					return false;
				}

				const auto field_info = domain::field_info_of(instance->placement());
				return !field_info.has_value() ||
				       (field_info->position != domain::Position::FACE_UP_ATTACK &&
				        field_info->position != domain::Position::FACE_UP_DEFENSE);
			}
		);

		auto filtered = filter(procedure->material, material_candidates, monster_context);
		if (!filtered.has_value())
		{
			return std::nullopt;
		}

		const auto selected_materials = select(context, *filtered, 1, filtered->size());
		if (!selected_materials.has_value() || selected_materials->empty())
		{
			return std::nullopt;
		}

		std::uint64_t total_level{};
		for (const auto material: *selected_materials)
		{
			const auto* instance = game.find_instance(material);
			if (instance == nullptr)
			{
				return std::nullopt;
			}

			total_level += static_cast<std::uint64_t>(state_resolver_.level(*instance));
		}

		if (std::cmp_not_equal(total_level, target_level->operator Integer::value_type()))
		{
			return std::nullopt;
		}

		auto candidates = game.collect_monster_empty_zone(context.controller);

		const auto location = selector_.get().select_monster_location(context.controller, monster->id(), candidates);
		if (!location.has_value() || !std::ranges::contains(candidates, *location))
		{
			return std::nullopt;
		}

		const auto from = monster->placement();
		for (const auto material: *selected_materials)
		{
			auto* material_instance = game.find_instance_mutable(material);
			if (material_instance == nullptr)
			{
				return std::nullopt;
			}

			game.move_to_graveyard(*material_instance, {domain::Reason::RITUAL});
			pending_events_.emplace_back(domain::EventCode::SENT_TO_GRAVEYARD, material);
		}

		game.move_to_monster(
			*monster,
			{domain::Reason::EFFECT},
			{.column = location->column, .position = position}
		);
		game.set_summon_record(
			*monster,
			state::CardInstance::SummonRecord
			{
					.player = context.controller,
					.method = domain::SummonMethod::RITUAL,
					.from = from,
					.materials = {selected_materials->begin(), selected_materials->end()},
			}
		);
		pending_events_.emplace_back(domain::EventCode::SPECIAL_SUMMON_SUCCESS, monster_id);

		return Value{Void{}};
	}

	auto Interpreter::negate_activation(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		// 无效对象 = 本效果直接响应的那一环
		if (!context.chain.has_value() || context.chain->link_index == 0)
		{
			return std::nullopt;
		}

		auto& chain = context.chain->chain.get();
		auto& link = chain[context.chain->link_index - 1];

		link.negate_activation();

		return Value{Void{}};
	}

	auto Interpreter::negate_activation_and_destroy(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		auto& game = game_.get();

		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		if (!context.chain.has_value() || context.chain->link_index == 0)
		{
			return std::nullopt;
		}

		auto& chain = context.chain->chain.get();
		auto& link = chain[context.chain->link_index - 1];
		const auto source = link.source();

		link.negate_activation();

		auto* instance = game.find_instance_mutable(source);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		game.move_to_graveyard(*instance, {domain::Reason::EFFECT, domain::Reason::DESTROY});

		pending_events_.emplace_back(domain::EventCode::DESTROYED, source);
		pending_events_.emplace_back(domain::EventCode::EFFECT_DESTROYED, source);

		return Value{Void{}};
	}

	auto Interpreter::negate_effect(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		if (!context.chain.has_value() || context.chain->link_index == 0)
		{
			return std::nullopt;
		}

		auto& chain = context.chain->chain.get();
		auto& link = chain[context.chain->link_index - 1];

		link.negate_effect();

		return Value{Void{}};
	}

	Interpreter::Interpreter(
		state::Game& game,
		const dsl::TypeRegistry& types,
		const dsl::Vocabulary& vocabulary,
		Selector& selector
	) noexcept
		: game_{game},
		  types_
		  {types},
		  vocabulary_{vocabulary},
		  selector_{selector},
		  // scripts_{},
		  state_resolver_{game, vocabulary, *this, scripts_} {}

	auto Interpreter::load_scripts(const std::filesystem::path& directory) noexcept -> bool
	{
		return scripts_.load_directory(directory, types_, vocabulary_);
	}

	auto Interpreter::load_script(const std::filesystem::path& file) noexcept -> bool
	{
		return scripts_.load_file(file, types_, vocabulary_);
	}

	auto Interpreter::add_script(const domain::CardCode code, dsl::CardScript script) noexcept -> bool
	{
		return scripts_.add(code, std::move(script));
	}

	auto Interpreter::find_script(const domain::CardCode code) const noexcept -> const dsl::CardScript*
	{
		return scripts_.find(code);
	}

	auto Interpreter::state_resolver() const noexcept -> const CardStateResolver&
	{
		return state_resolver_;
	}

	auto Interpreter::take_events() noexcept -> PendingEventList
	{
		return std::exchange(pending_events_, {});
	}

	auto Interpreter::evaluate_list(const dsl::ExpressionList& expressions, Context& context) noexcept -> std::optional<std::vector<Value>>
	{
		std::vector<Value> values{};
		values.reserve(expressions.size());

		for (const auto& expression: expressions)
		{
			auto value = evaluate(expression, context);
			if (!value.has_value())
			{
				return std::nullopt;
			}

			values.emplace_back(*std::move(value));
		}

		return values;
	}

	auto Interpreter::evaluate(const dsl::Expression& expression, Context& context) noexcept -> std::optional<Value>
	{
		return std::visit(
			utility::Overloaded
			{
					[](const dsl::Literal& literal) noexcept -> std::optional<Value>
					{
						return std::visit(
							utility::Overloaded
							{
									[](const dsl::Literal::boolean_type boolean) noexcept -> Value
									{
										return {Boolean{boolean}};
									},
									[](const dsl::Literal::number_type number) noexcept -> Value
									{
										const auto value = static_cast<Integer::value_type>(number);

										return {Integer{value}};
									},
									[](const dsl::Literal::Symbol& symbol) noexcept -> Value
									{
										const auto value = static_cast<Integer::value_type>(symbol.value);

										return {Integer{value}};
									},
							},
							literal.value
						);
					},
					[](const dsl::EnumSetConstant& enums) noexcept -> std::optional<Value>
					{
						const auto value = static_cast<EnumMask::value_type>(enums.mask);

						return Value{EnumMask{value}};
					},
					[&](const dsl::BindingRef& binding) noexcept -> std::optional<Value>
					{
						const auto it = context.frame.bindings.find(binding.id);
						if (it == context.frame.bindings.end())
						{
							return std::nullopt;
						}

						return it->second;
					},
					[&](const dsl::PrimitiveCall& call) noexcept -> std::optional<Value>
					{
						return invoke(call, context);
					},
					[&](const dsl::BooleanOperation& boolean) noexcept -> std::optional<Value>
					{
						const auto operands = evaluate_list(boolean.operands, context);
						if (!operands.has_value())
						{
							return std::nullopt;
						}

						if (boolean.op == dsl::BooleanOperator::NOT)
						{
							if (operands->size() != 1)
							{
								return std::nullopt;
							}

							const auto value = as<dsl::ValueClass::BOOL>(operands->operator[](0));
							if (value == nullptr)
							{
								return std::nullopt;
							}

							return Value{Boolean{!*value}};
						}

						auto result = boolean.op == dsl::BooleanOperator::AND;
						for (const auto& operand: *operands)
						{
							const auto value = as<dsl::ValueClass::BOOL>(operand);
							if (value == nullptr)
							{
								return std::nullopt;
							}

							if (boolean.op == dsl::BooleanOperator::AND)
							{
								result = result && *value;
							}
							else
							{
								result = result || *value;
							}
						}

						return Value{Boolean{result}};
					},
					[&](const dsl::CompareOperation& compare) noexcept -> std::optional<Value>
					{
						const auto operands = evaluate_list(compare.operands, context);
						if (!operands.has_value() || operands->size() != 2)
						{
							return std::nullopt;
						}

						// TODO: 卡片/组的相等比较
						const auto lhs = as<dsl::ValueClass::INT>(operands->operator[](0));
						const auto rhs = as<dsl::ValueClass::INT>(operands->operator[](1));
						if (lhs == nullptr || rhs == nullptr)
						{
							return std::nullopt;
						}

						return Value{Boolean{integer_compare(compare.op, *lhs, *rhs)}};
					},
					[&](const dsl::ArithmeticOperation& arithmetic) noexcept -> std::optional<Value>
					{
						const auto operands = evaluate_list(arithmetic.operands, context);
						if (!operands.has_value() || operands->size() != 2)
						{
							return std::nullopt;
						}

						const auto lhs = as<dsl::ValueClass::INT>(operands->operator[](0));
						const auto rhs = as<dsl::ValueClass::INT>(operands->operator[](1));
						if (lhs == nullptr || rhs == nullptr)
						{
							return std::nullopt;
						}

						const auto result = integer_arithmetic(arithmetic.op, *lhs, *rhs);
						if (!result.has_value())
						{
							return std::nullopt;
						}

						return Value{Integer{*result}};
					},
			},
			expression
		);
	}

	auto Interpreter::evaluate_scoped(const dsl::Expression& expression, const state::CardInstanceId element, Context& context) noexcept -> std::optional<Value>
	{
		context.frame.element_scopes.push_back(element);

		auto value = evaluate(expression, context);

		context.frame.element_scopes.pop_back();

		return value;
	}

	auto Interpreter::execute(const dsl::StatementList& statements, Context& context) noexcept -> bool
	{
		return execute_statements(statements, context);
	}

	auto Interpreter::filter(const dsl::Expression& predicate, const Group& candidates, Context& context) noexcept -> std::optional<Group>
	{
		Group result{};
		result.reserve(candidates.size());

		for (const auto card: candidates)
		{
			auto value = evaluate_scoped(predicate, card, context);
			if (!value.has_value())
			{
				return std::nullopt;
			}

			const auto condition = as<dsl::ValueClass::BOOL>(*value);
			if (condition == nullptr)
			{
				return std::nullopt;
			}

			if (*condition)
			{
				result.push_back(card);
			}
		}

		return result;
	}

	auto Interpreter::matches(const dsl::Trigger& trigger, Context& context) noexcept -> bool
	{
		if (!context.event.has_value())
		{
			return false;
		}

		if (!std::ranges::contains(trigger.spec.sources, context.event->source))
		{
			return false;
		}

		if (trigger.spec.condition.has_value())
		{
			const auto value = evaluate(*trigger.spec.condition, context);
			if (!value.has_value())
			{
				return false;
			}

			if (const auto condition = as<dsl::ValueClass::BOOL>(*value);
				condition == nullptr || !static_cast<bool>(*condition))
			{
				return false;
			}
		}

		return true;
	}

	auto Interpreter::collect_triggered(const std::span<const EventWindow> windows, const domain::Player turn_player) noexcept -> TriggeredEffectList
	{
		const auto& game = game_.get();

		TriggeredEffectList result{};
		for (const auto& instance: game.instances())
		{
			const auto* script = scripts_.find(instance.origin_code());
			if (script == nullptr)
			{
				continue;
			}

			const auto zone = game.zone_of(instance.id());

			// 卡组/额外卡组中的卡不会发动效果
			if (zone == domain::Zone::DECK || zone == domain::Zone::EXTRA_DECK || zone == domain::Zone::NOT_EXIST)
			{
				continue;
			}

			for (const auto& effect: script->effects)
			{
				const auto* activated = std::get_if<dsl::ActivatedEffect>(&effect.body);
				if (activated == nullptr)
				{
					continue;
				}

				const auto* trigger = std::get_if<dsl::Trigger>(&activated->activation);
				if (trigger == nullptr)
				{
					continue;
				}

				if (!activated->activation_zones.empty() && !activated->activation_zones.contains(zone))
				{
					continue;
				}

				// 命中任一事件时点即可;同一效果只取首个命中的窗口
				for (const auto& window: windows)
				{
					if (Context context{instance.controller(), instance.id(), window};
						!matches(*trigger, context))
					{
						continue;
					}

					const TriggeredEffect triggered_effect
					{
							.controller = instance.controller(),
							.instance = std::ref(instance),
							.effect = std::ref(effect),
							.activated = std::ref(*activated),
							.trigger = std::ref(*trigger),
							.window = window,
					};

					result.push_back(triggered_effect);
					break;
				}
			}
		}

		// SEGOC:必发优先;同组内回合玩家优先
		std::ranges::stable_sort(
			result,
			[&](const TriggeredEffect& lhs, const TriggeredEffect& rhs) noexcept -> bool
			{
				const auto lhs_priority = std::tuple{!lhs.trigger.get().optional ? 0 : 1, lhs.controller == turn_player ? 0 : 1};
				const auto rhs_priority = std::tuple{!rhs.trigger.get().optional ? 0 : 1, rhs.controller == turn_player ? 0 : 1};

				return lhs_priority < rhs_priority;
			}
		);

		// 同一效果被多个事件命中只取首个;不同效果可以进入同一连锁
		return result;
	}

	auto Interpreter::activate(const dsl::ActivatedEffect& effect, Context& context) noexcept -> bool
	{
		if (effect.condition.has_value())
		{
			const auto value = evaluate(*effect.condition, context);
			if (!value.has_value())
			{
				return false;
			}

			if (const auto condition = as<dsl::ValueClass::BOOL>(*value);
				condition == nullptr || !*condition)
			{
				return false;
			}
		}

		if (!execute_statements(effect.cost, context))
		{
			return false;
		}

		return execute_statements(effect.target, context);
	}

	auto Interpreter::resolve(Chain& chain) noexcept -> bool
	{
		auto resolved = true;

		for (auto index = chain.size(); index > 0; --index)
		{
			auto& link = chain[index - 1];

			// 结算期上下文:供无效类原语定位"上一环"
			link.set_chain_position(chain, index - 1);

			// 发动无效 → 不结算;效果无效 → 结算但什么也不做
			if (link.activation_negated() || link.effect_negated())
			{
				continue;
			}

			if (const auto* activated = link.activated_effect();
				activated != nullptr && !resolve(*activated, link.context()))
			{
				// 单个效果处理失败不阻断更早的连锁
				resolved = false;
			}
		}

		return resolved;
	}

	auto Interpreter::make_link(const TriggeredEffect& entry) noexcept -> std::optional<ChainLink>
	{
		Context link_context{entry.controller, entry.instance.get().id(), entry.window};

		if (!activate(entry.activated.get(), link_context))
		{
			return std::nullopt;
		}

		return ChainLink
		{
				entry.controller,
				// 诱发效果固定为速度1
				domain::EffectSpeed::NORMAL,
				entry.instance.get().id(),
				&entry.activated.get(),
				&entry.effect.get(),
				std::move(link_context),
		};
	}

	auto Interpreter::collect_triggered(const EventWindow& window, const domain::Player turn_player) noexcept -> TriggeredEffectList
	{
		return collect_triggered({&window, 1}, turn_player);
	}

	auto Interpreter::build_chain(const TriggeredEffectList& triggered) noexcept -> Chain
	{
		Chain chain{};

		for (const auto& entry: triggered)
		{
			auto link = make_link(entry);
			if (!link.has_value())
			{
				continue;
			}

			chain.push_back(std::move(*link));
		}

		return chain;
	}

	auto Interpreter::execute(const dsl::ActivatedEffect& effect, Context& context) noexcept -> bool
	{
		if (!activate(effect, context))
		{
			return false;
		}

		return resolve(effect, context);
	}
}

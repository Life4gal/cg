#include <cg/execution/interpreter.hpp>

#include <algorithm>
#include <ranges>
#include <utility>

#include <cg/utility/variant.hpp>
#include <cg/utility/functional.hpp>
#include <cg/dsl/expression.hpp>
#include <cg/dsl/statement.hpp>
#include <cg/dsl/types.hpp>
#include <cg/dsl/vocabulary.hpp>
#include <cg/state/game.hpp>

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

		[[nodiscard]] auto players_of(const EnumMask mask, const Context& context) noexcept -> std::vector<domain::Player>
		{
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

		[[nodiscard]] auto collect(const Context& context, const std::vector<domain::Player>& players, const utility::EnumMask<domain::Zone> zones) noexcept -> Group
		{
			Group result{};

			for (const auto player: players)
			{
				auto cards = context.game.get().collect_zones_card(player, zones);
				result.append_range(std::move(cards));
			}

			return result;
		}

		[[nodiscard]] auto select(const Context& context, const Group& candidates, const std::size_t min, const std::size_t max) -> Group
		{
			if (context.selector)
			{
				return context.selector(candidates, min, max);
			}

			// 缺省:取前 min 张
			const auto count = std::ranges::min(min, candidates.size());
			return {candidates.begin(), candidates.begin() + static_cast<std::ptrdiff_t>(count)};
		}
	}

	Interpreter::Interpreter(const dsl::Vocabulary& vocabulary) noexcept
		: vocabulary_{vocabulary} {}

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

	auto Interpreter::evaluate_scoped(const dsl::Expression& expression, const state::CardInstanceId element, Context& context) noexcept -> std::optional<Value>
	{
		context.element_scopes.push_back(element);

		auto value = evaluate(expression, context);

		context.element_scopes.pop_back();

		return value;
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
						const auto it = context.bindings.find(binding.id);
						if (it == context.bindings.end())
						{
							return std::nullopt;
						}

						return it->second;
					},
					[&]([[maybe_unused]] const dsl::CandidateRef& candidate) noexcept -> std::optional<Value>
					{
						if (context.element_scopes.empty())
						{
							return std::nullopt;
						}

						return Value{Card{context.element_scopes.back()}};
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
			context.bindings.insert_or_assign(*call.binding, *std::move(value));
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
								context.bindings.insert_or_assign(for_each.binding, Value{Card{card}});
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

	auto Interpreter::execute(const dsl::ActivatedEffect& effect, Context& context) noexcept -> bool
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
		if (!execute_statements(effect.target, context))
		{
			return false;
		}

		return execute_statements(effect.effect, context);
	}

	auto Interpreter::filter(Context& context, const dsl::Expression& predicate, const Group& candidates) noexcept -> std::optional<Group>
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

	auto Interpreter::find_cards(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* players = as<dsl::ValueClass::ENUM_SET>(*argument(*args, 0));
		const auto* zones = as<dsl::ValueClass::ENUM_SET>(*argument(*args, 1));
		if (players == nullptr || zones == nullptr)
		{
			return std::nullopt;
		}

		auto candidates = collect(context, players_of(*players, context), to_enum_mask<domain::Zone>(*zones));

		if (call.args.size() > 2)
		{
			auto filtered = filter(context, call.args[2], candidates);
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

		const auto* players = as<dsl::ValueClass::ENUM_SET>(*argument(*args, 0));
		const auto* zones = as<dsl::ValueClass::ENUM_SET>(*argument(*args, 1));
		const auto* min = as<dsl::ValueClass::INT>(*argument(*args, 2));
		if (players == nullptr || zones == nullptr || min == nullptr)
		{
			return std::nullopt;
		}

		auto candidates = collect(context, players_of(*players, context), to_enum_mask<domain::Zone>(*zones));

		if (call.args.size() > 3)
		{
			auto filtered = filter(context, call.args[3], candidates);
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

		const auto* players = as<dsl::ValueClass::ENUM_SET>(*argument(*args, 0));
		const auto* zones = as<dsl::ValueClass::ENUM_SET>(*argument(*args, 1));
		const auto* min = as<dsl::ValueClass::INT>(*argument(*args, 2));
		const auto* max = as<dsl::ValueClass::INT>(*argument(*args, 3));
		if (players == nullptr || zones == nullptr || min == nullptr || max == nullptr)
		{
			return std::nullopt;
		}

		auto candidates = collect(context, players_of(*players, context), to_enum_mask<domain::Zone>(*zones));

		if (call.args.size() > 4)
		{
			auto filtered = filter(context, call.args[4], candidates);
			if (!filtered.has_value())
			{
				return std::nullopt;
			}

			candidates = *std::move(filtered);
		}

		auto selected = select(context, candidates, static_cast<std::size_t>(*min), static_cast<std::size_t>(*max));

		return Value{Group{std::move(selected)}};
	}

	auto Interpreter::destroy(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* cards = as<dsl::ValueClass::GROUP>(*argument(*args, 0));
		const auto* reason = as<dsl::ValueClass::INT>(*argument(*args, 1));
		if (cards == nullptr || reason == nullptr)
		{
			return std::nullopt;
		}

		utility::EnumMask<domain::Reason> reasons{};
		reasons.add(static_cast<domain::Reason>(static_cast<std::underlying_type_t<domain::Reason>>(*reason)));

		for (const auto card: *cards)
		{
			if (!context.game.get().move_to_graveyard(card, reasons))
			{
				return std::nullopt;
			}
		}

		return Value{Void{}};
	}

	auto Interpreter::discard(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>
	{
		const auto args = evaluate_arguments(definition, call, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		const auto* cards = as<dsl::ValueClass::GROUP>(*argument(*args, 0));
		if (cards == nullptr)
		{
			return std::nullopt;
		}

		for (const auto card: *cards)
		{
			if (!context.game.get().move_to_graveyard(card, {domain::Reason::DISCARD}))
			{
				return std::nullopt;
			}
		}

		return Value{Void{}};
	}
}

#include <cg/execution/interpreter.hpp>

#include <algorithm>
#include <ranges>

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

		[[nodiscard]] auto integer_compare(const dsl::CompareOperator op, const Integer::value_type lhs, const Integer::value_type rhs) noexcept -> bool
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

		[[nodiscard]] auto integer_arithmetic(const dsl::ArithmeticOperator op, const Integer::value_type lhs, const Integer::value_type rhs) noexcept -> std::optional<Integer::value_type>
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
					return std::min(lhs, rhs);
				}
				case dsl::ArithmeticOperator::MAX:
				{
					return std::max(lhs, rhs);
				}
			}

			return std::nullopt;
		}

		template<typename T>
		[[nodiscard]] auto to_enum_mask(const EnumMask mask) noexcept -> utility::EnumMask<T>
		{
			// read_enum_set_constant_expression应该保证dsl::EnumSetConstant::mask就是我们需要的值
			return utility::EnumMask<T>{static_cast<typename utility::EnumMask<T>::value_type>(mask.value)};
		}
	}

	auto Interpreter::evaluate_arguments(const dsl::ExpressionList& args, Context& context) noexcept -> std::optional<ValueList>
	{
		ValueList values{};
		values.reserve(args.size());

		for (const auto& arg: args)
		{
			auto value = evaluate(arg, context);
			if (!value.has_value())
			{
				return std::nullopt;
			}

			values.push_back(*std::move(value));
		}

		return values;
	}

	auto Interpreter::evaluate(const dsl::Expression& expression, Context& context) noexcept -> std::optional<Value>
	{
		return std::visit(
			utility::Overloaded
			{
					[&](const dsl::Literal& literal) noexcept -> std::optional<Value>
					{
						return std::visit(
							utility::Overloaded
							{
									[](const dsl::Literal::boolean_type boolean) noexcept -> Value
									{
										return {Boolean{.value = boolean}};
									},
									[](const dsl::Literal::number_type number) noexcept -> Value
									{
										const auto value = static_cast<Integer::value_type>(number);

										return {Integer{.value = value}};
									},

									[](const dsl::Literal::Symbol& symbol) noexcept -> Value
									{
										const auto value = static_cast<Integer::value_type>(symbol.value);

										return {Integer{.value = value}};
									},
							},
							literal.value
						);
					},
					[](const dsl::EnumSetConstant& enums) noexcept -> std::optional<Value>
					{
						const auto value = static_cast<EnumMask::value_type>(enums.mask);

						return Value{EnumMask{.value = value}};
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
					[](const dsl::CandidateRef& candidate) noexcept -> std::optional<Value>
					{
						// TODO: CandidateRef(需要元素作用域)
						std::ignore = candidate;
						return std::nullopt;
					},
					[&](const dsl::PrimitiveCall& call) noexcept -> std::optional<Value>
					{
						return invoke(call, context);
					},
					[&](const dsl::BooleanOperation& boolean) noexcept -> std::optional<Value>
					{
						const auto operands = evaluate_arguments(boolean.operands, context);
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

							return Value{Boolean{.value = !value->value}};
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
								result = result && value->value;
							}
							else
							{
								result = result || value->value;
							}
						}

						return Value{Boolean{.value = result}};
					},
					[&](const dsl::CompareOperation& compare) noexcept -> std::optional<Value>
					{
						const auto operands = evaluate_arguments(compare.operands, context);
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

						return Value{Boolean{.value = integer_compare(compare.op, lhs->value, rhs->value)}};
					},
					[&](const dsl::ArithmeticOperation& arithmetic) noexcept -> std::optional<Value>
					{
						const auto operands = evaluate_arguments(arithmetic.operands, context);
						if (!operands.has_value())
						{
							return std::nullopt;
						}

						const auto lhs = as<dsl::ValueClass::INT>(operands->operator[](0));
						const auto rhs = as<dsl::ValueClass::INT>(operands->operator[](1));
						if (lhs == nullptr || rhs == nullptr)
						{
							return std::nullopt;
						}

						const auto result = integer_arithmetic(arithmetic.op, lhs->value, rhs->value);
						if (!result.has_value())
						{
							return std::nullopt;
						}

						return Value{Integer{.value = *result}};
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

		const auto args = evaluate_arguments(call.args, context);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		if (definition->name == "find_cards")
		{
			return find_cards(*args, context);
		}

		// TODO: 其余查询原语
		return std::nullopt;
	}

	auto Interpreter::execute_action(const dsl::ActionCall& call, Context& context) noexcept -> bool
	{
		const auto* definition = vocabulary_.get().find(call.primitive);
		if (definition == nullptr)
		{
			return false;
		}

		const auto args = evaluate_arguments(call.args, context);
		if (!args.has_value())
		{
			return false;
		}

		if (definition->name == "destroy")
		{
			return destroy(*args, context);
		}

		// TODO: 其余动作原语
		return false;
	}

	auto Interpreter::find_cards(const ValueList& args, Context& context) noexcept -> std::optional<Value>
	{
		if (args.size() < 2)
		{
			return std::nullopt;
		}

		const auto players = as<dsl::ValueClass::ENUM_SET>(args[0]);
		const auto zones = as<dsl::ValueClass::ENUM_SET>(args[1]);
		if (players == nullptr || zones == nullptr)
		{
			return std::nullopt;
		}

		const auto players_mask = to_enum_mask<domain::PlayerSelector>(*players);
		const auto zones_mask = to_enum_mask<domain::Zone>(*zones);

		Group result{};
		const auto append = [&](const domain::Player player) noexcept -> void
		{
			auto cards = context.game.get().collect_zones_card(player, zones_mask);
			result.value.append_range(std::move(cards));
		};

		if (players_mask.contains(domain::PlayerSelector::SELF))
		{
			append(context.controller);
		}
		if (players_mask.contains(domain::PlayerSelector::OPPONENT))
		{
			append(domain::opponent_of(context.controller));
		}

		// TODO: EVENT_PLAYER / REASON_PLAYER / TRIGGERING_PLAYER;filter 参数
		std::ignore = context;

		return Value{std::move(result)};
	}

	auto Interpreter::destroy(const ValueList& args, const Context& context) noexcept -> bool
	{
		if (args.size() < 2)
		{
			return false;
		}

		const auto* cards = as<dsl::ValueClass::GROUP>(args[0]);
		const auto reason = as<dsl::ValueClass::INT>(args[1]);
		if (cards == nullptr || reason == nullptr)
		{
			return false;
		}

		utility::EnumMask<domain::Reason> reasons{};
		reasons.add(static_cast<domain::Reason>(reason->value));

		for (const auto card: cards->value)
		{
			if (!context.game.get().move_to_graveyard(card, reasons))
			{
				return false;
			}
		}

		return true;
	}

	Interpreter::Interpreter(const dsl::Vocabulary& vocabulary) noexcept
		: vocabulary_{vocabulary} {}

	auto Interpreter::execute(const dsl::ActivatedEffect& effect, Context& context) noexcept -> bool
	{
		for (const auto& [conjunction, body]: effect.effect)
		{
			const auto r = std::visit(
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

							if (!condition->value)
							{
								// 中止剩余处理
								return true;
							}

							return std::nullopt;
						},
						[&](const dsl::If& i) noexcept -> std::optional<bool>
						{
							// todo
							std::ignore = i;
							return std::nullopt;
						},
						[&](const dsl::ForEach& for_each) noexcept -> std::optional<bool>
						{
							// todo
							std::ignore = for_each;
							return std::nullopt;
						},
				},
				body
			);

			if (r.has_value())
			{
				return *r;
			}
		}

		return true;
	}
}

#include <cg/dsl/json/reader.hpp>

#include <algorithm>
#include <ranges>

#include <cg/dsl/archetype.hpp>
#include <cg/dsl/types.hpp>
#include <cg/dsl/vocabulary.hpp>

#include <libassert/assert.hpp>

namespace cg::dsl::json
{
	namespace
	{
		// 检查json中的key都在keys列表中
		[[nodiscard]] auto only_specified_keys(const nlohmann::json& json, const std::initializer_list<std::string_view> keys) noexcept -> bool
		{
			for (const auto& [key, _]: json.items())
			{
				if (!std::ranges::contains(keys, key))
				{
					return false;
				}
			}

			return true;
		}
	}

	auto Reader::fail(std::string path, std::string message) noexcept -> void
	{
		diagnostics_.emplace_back(std::move(path), std::move(message));
	}

	auto Reader::fail(const std::string_view path, std::string message) noexcept -> void
	{
		fail(std::string{path}, std::move(message));
	}

	auto Reader::contains_binding(const std::string_view name) const noexcept -> bool
	{
		for (const auto& scope: scopes_ | std::views::reverse)
		{
			if (scope.contains(name))
			{
				return true;
			}
		}

		return false;
	}

	auto Reader::find_binding(const std::string_view name) const noexcept -> std::optional<BindingId>
	{
		for (const auto& scope: scopes_ | std::views::reverse)
		{
			const auto it = scope.find(name);
			if (it == scope.end())
			{
				continue;
			}

			return it->second;
		}

		return std::nullopt;
	}

	auto Reader::generate_binding_id() noexcept -> BindingId
	{
		const auto current = binding_id_;
		binding_id_ += 1;

		return BindingId{current};
	}

	auto Reader::read_statement_list(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<StatementList>
	{
		if (!json.is_array())
		{
			fail(path, "statement list must be an array");
			return std::nullopt;
		}

		StatementList list{};
		list.reserve(json.size());

		for (const auto [index, item]: json | std::views::enumerate)
		{
			const auto item_path = std::format("{}[{}]", path, index);

			auto statement = read_statement(item, item_path);
			if (!statement.has_value())
			{
				return std::nullopt;
			}

			list.push_back(*std::move(statement));
		}

		return list;
	}

	auto Reader::read_statement(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<Statement>
	{
		if (!json.is_object())
		{
			fail(path, "statement must be an object");
			return std::nullopt;
		}

		auto conjunction = Conjunction::NONE;
		if (const auto it = json.find("conjunction");
			it != json.end())
		{
			const auto c = read_statement_conjunction(it, path);
			if (!c.has_value())
			{
				return std::nullopt;
			}

			conjunction = *c;
		}

		if (json.contains("call"))
		{
			auto body = read_call_statement_body(json, path);
			if (!body.has_value())
			{
				return std::nullopt;
			}

			return Statement{.conjunction = conjunction, .body = *std::move(body)};
		}

		if (json.contains("guard"))
		{
			auto body = read_guard_statement_body(json, path);
			if (!body.has_value())
			{
				return std::nullopt;
			}

			return Statement{.conjunction = conjunction, .body = *std::move(body)};
		}

		if (json.contains("if"))
		{
			auto body = read_if_statement_body(json, path);
			if (!body.has_value())
			{
				return std::nullopt;
			}

			return Statement{.conjunction = conjunction, .body = *std::move(body)};
		}

		if (json.contains("foreach"))
		{
			auto body = read_foreach_statement_body(json, path);
			if (!body.has_value())
			{
				return std::nullopt;
			}

			return Statement{.conjunction = conjunction, .body = *std::move(body)};
		}

		fail(path, "unknown statement form");
		return std::nullopt;
	}

	auto Reader::read_statement_conjunction(
		const nlohmann::json::const_iterator& conjunction,
		const std::string_view path
	) noexcept -> std::optional<Conjunction>
	{
		auto item_path = std::format("{}.conjunction", path);

		if (!conjunction.value().is_string())
		{
			fail(std::move(item_path), "must be a string");
			return std::nullopt;
		}

		const auto& text = conjunction.value().get_ref<const nlohmann::json::string_t&>();
		const auto c = utility::from_string<Conjunction>(text);
		if (!c.has_value())
		{
			fail(std::move(item_path), std::format("unknown conjunction `{}`", text));
			return std::nullopt;
		}

		return c;
	}

	auto Reader::read_call_statement_body(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<ActionCall>
	{
		LIBASSERT_DEBUG_ASSERT(json.contains("call"));

		if (!only_specified_keys(json, {"call", "args", "bind", "conjunction"}))
		{
			fail(path, "call statement allows on `call` / `args` / `bind` / `conjunction`");
			return std::nullopt;
		}

		auto item_path = std::format("{}.call", path);

		const auto& call = json.at("call");
		if (!call.is_string())
		{
			fail(std::move(item_path), "must be a string");
			return std::nullopt;
		}

		const auto& text = call.get_ref<const nlohmann::json::string_t&>();
		const auto* def = vocabulary_.get().find(text);
		if (def == nullptr)
		{
			fail(std::move(item_path), std::format("unknown primitive `{}`", text));
			return std::nullopt;
		}

		if (def->kind != PrimitiveKind::SELECT && def->kind != PrimitiveKind::ACTION && def->kind != PrimitiveKind::CONTROL && def->kind != PrimitiveKind::STATIC)
		{
			fail(std::move(item_path), std::format("primitive `{}` is not usable as a statement", text));
			return std::nullopt;
		}

		auto args_path = std::format("{}.args", path);

		const auto& args_it = json.find("args");
		if (args_it == json.end())
		{
			fail(std::move(args_path), "missing");
			return std::nullopt;
		}
		auto args = read_argument_list(*def, args_it, args_path);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		std::optional<BindingId> binding;
		if (const auto it = json.find("bind");
			it != json.end())
		{
			auto binding_path = std::format("{}.bind", path);

			if (!it.value().is_string())
			{
				fail(std::move(binding_path), "must be a string");
				return std::nullopt;
			}

			const auto& binding_name = it.value().get_ref<const nlohmann::json::string_t&>();
			if (contains_binding(binding_name))
			{
				fail(std::move(binding_path), std::format("duplicate bind `{}` (shadowing forbidden)", binding_name));
				return std::nullopt;
			}

			const auto id = generate_binding_id();
			scopes_.back().emplace(binding_name, id);
			binding = id;
		}

		return ActionCall{.primitive = def->id, .args = *std::move(args), .binding = binding};
	}

	auto Reader::read_guard_statement_body(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<Guard>
	{
		LIBASSERT_DEBUG_ASSERT(json.contains("guard"));
		LIBASSERT_DEBUG_ASSERT(types_.get().contains("bool"));

		if (!only_specified_keys(json, {"guard", "conjunction"}))
		{
			fail(path, "guard allows only `guard` / `conjunction`");
			return std::nullopt;
		}

		const auto guard_path = std::format("{}.guard", path);

		const auto guard_it = json.find("guard");
		auto condition = read_expression(guard_it.value(), types_.get().find("bool")->id, guard_path);
		if (!condition.has_value())
		{
			return std::nullopt;
		}

		return Guard{.condition = *std::move(condition)};
	}

	auto Reader::read_if_statement_body(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<If>
	{
		LIBASSERT_DEBUG_ASSERT(json.contains("if"));
		LIBASSERT_DEBUG_ASSERT(types_.get().contains("bool"));

		if (!only_specified_keys(json, {"if", "then", "else", "conjunction"}))
		{
			fail(path, "if allows only `if` / `then` / `else` / `conjunction`");
			return std::nullopt;
		}

		const auto if_path = std::format("{}.if", path);

		const auto if_it = json.find("if");
		auto condition = read_expression(if_it.value(), types_.get().find("bool")->id, if_path);
		if (!condition.has_value())
		{
			return std::nullopt;
		}

		StatementList then_body;
		if (const auto it = json.find("then");
			it != json.end())
		{
			const auto then_path = std::format("{}.then", path);

			scopes_.emplace_back();
			auto list = read_statement_list(it.value(), then_path);
			scopes_.pop_back();

			if (!list.has_value())
			{
				return std::nullopt;
			}

			then_body = *std::move(list);
		}

		StatementList else_body;
		if (const auto it = json.find("else");
			it != json.end())
		{
			const auto else_path = std::format("{}.else", path);

			scopes_.emplace_back();
			auto list = read_statement_list(it.value(), else_path);
			scopes_.pop_back();

			if (!list.has_value())
			{
				return std::nullopt;
			}

			else_body = *std::move(list);
		}

		return If{.condition = *std::move(condition), .then_body = std::move(then_body), .else_body = std::move(else_body)};
	}

	auto Reader::read_foreach_statement_body(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<ForEach>
	{
		LIBASSERT_DEBUG_ASSERT(json.contains("foreach"));
		LIBASSERT_DEBUG_ASSERT(types_.get().contains("group"));

		if (!only_specified_keys(json, {"foreach", "as", "do", "conjunction"}))
		{
			fail(path, "foreach allows only `foreach` / `as` / `do` / `conjunction`");
			return std::nullopt;
		}

		const auto foreach_path = std::format("{}.foreach", path);
		const auto as_path = std::format("{}.as", path);

		const auto foreach_it = json.find("foreach");
		const auto as_it = json.find("as");

		if (as_it == json.end() || !as_it.value().is_string())
		{
			fail(as_path, "must be a string");
			return std::nullopt;
		}

		auto group = read_expression(foreach_it.value(), types_.get().find("group")->id, foreach_path);
		if (!group.has_value())
		{
			return std::nullopt;
		}

		const auto& name = as_it.value().get_ref<const nlohmann::json::string_t&>();
		if (contains_binding(name))
		{
			fail(as_path, std::format("duplicate binding `{}` (shadowing forbidden)", name));
			return std::nullopt;
		}

		const auto binding_id = generate_binding_id();
		auto& scope = scopes_.emplace_back();
		scope.emplace(name, binding_id);

		StatementList body;
		if (const auto do_it = json.find("do");
			do_it != json.end())
		{
			const auto do_path = std::format("{}.do", path);

			auto list = read_statement_list(do_it.value(), do_path);
			if (!list.has_value())
			{
				scopes_.pop_back();
				return std::nullopt;
			}

			body = *std::move(list);
		}

		scopes_.pop_back();

		return ForEach{.group = *std::move(group), .binding = binding_id, .body = std::move(body)};
	}

	auto Reader::read_expression(const nlohmann::json& json, const TypeId expected, const std::string_view path) noexcept -> std::optional<Expression>
	{
		// dsl::Literal::number_type
		if (json.is_number_integer())
		{
			return read_literal_number_expression(json, path);
		}

		// dsl::Literal::boolean_type
		if (json.is_boolean())
		{
			return read_literal_boolean_expression(json, path);
		}

		// dsl::Literal::Symbol / dsl::CandidateRef
		if (json.is_string())
		{
			if (const auto& text = json.get_ref<const nlohmann::json::string_t&>();
				text == CandidateRef::ref)
			{
				return Expression{CandidateRef{}};
			}

			return read_literal_symbol_expression(json, expected, path);
		}

		// dsl::EnumSetConstant
		if (json.is_array())
		{
			return read_enum_set_constant_expression(json, expected, path);
		}

		if (!json.is_object())
		{
			fail(path, std::format("expression must be bool/int/string/object, got `{}`", json.type_name()));
			return std::nullopt;
		}

		// dsl::BindingRef
		if (const auto it = json.find("var");
			it != json.end())
		{
			return read_binding_ref_expression(it, path);
		}

		// dsl::PrimitiveCall
		if (const auto it = json.find("call");
			it != json.end())
		{
			return read_call_expression(json, path);
		}

		// dsl::BooleanOperation
		if (const auto it = json.find(boolean_operator_names[std::to_underlying(BooleanOperator::AND)]);
			it != json.end())
		{
			return read_boolean_operation_expression(it, BooleanOperator::AND, path);
		}
		if (const auto it = json.find(boolean_operator_names[std::to_underlying(BooleanOperator::OR)]);
			it != json.end())
		{
			return read_boolean_operation_expression(it, BooleanOperator::OR, path);
		}
		if (const auto it = json.find(boolean_operator_names[std::to_underlying(BooleanOperator::NOT)]);
			it != json.end())
		{
			return read_boolean_operation_expression(it, BooleanOperator::NOT, path);
		}

		// dsl::CompareOperation
		if (const auto it = json.find(compare_operator_names[std::to_underlying(CompareOperator::EQUAL)]);
			it != json.end())
		{
			return read_compare_operation_expression(it, CompareOperator::EQUAL, path);
		}
		if (const auto it = json.find(compare_operator_names[std::to_underlying(CompareOperator::NOT_EQUAL)]);
			it != json.end())
		{
			return read_compare_operation_expression(it, CompareOperator::NOT_EQUAL, path);
		}
		if (const auto it = json.find(compare_operator_names[std::to_underlying(CompareOperator::LESS_THAN)]);
			it != json.end())
		{
			return read_compare_operation_expression(it, CompareOperator::LESS_THAN, path);
		}
		if (const auto it = json.find(compare_operator_names[std::to_underlying(CompareOperator::LESS_EQUAL)]);
			it != json.end())
		{
			return read_compare_operation_expression(it, CompareOperator::LESS_EQUAL, path);
		}
		if (const auto it = json.find(compare_operator_names[std::to_underlying(CompareOperator::GREATER_THAN)]);
			it != json.end())
		{
			return read_compare_operation_expression(it, CompareOperator::GREATER_THAN, path);
		}
		if (const auto it = json.find(compare_operator_names[std::to_underlying(CompareOperator::GREATER_EQUAL)]);
			it != json.end())
		{
			return read_compare_operation_expression(it, CompareOperator::GREATER_EQUAL, path);
		}

		// dsl::ArithmeticOperation
		if (const auto it = json.find(arithmetic_operator_names[std::to_underlying(ArithmeticOperator::ADD)]);
			it != json.end())
		{
			return read_arithmetic_operation_expression(it, ArithmeticOperator::ADD, path);
		}
		if (const auto it = json.find(arithmetic_operator_names[std::to_underlying(ArithmeticOperator::SUB)]);
			it != json.end())
		{
			return read_arithmetic_operation_expression(it, ArithmeticOperator::SUB, path);
		}
		if (const auto it = json.find(arithmetic_operator_names[std::to_underlying(ArithmeticOperator::MUL)]);
			it != json.end())
		{
			return read_arithmetic_operation_expression(it, ArithmeticOperator::MUL, path);
		}
		if (const auto it = json.find(arithmetic_operator_names[std::to_underlying(ArithmeticOperator::DIV)]);
			it != json.end())
		{
			return read_arithmetic_operation_expression(it, ArithmeticOperator::DIV, path);
		}
		if (const auto it = json.find(arithmetic_operator_names[std::to_underlying(ArithmeticOperator::MIN)]);
			it != json.end())
		{
			return read_arithmetic_operation_expression(it, ArithmeticOperator::MIN, path);
		}
		if (const auto it = json.find(arithmetic_operator_names[std::to_underlying(ArithmeticOperator::MAX)]);
			it != json.end())
		{
			return read_arithmetic_operation_expression(it, ArithmeticOperator::MAX, path);
		}

		fail(path, "unknown expression form");
		return std::nullopt;
	}

	auto Reader::read_literal_number_expression(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<Expression>
	{
		LIBASSERT_DEBUG_ASSERT(json.is_number_integer());

		const auto raw = json.get<nlohmann::json::number_integer_t>();
		if (raw < std::numeric_limits<Literal::number_type>::min() || raw > std::numeric_limits<Literal::number_type>::max())
		{
			fail(path, "Literal.integer out of int32 range");
			return std::nullopt;
		}

		return Expression{Literal{.value = static_cast<Literal::number_type>(raw)}};
	}

	auto Reader::read_literal_boolean_expression(const nlohmann::json& json, [[maybe_unused]] const std::string_view path) noexcept -> std::optional<Expression>
	{
		LIBASSERT_DEBUG_ASSERT(json.is_boolean());

		std::ignore = this;

		const auto b = json.get<nlohmann::json::boolean_t>();
		return Expression{Literal{.value = b}};
	}

	auto Reader::read_literal_symbol_expression(const nlohmann::json& json, const TypeId expected, const std::string_view path) noexcept -> std::optional<Expression>
	{
		LIBASSERT_DEBUG_ASSERT(json.is_string());

		const auto* type = types_.get().find(expected);
		if (type == nullptr || type->symbol_resolver.symbol == nullptr || !(type->value_class == ValueClass::ENUM || type->value_class == ValueClass::SYMBOL))
		{
			fail(path, "string literal requires an enum/symbol-typed context");
			return std::nullopt;
		}

		const auto& text = json.get_ref<const nlohmann::json::string_t&>();
		const auto resolved = type->symbol_resolver.symbol(text);
		if (!resolved.has_value())
		{
			fail(path, std::format("unknown value `{}` for type `{}`", text, type->name));
			return std::nullopt;
		}

		return Expression{Literal{.value = Literal::Symbol{.type = type->id, .value = *resolved}}};
	}

	auto Reader::read_enum_set_constant_expression(const nlohmann::json& json, const TypeId expected, const std::string_view path) noexcept -> std::optional<Expression>
	{
		LIBASSERT_DEBUG_ASSERT(json.is_array());

		using value_type = EnumSetConstant::value_type;

		const auto* type = types_.get().find(expected);
		if (type == nullptr || type->symbol_resolver.symbol == nullptr || type->value_class != ValueClass::ENUM_SET)
		{
			fail(path, "array literal requires an enum-set-typed context");
			return std::nullopt;
		}

		value_type mask = 0;
		for (const auto [index, item]: json | std::views::enumerate)
		{
			auto item_path = std::format("{}[{}]", path, index);
			if (!item.is_string())
			{
				fail(std::move(item_path), "must be a string");
				return std::nullopt;
			}

			const auto& text = item.get_ref<const nlohmann::json::string_t&>();
			const auto bit = type->symbol_resolver.symbol(text);
			if (!bit.has_value() || *bit >= 64)
			{
				fail(std::move(item_path), std::format("unknown value `{}` for type `{}`", text, type->name));
				return std::nullopt;
			}

			const auto flag = value_type{1} << *bit;
			mask |= flag;
		}

		return Expression{EnumSetConstant{.type = type->id, .mask = mask}};
	}

	auto Reader::read_binding_ref_expression(
		const nlohmann::json::const_iterator& binding,
		const std::string_view path
	) noexcept -> std::optional<Expression>
	{
		auto item_path = std::format("{}.var", path);

		if (!binding.value().is_string())
		{
			fail(std::move(item_path), "must be a string");
			return std::nullopt;
		}

		const auto& text = binding.value().get_ref<const nlohmann::json::string_t&>();
		const auto id = find_binding(text);
		if (!id.has_value())
		{
			fail(std::move(item_path), std::format("unknown binding `{}`", text));
			return std::nullopt;
		}

		return Expression{BindingRef{.id = *id}};
	}

	auto Reader::read_call_expression(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<Expression>
	{
		LIBASSERT_DEBUG_ASSERT(json.contains("call"));

		if (!only_specified_keys(json, {"call", "args"}))
		{
			fail(path, "call expression allows only `call` / `args`");
			return std::nullopt;
		}

		auto item_path = std::format("{}.call", path);

		const auto& call = json.at("call");
		if (!call.is_string())
		{
			fail(std::move(item_path), "must be a string");
			return std::nullopt;
		}

		const auto& text = call.get_ref<const nlohmann::json::string_t&>();
		const auto* def = vocabulary_.get().find(text);
		if (def == nullptr)
		{
			fail(std::move(item_path), std::format("unknown primitive `{}`", text));
			return std::nullopt;
		}

		if (def->kind != PrimitiveKind::PREDICATE && def->kind != PrimitiveKind::QUERY)
		{
			fail(std::move(item_path), std::format("primitive `{}` is not usable in an expression", text));
			return std::nullopt;
		}

		auto args_path = std::format("{}.args", path);

		const auto args_it = json.find("args");
		if (args_it == json.end())
		{
			fail(std::move(args_path), "missing");
			return std::nullopt;
		}
		auto args = read_argument_list(*def, args_it, args_path);
		if (!args.has_value())
		{
			return std::nullopt;
		}

		return Expression{PrimitiveCall{.primitive = def->id, .args = *std::move(args)}};
	}

	auto Reader::read_boolean_operation_expression(
		const nlohmann::json::const_iterator& operation,
		const BooleanOperator op,
		const std::string_view path
	) noexcept -> std::optional<Expression>
	{
		const auto item_path = std::format("{}.{}", path, boolean_operator_names[std::to_underlying(op)]);

		if (op == BooleanOperator::NOT)
		{
			auto operand = read_expression(operation.value(), TypeId{TypeId::invalid_id}, item_path);
			if (!operand.has_value())
			{
				return std::nullopt;
			}

			ExpressionList operands{};
			operands.push_back(*std::move(operand));

			return Expression{BooleanOperation{.op = BooleanOperator::NOT, .operands = std::move(operands)}};
		}

		LIBASSERT_DEBUG_ASSERT(op == BooleanOperator::AND || op == BooleanOperator::OR);

		if (!operation.value().is_array() || operation.value().size() < 2)
		{
			fail(item_path, "expects an array of >= 2 operands");
			return std::nullopt;
		}

		ExpressionList operands{};
		operands.reserve(operation.value().size());

		for (const auto [index, operand]: operation.value() | std::views::enumerate)
		{
			const auto nested_item_path = std::format("{}[{}]", item_path, index);
			auto result = read_expression(operand, TypeId{TypeId::invalid_id}, nested_item_path);
			if (!result.has_value())
			{
				return std::nullopt;
			}

			operands.push_back(*std::move(result));
		}

		return Expression{BooleanOperation{.op = op, .operands = std::move(operands)}};
	}

	auto Reader::read_compare_operation_expression(
		const nlohmann::json::const_iterator& operation,
		const CompareOperator op,
		const std::string_view path
	) noexcept -> std::optional<Expression>
	{
		const auto item_path = std::format("{}.{}", path, compare_operator_names[std::to_underlying(op)]);

		if (!operation.value().is_array() || operation.value().size() != 2)
		{
			fail(item_path, "expects exactly 2 operands");
			return std::nullopt;
		}

		const auto left_nested_item_path = std::format("{}[0]", item_path);
		auto left_operand = read_expression(operation[0], TypeId{TypeId::invalid_id}, left_nested_item_path);
		if (!left_operand.has_value())
		{
			return std::nullopt;
		}
		const auto right_nested_item_path = std::format("{}[1]", item_path);
		auto right_operand = read_expression(operation[1], TypeId{TypeId::invalid_id}, right_nested_item_path);
		if (!right_operand.has_value())
		{
			return std::nullopt;
		}

		ExpressionList operands{};
		operands.reserve(2);

		operands.push_back(*std::move(left_operand));
		operands.push_back(*std::move(right_operand));

		return Expression{CompareOperation{.op = op, .operands = std::move(operands)}};
	}

	auto Reader::read_arithmetic_operation_expression(
		const nlohmann::json::const_iterator& operation,
		const ArithmeticOperator op,
		const std::string_view path
	) noexcept -> std::optional<Expression>
	{
		const auto item_path = std::format("{}.{}", path, arithmetic_operator_names[std::to_underlying(op)]);

		if (!operation.value().is_array() || operation.value().size() != 2)
		{
			fail(item_path, "expects exactly 2 operands");
			return std::nullopt;
		}

		const auto left_nested_item_path = std::format("{}[0]", item_path);
		auto left_operand = read_expression(operation[0], TypeId{TypeId::invalid_id}, left_nested_item_path);
		if (!left_operand.has_value())
		{
			return std::nullopt;
		}
		const auto right_nested_item_path = std::format("{}[1]", item_path);
		auto right_operand = read_expression(operation[1], TypeId{TypeId::invalid_id}, right_nested_item_path);
		if (!right_operand.has_value())
		{
			return std::nullopt;
		}

		ExpressionList operands{};
		operands.reserve(2);

		operands.push_back(*std::move(left_operand));
		operands.push_back(*std::move(right_operand));

		return Expression{ArithmeticOperation{.op = op, .operands = std::move(operands)}};
	}

	auto Reader::read_argument_list(const PrimitiveDef& primitive, const nlohmann::json::const_iterator& args, const std::string_view path) noexcept -> std::optional<ExpressionList>
	{
		const auto item_path = std::format("{}.args", path);

		if (const auto required_params_count = static_cast<std::size_t>(std::ranges::count(primitive.params, true, &ParamDef::required));
			!args.value().is_object() || args.value().size() < required_params_count || args.value().size() > primitive.params.size())
		{
			fail(item_path, std::format("primitive `{}` expects [{}..{}] args, got {}", primitive.name, required_params_count, primitive.params.size(), args.value().size()));
			return std::nullopt;
		}

		std::vector<const nlohmann::json*> provided{};
		provided.resize(primitive.params.size());

		for (const auto& [key, value]: args.value().items())
		{
			const auto it = std::ranges::find(primitive.params, key, &ParamDef::name);
			if (it == primitive.params.end())
			{
				fail(item_path, std::format("unknown parameter `{}` for primitive `{}`", key, primitive.name));
				return std::nullopt;
			}

			const auto index = std::ranges::distance(primitive.params.begin(), it);
			if (provided[index] != nullptr)
			{
				fail(item_path, std::format("duplicate parameter `{}`", key));
				return std::nullopt;
			}

			provided[index] = &value;
		}

		ExpressionList list{};
		for (
			// 位置槽无空洞:一旦省略可选参数,其后不得再提供
			auto omitted = false;
			const auto [index, param]: primitive.params | std::views::enumerate)
		{
			const auto nested_item_path = std::format("{}.{}", item_path, param.name);

			if (provided[index] != nullptr)
			{
				if (omitted)
				{
					fail(nested_item_path, std::format("cannot omit earlier optional parameter(s) of `{}`", primitive.name));
					return std::nullopt;
				}

				auto arg = read_expression(*provided[index], param.type, nested_item_path);
				if (!arg)
				{
					return std::nullopt;
				}

				list.push_back(*std::move(arg));
			}
			else if (param.required)
			{
				fail(nested_item_path, std::format("missing required parameter `{}`", param.name));
				return std::nullopt;
			}
			else
			{
				omitted = true;
			}
		}

		return list;
	}

	auto Reader::read_identity(const nlohmann::json::const_iterator& identity, const std::string_view path) noexcept -> std::optional<CardIdentity>
	{
		if (!identity.value().is_object())
		{
			fail(path, "identity must be an object");
			return std::nullopt;
		}

		if (!only_specified_keys(identity.value(), {"kind", "subtypes", "monster", "pendulum", "link_marker", "archetypes", "code_list"}))
		{
			fail(path, "identity has unknown field");
			return std::nullopt;
		}

		domain::CardType kind;
		if (const auto it = identity.value().find("kind");
			it == identity.value().end())
		{
			fail(std::format("{}.kind", path), "is required field");
			return std::nullopt;
		}
		else
		{
			const auto result = read_identity_kind(it, path);
			if (!result.has_value())
			{
				return std::nullopt;
			}

			kind = *result;
		}

		utility::EnumMask<domain::CardSubtype> subtypes;
		if (const auto it = identity.value().find("subtypes");
			it != identity.value().end())
		{
			const auto result = read_identity_subtypes(it, path);
			if (!result.has_value())
			{
				return std::nullopt;
			}

			subtypes = *result;
		}

		std::optional<MonsterIdentity> monster;
		if (const auto it = identity.value().find("monster");
			it != identity.value().end())
		{
			monster = read_identity_monster(it, path);
			if (!monster.has_value())
			{
				return std::nullopt;
			}
		}

		std::optional<PendulumIdentity> pendulum;
		if (const auto it = identity.value().find("pendulum");
			it != identity.value().end())
		{
			pendulum = read_identity_pendulum(it, path);
			if (!pendulum.has_value())
			{
				return std::nullopt;
			}
		}

		std::optional<LinkMarkerIdentity> link_marker;
		if (const auto it = identity.value().find("link_marker");
			it != identity.value().end())
		{
			link_marker = read_identity_link_marker(it, path);
			if (!link_marker.has_value())
			{
				return std::nullopt;
			}
		}

		std::vector<ArchetypeId> archetypes;
		if (const auto it = identity.value().find("archetypes");
			it != identity.value().end())
		{
			auto result = read_identity_archetypes(it, path);
			if (!result.has_value())
			{
				return std::nullopt;
			}

			archetypes = *std::move(result);
		}

		std::vector<domain::CardCode> code_list;
		if (const auto it = identity.value().find("code_list");
			it != identity.value().end())
		{
			auto result = read_identity_code_list(it, path);
			if (!result.has_value())
			{
				return std::nullopt;
			}

			code_list = *std::move(result);
		}

		return CardIdentity
		{
				.code = {},
				.kind = kind,
				.subtypes = subtypes,
				.monster = monster,
				.pendulum = pendulum,
				.link_marker = link_marker,
				.archetypes = std::move(archetypes),
				.code_list = std::move(code_list),
		};
	}

	auto Reader::read_identity_kind(
		const nlohmann::json::const_iterator& kind,
		const std::string_view path
	) noexcept -> std::optional<domain::CardType>
	{
		auto kind_path = std::format("{}.kind", path);

		if (!kind.value().is_string())
		{
			fail(std::move(kind_path), "must be a string");
			return std::nullopt;
		}

		const auto& text = kind.value().get_ref<const nlohmann::json::string_t&>();
		const auto result = utility::from_string<domain::CardType>(text);
		if (!result.has_value())
		{
			fail(std::move(kind_path), "unknown card kind");
			return std::nullopt;
		}

		return result;
	}

	auto Reader::read_identity_subtypes(
		const nlohmann::json::const_iterator& subtypes,
		const std::string_view path
	) noexcept -> std::optional<utility::EnumMask<domain::CardSubtype>>
	{
		auto subtypes_path = std::format("{}.subtypes", path);

		if (!subtypes.value().is_array())
		{
			fail(std::move(subtypes_path), "must be an array");
			return std::nullopt;
		}

		utility::EnumMask<domain::CardSubtype> result{};
		for (const auto [index, item]: subtypes.value() | std::views::enumerate)
		{
			auto subtype_item_path = std::format("{}[{}]", subtypes_path, index);

			if (!item.is_string())
			{
				fail(std::move(subtype_item_path), "must be a string");
				return std::nullopt;
			}

			const auto& text = item.get_ref<const nlohmann::json::string_t&>();
			const auto subtype = utility::from_string<domain::CardSubtype>(text);
			if (!subtype.has_value())
			{
				fail(std::move(subtype_item_path), std::format("unknown subtype `{}`", text));
				return std::nullopt;
			}

			result.add(*subtype);
		}

		return result;
	}

	auto Reader::read_identity_monster(
		const nlohmann::json::const_iterator& monster,
		const std::string_view path
	) noexcept -> std::optional<MonsterIdentity>
	{
		auto monster_path = std::format("{}.monster", path);
		auto attribute_path = std::format("{}.attribute", monster_path);
		auto race_path = std::format("{}.race", monster_path);
		auto level_path = std::format("{}.level", monster_path);
		auto attack_path = std::format("{}.attack", monster_path);
		auto defense_path = std::format("{}.defense", monster_path);

		if (!monster.value().is_object())
		{
			fail(std::move(monster_path), "must be an object");
			return std::nullopt;
		}

		if (!only_specified_keys(monster.value(), {"attribute", "race", "level", "attack", "defense"}))
		{
			fail(std::move(monster_path), "invalid monster block");
			return std::nullopt;
		}

		const auto attribute_it = monster.value().find("attribute");
		if (attribute_it == monster.value().end() || !attribute_it.value().is_string())
		{
			fail(std::move(attribute_path), "must be a string");
			return std::nullopt;
		}
		const auto& attribute_text = attribute_it.value().get_ref<const nlohmann::json::string_t&>();
		const auto attribute = utility::from_string<domain::Attribute>(attribute_text);
		if (!attribute.has_value())
		{
			fail(std::move(attribute_path), "unknown attribute");
			return std::nullopt;
		}

		const auto race_it = monster.value().find("race");
		if (race_it == monster.value().end() || !race_it.value().is_string())
		{
			fail(std::move(race_path), "must be a string");
			return std::nullopt;
		}
		const auto& race_text = race_it.value().get_ref<const nlohmann::json::string_t&>();
		const auto race = utility::from_string<domain::Race>(race_text);
		if (!race.has_value())
		{
			fail(std::move(race_path), "unknown race");
			return std::nullopt;
		}

		const auto level_it = monster.value().find("level");
		if (level_it == monster.value().end() || !level_it.value().is_number_unsigned())
		{
			fail(std::move(level_path), "must be a unsigned integer");
			return std::nullopt;
		}
		const auto level = level_it.value().get<nlohmann::json::number_unsigned_t>();
		// 等级 => 12
		// 阶级 => 13
		if (level > 13)
		{
			fail(std::move(level_path), "cannot greater than 13");
			return std::nullopt;
		}

		const auto attack_it = monster.value().find("attack");
		if (attack_it == monster.value().end() || !attack_it.value().is_number_unsigned())
		{
			fail(std::move(attack_path), "must be a unsigned integer");
			return std::nullopt;
		}
		const auto attack = attack_it.value().get<nlohmann::json::number_unsigned_t>();
		if (attack > std::numeric_limits<std::underlying_type_t<domain::AttackDefense>>::max())
		{
			fail(std::move(attack_path), std::format("must less than {}", std::numeric_limits<std::underlying_type_t<domain::AttackDefense>>::max()));
			return std::nullopt;
		}

		const auto defense_it = monster.value().find("defense");
		if (defense_it != monster.value().end() && !defense_it.value().is_number_unsigned())
		{
			fail(std::move(defense_path), "must be a unsigned integer");
			return std::nullopt;
		}
		const auto defense = defense_it != monster.value().end() ? defense_it.value().get<nlohmann::json::number_unsigned_t>() : 0;
		if (defense > std::numeric_limits<std::underlying_type_t<domain::AttackDefense>>::max())
		{
			fail(std::move(defense_path), std::format("must less than {}", std::numeric_limits<std::underlying_type_t<domain::AttackDefense>>::max()));
			return std::nullopt;
		}

		return MonsterIdentity
		{
				.attribute = *attribute,
				.race = *race,
				.level = static_cast<domain::LevelRank>(static_cast<std::underlying_type_t<domain::LevelRank>>(level)),
				.attack = static_cast<domain::AttackDefense>(static_cast<std::underlying_type_t<domain::AttackDefense>>(attack)),
				.defense = static_cast<domain::AttackDefense>(static_cast<std::underlying_type_t<domain::AttackDefense>>(defense)),
		};
	}

	auto Reader::read_identity_pendulum(
		const nlohmann::json::const_iterator& pendulum,
		const std::string_view path
	) noexcept -> std::optional<PendulumIdentity>
	{
		auto pendulum_path = std::format("{}.pendulum", path);

		if (!pendulum.value().is_object())
		{
			fail(std::move(pendulum_path), "must be an object");
			return std::nullopt;
		}

		if (!only_specified_keys(pendulum.value(), {"left", "right"}))
		{
			fail(std::move(pendulum_path), "pendulum allows only `left` / `right`");
			return std::nullopt;
		}

		const auto left_it = pendulum.value().find("left");
		if (left_it == pendulum.value().end() || !left_it.value().is_number_unsigned())
		{
			fail(std::move(pendulum_path), "must be a unsigned integer");
			return std::nullopt;
		}
		const auto left = left_it.value().get<nlohmann::json::number_unsigned_t>();
		if (left > 13)
		{
			fail(std::move(pendulum_path), "scale out of range 0...13");
			return std::nullopt;
		}

		const auto right_it = pendulum.value().find("right");
		if (right_it == pendulum.value().end() || !right_it.value().is_number_unsigned())
		{
			fail(std::move(pendulum_path), "must be a unsigned integer");
			return std::nullopt;
		}
		const auto right = right_it.value().get<nlohmann::json::number_unsigned_t>();
		if (right > 13)
		{
			fail(std::move(pendulum_path), "scale out of range 0...13");
			return std::nullopt;
		}

		return PendulumIdentity
		{
				.left = static_cast<domain::PendulumScale>(static_cast<std::underlying_type_t<domain::PendulumScale>>(left)),
				.right = static_cast<domain::PendulumScale>(static_cast<std::underlying_type_t<domain::PendulumScale>>(right)),
		};
	}

	auto Reader::read_identity_link_marker(
		const nlohmann::json::const_iterator& link_marker,
		const std::string_view path
	) noexcept -> std::optional<LinkMarkerIdentity>
	{
		auto link_marker_path = std::format("{}.link_marker", path);

		if (!link_marker.value().is_array())
		{
			fail(std::move(link_marker_path), "must be an array");
			return std::nullopt;
		}

		utility::EnumMask<domain::LinkMarker> markers{};
		for (const auto [index, item]: link_marker.value() | std::views::enumerate)
		{
			auto item_path = std::format("{}[{}]", link_marker_path, index);

			if (!item.is_string())
			{
				fail(std::move(item_path), "must be a string");
				return std::nullopt;
			}

			const auto& text = item.get_ref<const nlohmann::json::string_t&>();
			const auto marker = utility::from_string<domain::LinkMarker>(text);
			if (!marker.has_value())
			{
				fail(std::move(item_path), std::format("unknown link marker `{}`", text));
				return std::nullopt;
			}

			markers.add(*marker);
		}

		return LinkMarkerIdentity{.markers = markers};
	}

	auto Reader::read_identity_archetypes(
		const nlohmann::json::const_iterator& archetypes,
		const std::string_view path
	) noexcept -> std::optional<std::vector<ArchetypeId>>
	{
		auto archetypes_path = std::format("{}.archetypes", path);

		if (!archetypes.value().is_array())
		{
			fail(std::move(archetypes_path), "must be an array");
			return std::nullopt;
		}

		std::vector<ArchetypeId> as{};
		as.reserve(archetypes.value().size());

		for (const auto [index, item]: archetypes.value() | std::views::enumerate)
		{
			auto item_path = std::format("{}[{}]", archetypes_path, index);

			if (!item.is_string())
			{
				fail(std::move(item_path), "must be a string");
				return std::nullopt;
			}

			const auto& text = item.get_ref<const nlohmann::json::string_t&>();
			const auto archetype = ArchetypeRegistry::id_of(text);

			as.push_back(archetype);
		}

		return as;
	}

	auto Reader::read_identity_code_list(
		const nlohmann::json::const_iterator& code_list,
		const std::string_view path
	) noexcept -> std::optional<std::vector<domain::CardCode>>
	{
		auto code_list_path = std::format("{}.code_list", path);

		if (!code_list.value().is_array())
		{
			fail(std::move(code_list_path), "must be an array");
			return std::nullopt;
		}

		std::vector<domain::CardCode> cl{};
		cl.reserve(code_list.value().size());

		for (const auto [index, item]: code_list.value() | std::views::enumerate)
		{
			auto item_path = std::format("{}[{}]", code_list_path, index);

			if (!item.is_number_unsigned())
			{
				fail(std::move(item_path), "must be an integer");
				return std::nullopt;
			}

			const auto code = item.get<nlohmann::json::number_unsigned_t>();
			if (code > std::numeric_limits<std::underlying_type_t<domain::CardCode>>::max())
			{
				fail(std::move(item_path), std::format("must less than {}", std::numeric_limits<std::underlying_type_t<domain::CardCode>>::max()));
				return std::nullopt;
			}

			cl.push_back(static_cast<domain::CardCode>(static_cast<std::underlying_type_t<domain::CardCode>>(code)));
		}

		return cl;
	}

	auto Reader::read_summon_list(
		const nlohmann::json::const_iterator& summons,
		const std::string_view path
	) noexcept -> std::optional<SummonProcedureList>
	{
		auto summons_path = std::format("{}.summons", path);

		if (!summons.value().is_array())
		{
			fail(std::move(summons_path), "must be an array");
			return std::nullopt;
		}

		SummonProcedureList list{};
		list.reserve(summons.value().size());

		for (const auto [index, item]: summons.value() | std::views::enumerate)
		{
			const auto item_path = std::format("{}[{}]", summons_path, index);

			auto summon = read_summon(item, item_path);
			if (!summon.has_value())
			{
				return std::nullopt;
			}

			list.push_back(*std::move(summon));
		}

		return list;
	}

	auto Reader::read_summon(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<SummonProcedure>
	{
		if (!json.is_object())
		{
			fail(path, "must be a object");
			return std::nullopt;
		}

		auto method_path = std::format("{}.method", path);

		const auto method_it = json.find("method");
		if (method_it == json.end() || !method_it.value().is_string())
		{
			fail(std::move(method_path), "must be a string");
			return std::nullopt;
		}

		const auto do_read_procedure = [&]<typename Result>(auto (Reader::*f)(const nlohmann::json&, std::string_view) noexcept -> std::optional<Result>) noexcept -> std::optional<SummonProcedure>
		{
			auto procedure = std::invoke(f, this, json, path);
			if (!procedure)
			{
				return std::nullopt;
			}

			return SummonProcedure{*std::move(procedure)};
		};

		const auto& method_text = method_it.value().get_ref<const nlohmann::json::string_t&>();

		if (method_text == "fusion")
		{
			return do_read_procedure(&Reader::read_summon_fusion_procedure);
		}

		if (method_text == "synchro")
		{
			return do_read_procedure(&Reader::read_summon_synchro_procedure);
		}

		if (method_text == "xyz")
		{
			return do_read_procedure(&Reader::read_summon_xyz_procedure);
		}

		if (method_text == "link")
		{
			return do_read_procedure(&Reader::read_summon_link_procedure);
		}

		if (method_text == "ritual")
		{
			return do_read_procedure(&Reader::read_summon_ritual_procedure);
		}

		if (method_text == "special")
		{
			return do_read_procedure(&Reader::read_summon_self_special_procedure);
		}

		fail(std::move(method_path), std::format("unknown summon method `{}`", method_text));
		return std::nullopt;
	}

	auto Reader::read_summon_fusion_procedure(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<FusionProcedure>
	{
		LIBASSERT_DEBUG_ASSERT(json.is_object());
		LIBASSERT_DEBUG_ASSERT(json.contains("method") && json.at("method").is_string() && json.at("method").get_ref<const nlohmann::json::string_t&>() == "fusion");

		if (!only_specified_keys(json, {"method", "materials"}))
		{
			fail(path, "allows only `method` / `materials`");
			return std::nullopt;
		}

		auto materials_path = std::format("{}.materials", path);

		MaterialSpecList materials{};
		if (const auto it = json.find("materials");
			it != json.end())
		{
			if (!it.value().is_array())
			{
				fail(std::move(materials_path), "must be an array");
				return std::nullopt;
			}

			materials.reserve(it.value().size());

			for (const auto [index, item]: it.value() | std::views::enumerate)
			{
				auto item_path = std::format("{}[{}]", materials_path, index);

				auto material = read_summon_fusion_procedure_material(item, item_path);
				if (!material.has_value())
				{
					return std::nullopt;
				}

				materials.push_back(*std::move(material));
			}
		}

		return FusionProcedure{.materials = std::move(materials)};
	}

	auto Reader::read_summon_fusion_procedure_material(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<MaterialSpec>
	{
		LIBASSERT_DEBUG_ASSERT(types_.get().contains("bool"));

		if (!json.is_object())
		{
			fail(path, "must be an object");
			return std::nullopt;
		}

		if (!only_specified_keys(json, {"card", "filter", "count"}))
		{
			fail(path, "allows only `card` / `filter` / `count`");
			return std::nullopt;
		}

		auto card_path = std::format("{}.card", path);
		const auto filter_path = std::format("{}.filter", path);

		const auto card_it = json.find("card");
		const auto filter_it = json.find("filter");

		const auto has_card = card_it != json.end();
		const auto has_filter = filter_it != json.end();

		if (has_card == has_filter)
		{
			fail(path, "needs exactly one of `card` / `filter`");
			return std::nullopt;
		}

		MaterialSpec::selector_type selector{};
		if (has_card)
		{
			if (!card_it.value().is_number_unsigned())
			{
				fail(std::move(card_path), "must be an unsigned integer");
				return std::nullopt;
			}

			const auto card = card_it.value().get<nlohmann::json::number_unsigned_t>();
			if (card > std::numeric_limits<std::underlying_type_t<domain::CardCode>>::max())
			{
				fail(std::move(card_path), std::format("must less than {}", std::numeric_limits<std::underlying_type_t<domain::CardCode>>::max()));
				return std::nullopt;
			}

			selector = static_cast<domain::CardCode>(static_cast<std::underlying_type_t<domain::CardCode>>(card));
		}
		else if (has_filter)
		{
			auto filter = read_expression(filter_it.value(), types_.get().find("bool")->id, filter_path);
			if (!filter.has_value())
			{
				return std::nullopt;
			}

			selector = *std::move(filter);
		}
		else
		{
			std::unreachable();
		}

		MaterialSpec::size_type count = 1;
		if (const auto it = json.find("count");
			it != json.end())
		{
			auto count_path = std::format("{}.count", path);

			if (!it.value().is_number_unsigned())
			{
				fail(std::move(count_path), "must be an unsigned integer");
				return std::nullopt;
			}

			const auto v = it.value().get<nlohmann::json::number_unsigned_t>();
			if (v == 0 || v > std::numeric_limits<MaterialSpec::size_type>::max())
			{
				fail(std::move(count_path), std::format("must greater than 0 and less than {}", std::numeric_limits<MaterialSpec::size_type>::max()));
				return std::nullopt;
			}

			count = static_cast<MaterialSpec::size_type>(v);
		}

		return MaterialSpec{.selector = std::move(selector), .count = count};
	}

	auto Reader::read_summon_synchro_procedure(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<SynchroProcedure>
	{
		LIBASSERT_DEBUG_ASSERT(json.is_object());
		LIBASSERT_DEBUG_ASSERT(json.contains("method") && json.at("method").is_string() && json.at("method").get_ref<const nlohmann::json::string_t&>() == "synchro");
		LIBASSERT_DEBUG_ASSERT(types_.get().contains("bool"));

		if (!only_specified_keys(json, {"method", "tuner", "non_tuner"}))
		{
			fail(path, "allows only `method` / `tuner` / `non_tuner`");
			return std::nullopt;
		}

		const auto tuner_path = std::format("{}.tuner", path);
		const auto non_tuner_path = std::format("{}.non_tuner", path);

		const auto tuner_it = json.find("tuner");
		const auto non_tuner_it = json.find("non_tuner");

		if (tuner_it == json.end() || non_tuner_it == json.end())
		{
			fail(path, "synchro requires `tuner` / `non_tuner`");
			return std::nullopt;
		}

		auto tuner = read_expression(tuner_it.value(), types_.get().find("bool")->id, tuner_path);
		if (!tuner.has_value())
		{
			return std::nullopt;
		}
		auto non_tuner = read_expression(non_tuner_it.value(), types_.get().find("bool")->id, non_tuner_path);
		if (!non_tuner.has_value())
		{
			return std::nullopt;
		}

		return SynchroProcedure{.tuner = *std::move(tuner), .non_tuner = *std::move(non_tuner)};
	}

	auto Reader::read_summon_xyz_procedure(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<XyzProcedure>
	{
		LIBASSERT_DEBUG_ASSERT(json.is_object());
		LIBASSERT_DEBUG_ASSERT(json.contains("method") && json.at("method").is_string() && json.at("method").get_ref<const nlohmann::json::string_t&>() == "xyz");
		LIBASSERT_DEBUG_ASSERT(types_.get().contains("bool"));

		if (!only_specified_keys(json, {"method", "material", "count"}))
		{
			fail(path, "allows only `method` / `material` / `count`");
			return std::nullopt;
		}

		const auto material_path = std::format("{}.material", path);

		const auto material_it = json.find("material");
		if (material_it == json.end())
		{
			fail(path, "xyz requires `material`");
			return std::nullopt;
		}
		auto material = read_expression(material_it.value(), types_.get().find("bool")->id, material_path);
		if (!material.has_value())
		{
			return std::nullopt;
		}

		XyzProcedure::size_type count = 0;
		if (const auto it = json.find("count");
			it != json.end())
		{
			auto count_path = std::format("{}.count", path);

			if (!it.value().is_number_unsigned())
			{
				fail(std::move(count_path), "must be an unsigned integer");
				return std::nullopt;
			}

			const auto v = it.value().get<nlohmann::json::number_unsigned_t>();
			if (v == 0 || v > std::numeric_limits<XyzProcedure::size_type>::max())
			{
				fail(std::move(count_path), std::format("must greater than 0 and less than {}", std::numeric_limits<XyzProcedure::size_type>::max()));
				return std::nullopt;
			}

			count = static_cast<XyzProcedure::size_type>(v);
		}

		return XyzProcedure{.material = *std::move(material), .count = count};
	}

	auto Reader::read_summon_link_procedure(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<LinkProcedure>
	{
		LIBASSERT_DEBUG_ASSERT(json.is_object());
		LIBASSERT_DEBUG_ASSERT(json.contains("method") && json.at("method").is_string() && json.at("method").get_ref<const nlohmann::json::string_t&>() == "link");
		LIBASSERT_DEBUG_ASSERT(types_.get().contains("bool"));

		if (!only_specified_keys(json, {"method", "material", "count"}))
		{
			fail(path, "allows only `method` / `material` / `count`");
			return std::nullopt;
		}

		const auto material_path = std::format("{}.material", path);

		const auto material_it = json.find("material");
		if (material_it == json.end())
		{
			fail(path, "link requires `material`");
			return std::nullopt;
		}
		auto material = read_expression(material_it.value(), types_.get().find("bool")->id, material_path);
		if (!material.has_value())
		{
			return std::nullopt;
		}

		LinkProcedure::size_type count = 0;
		if (const auto it = json.find("count"); it != json.end())
		{
			auto count_path = std::format("{}.count", path);

			if (!it.value().is_number_unsigned())
			{
				fail(std::move(count_path), "must be an unsigned integer");
				return std::nullopt;
			}

			const auto v = it.value().get<nlohmann::json::number_unsigned_t>();
			if (v == 0 || v > std::numeric_limits<LinkProcedure::size_type>::max())
			{
				fail(std::move(count_path), std::format("must greater than 0 and less than {}", std::numeric_limits<LinkProcedure::size_type>::max()));
				return std::nullopt;
			}

			count = static_cast<LinkProcedure::size_type>(v);
		}

		return LinkProcedure{.material = *std::move(material), .count = count};
	}

	auto Reader::read_summon_ritual_procedure(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<RitualProcedure>
	{
		LIBASSERT_DEBUG_ASSERT(json.is_object());
		LIBASSERT_DEBUG_ASSERT(json.contains("method") && json.at("method").is_string() && json.at("method").get_ref<const nlohmann::json::string_t&>() == "ritual");
		LIBASSERT_DEBUG_ASSERT(types_.get().contains("bool") && types_.get().contains("int"));

		if (!only_specified_keys(json, {"method", "material", "level"}))
		{
			fail(path, "allows only `method` / `material` / `level`");
			return std::nullopt;
		}

		const auto material_path = std::format("{}.material", path);
		const auto level_path = std::format("{}.level", path);

		const auto material_it = json.find("material");
		const auto level_it = json.find("level");

		if (material_it == json.end() || level_it == json.end())
		{
			fail(path, "ritual requires `material` / `level`");
			return std::nullopt;
		}

		auto material = read_expression(material_it.value(), types_.get().find("bool")->id, material_path);
		if (!material.has_value())
		{
			return std::nullopt;
		}
		auto level = read_expression(level_it.value(), types_.get().find("int")->id, level_path);
		if (!level.has_value())
		{
			return std::nullopt;
		}

		return RitualProcedure{.material = *std::move(material), .level = *std::move(level)};
	}

	auto Reader::read_summon_self_special_procedure(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<SelfSpecialProcedure>
	{
		LIBASSERT_DEBUG_ASSERT(json.is_object());
		LIBASSERT_DEBUG_ASSERT(json.contains("method") && json.at("method").is_string() && json.at("method").get_ref<const nlohmann::json::string_t&>() == "special");
		LIBASSERT_DEBUG_ASSERT(types_.get().contains("bool"));

		if (!only_specified_keys(json, {"method", "condition", "cost", "source_zones"}))
		{
			fail(path, "allows only `method` / `condition` / `cost` / `source_zones`");
			return std::nullopt;
		}

		std::optional<Expression> condition;
		StatementList cost;
		bool error = false;

		scopes_.emplace_back();
		if (const auto it = json.find("condition");
			it != json.end())
		{
			const auto condition_path = std::format("{}.condition", path);

			condition = read_expression(it.value(), types_.get().find("bool")->id, condition_path);
			if (!condition.has_value())
			{
				error = true;
			}
		}
		if (!error)
		{
			if (const auto it = json.find("cost");
				it != json.end())
			{
				const auto cost_path = std::format("{}.cost", path);

				if (auto c = read_statement_list(it.value(), cost_path);
					!c.has_value())
				{
					error = true;
				}
				else
				{
					cost = *std::move(c);
				}
			}
		}
		scopes_.pop_back();

		if (error)
		{
			return std::nullopt;
		}

		utility::EnumMask<domain::Zone> source_zones{};
		if (const auto it = json.find("source_zones");
			it != json.end())
		{
			auto source_zones_path = std::format("{}.source_zones", path);

			if (!it.value().is_array())
			{
				fail(std::move(source_zones_path), "must be an array");
				return std::nullopt;
			}

			for (const auto [index, item]: it.value() | std::views::enumerate)
			{
				auto item_path = std::format("{}[{}]", source_zones_path, index);

				if (!item.is_string())
				{
					fail(std::move(item_path), "must be a string");
					return std::nullopt;
				}

				const auto& text = it.value().get_ref<const nlohmann::json::string_t&>();
				const auto zone = utility::from_string<domain::Zone>(text);
				if (!zone.has_value())
				{
					fail(std::move(item_path), "unknown zone");
					return std::nullopt;
				}

				source_zones.add(*zone);
			}
		}

		return SelfSpecialProcedure{.condition = std::move(condition), .cost = std::move(cost), .source_zones = source_zones};
	}

	auto Reader::read_effect_list(
		const nlohmann::json::const_iterator& effects,
		const std::string_view path
	) noexcept -> std::optional<EffectList>
	{
		if (!effects.value().is_array())
		{
			fail(path, "must be an array");
			return std::nullopt;
		}

		EffectList list{};
		list.reserve(effects.value().size());

		for (const auto [index, item]: effects.value() | std::views::enumerate)
		{
			const auto item_path = std::format("{}[{}]", path, index);

			auto body = read_effect_body(item, item_path);
			if (!body.has_value())
			{
				return std::nullopt;
			}

			// TODO: EffectId如何设计?是全局ID,还是卡牌内效果ID?
			EffectId id{static_cast<EffectId::value_type>(index + 1)};
			list.emplace_back(id, *std::move(body));
		}

		return list;
	}

	auto Reader::read_effect_body(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<EffectBody>
	{
		if (!json.is_object())
		{
			fail(path, "must be an object");
			return std::nullopt;
		}

		auto type_path = std::format("{}.type", path);

		const auto type_it = json.find("type");
		if (type_it == json.end() || !type_it.value().is_string())
		{
			fail(std::move(type_path), "missing or not a string");
			return std::nullopt;
		}
		const auto& type = type_it.value().get_ref<const nlohmann::json::string_t&>();

		if (type == "activated")
		{
			auto body = read_activated_effect_body(json, path);
			if (!body.has_value())
			{
				return std::nullopt;
			}

			return EffectBody{*std::move(body)};
		}

		if (type == "static")
		{
			auto body = read_static_effect_body(json, path);
			if (!body.has_value())
			{
				return std::nullopt;
			}

			return EffectBody{*std::move(body)};
		}

		fail(std::move(type_path), std::format("unknown effect type `{}`", type));
		return std::nullopt;
	}

	auto Reader::read_activated_effect_body(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<ActivatedEffect>
	{
		LIBASSERT_DEBUG_ASSERT(json.is_object());
		LIBASSERT_DEBUG_ASSERT(json.contains("type") && json.at("type").is_string() && json.at("type").get_ref<const nlohmann::json::string_t&>() == "activated");
		LIBASSERT_DEBUG_ASSERT(types_.get().contains("bool"));

		if (!only_specified_keys(json, {"type", "activation", "zones", "condition", "cost", "target", "effect"}))
		{
			fail(path, "activated effect allows only `type` / `activation` / `zones` / `condition` / `cost` / `target` / `effect`");
			return std::nullopt;
		}

		auto activation_path = std::format("{}.activation", path);

		const auto activation_it = json.find("activation");
		if (activation_it == json.end())
		{
			fail(std::move(activation_path), "missing");
			return std::nullopt;
		}
		auto activation = read_activated_effect_activation(activation_it, activation_path);
		if (!activation.has_value())
		{
			return std::nullopt;
		}

		utility::EnumMask<domain::Zone> activation_zones{};
		if (const auto it = json.find("zones");
			it != json.end())
		{
			auto zones_path = std::format("{}.zones", path);

			if (!it.value().is_array())
			{
				fail(std::move(zones_path), "must be an array");
				return std::nullopt;
			}

			for (const auto [index, item]: it.value() | std::views::enumerate)
			{
				auto item_path = std::format("{}[{}]", zones_path, index);

				if (!item.is_string())
				{
					fail(std::move(item_path), "must be a string");
					return std::nullopt;
				}
				const auto& text = item.get_ref<const nlohmann::json::string_t&>();
				const auto zone = utility::from_string<domain::Zone>(text);
				if (!zone.has_value())
				{
					fail(std::move(item_path), std::format("unknown zone `{}`", text));
					return std::nullopt;
				}

				activation_zones.add(*zone);
			}
		}

		std::optional<Expression> condition;
		if (const auto it = json.find("condition");
			it != json.end())
		{
			const auto condition_path = std::format("{}.condition", path);

			condition = read_expression(it.value(), types_.get().find("bool")->id, condition_path);
			if (!condition.has_value())
			{
				return std::nullopt;
			}
		}

		StatementList cost;
		StatementList target;
		StatementList effect;
		bool error = false;
		const auto do_read = [&](const std::string_view name, StatementList& out) noexcept -> void
		{
			if (error)
			{
				return;
			}

			if (const auto it = json.find(name);
				it != json.end())
			{
				const auto name_path = std::format("{}.{}", path, name);

				if (auto list = read_statement_list(it.value(), name_path);
					!list.has_value())
				{
					error = true;
				}
				else
				{
					out = *std::move(list);
				}
			}
		};

		// 效果帧作用域(cost/target/effect共享)
		scopes_.emplace_back();
		do_read("cost", cost);
		do_read("target", target);
		do_read("effect", effect);
		scopes_.pop_back();

		if (error)
		{
			return std::nullopt;
		}

		return ActivatedEffect
		{
				.activation = *std::move(activation),
				.activation_zones = activation_zones,
				.condition = std::move(condition),
				.cost = std::move(cost),
				.target = std::move(target),
				.effect = std::move(effect),
		};
	}

	auto Reader::read_activated_effect_activation(
		const nlohmann::json::const_iterator& activation,
		const std::string_view path
	) noexcept -> std::optional<ActivationKind>
	{
		if (!activation.value().is_object())
		{
			fail(path, "must be an object");
			return std::nullopt;
		}

		auto mode_path = std::format("{}.mode", path);

		const auto mode_it = activation.value().find("mode");
		if (mode_it == activation.value().end() || !mode_it.value().is_string())
		{
			fail(std::move(mode_path), "must be a string");
			return std::nullopt;
		}

		const auto& mode = mode_it.value().get_ref<const nlohmann::json::string_t&>();

		if (mode == "activation")
		{
			if (!only_specified_keys(activation.value(), {"mode"}))
			{
				fail(path, "activation allows only `mode`");
				return std::nullopt;
			}

			return ActivationKind{CardActivation{}};
		}

		if (mode == "ignition")
		{
			if (!only_specified_keys(activation.value(), {"mode"}))
			{
				fail(path, "ignition allows only `mode`");
				return std::nullopt;
			}

			return ActivationKind{Ignition{}};
		}

		if (mode == "trigger")
		{
			if (!only_specified_keys(activation.value(), {"mode", "source", "timing", "optional"}))
			{
				fail(path, "trigger allows only `mode` / `source` / `timing` / `optional`");
				return std::nullopt;
			}

			auto source_path = std::format("{}.source", path);

			const auto source_it = activation.value().find("source");
			if (source_it == activation.value().end())
			{
				fail(std::move(source_path), "missing");
				return std::nullopt;
			}
			auto sources = read_activated_effect_activation_trigger_source_list(source_it.value(), source_path);
			if (!sources.has_value())
			{
				return std::nullopt;
			}

			auto timing = TriggerTiming::WHEN;
			if (const auto it = activation.value().find("timing");
				it != activation.value().end())
			{
				auto timing_path = std::format("{}.timing", path);

				if (!it.value().is_string())
				{
					fail(std::move(timing_path), "must be a string");
					return std::nullopt;
				}
				const auto& text = it.value().get_ref<const nlohmann::json::string_t&>();
				const auto e = utility::from_string<TriggerTiming>(text);
				if (!e.has_value())
				{
					fail(std::move(timing_path), std::format("unknown trigger timing `{}`", text));
					return std::nullopt;
				}

				timing = *e;
			}

			auto optional = false;
			if (const auto it = activation.value().find("optional");
				it != activation.value().end())
			{
				auto optional_path = std::format("{}.optional", path);

				if (!it.value().is_boolean())
				{
					fail(std::move(optional_path), "must be a boolean");
					return std::nullopt;
				}
				const auto b = it.value().get<nlohmann::json::boolean_t>();

				optional = b;
			}

			return ActivationKind{Trigger{.sources = *std::move(sources), .timing = timing, .optional = optional}};
		}

		if (mode == "quick")
		{
			if (!only_specified_keys(activation.value(), {"mode"}))
			{
				fail(path, "quick allows only `mode`");
				return std::nullopt;
			}

			return ActivationKind{Quick{}};
		}

		fail(std::move(mode_path), std::format("unknown activation mode `{}`", mode));
		return std::nullopt;
	}

	auto Reader::read_activated_effect_activation_trigger_source_list(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<TriggerSourceList>
	{
		// [{"event":…}, {"phase":…}]
		if (json.is_array())
		{
			if (json.empty())
			{
				fail(path, "trigger source list must not be empty");
				return std::nullopt;
			}

			TriggerSourceList sources{};
			sources.reserve(json.size());

			for (const auto [index, item]: json | std::views::enumerate)
			{
				const auto item_path = std::format("{}[{}]", path, index);

				const auto source = read_activated_effect_activation_trigger_source(item, item_path);
				if (!source.has_value())
				{
					return std::nullopt;
				}

				sources.push_back(*source);
			}

			return sources;
		}

		// {"event":…}
		if (json.is_object())
		{
			const auto source = read_activated_effect_activation_trigger_source(json, path);
			if (!source.has_value())
			{
				return std::nullopt;
			}

			TriggerSourceList list{};
			list.reserve(1);
			list.push_back(*source);

			return list;
		}

		fail(path, "unknown trigger list type");
		return std::nullopt;
	}

	auto Reader::read_activated_effect_activation_trigger_source(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<TriggerSource>
	{
		LIBASSERT_DEBUG_ASSERT(json.is_object());

		if (!json.is_object())
		{
			fail(path, "must be an object");
			return std::nullopt;
		}

		if (!only_specified_keys(json, {"event", "phase", "battle_step", "damage_step"}))
		{
			fail(path, "allows only `event` / `phase` / `battle_step` / `damage_step`");
			return std::nullopt;
		}

		const auto do_parse = [&]<typename EnumType,typename Result>(const std::string_view name, const nlohmann::json::const_iterator& it) noexcept -> std::optional<TriggerSource>
		{
			auto name_path = std::format("{}.{}", path, name);

			if (!it.value().is_string())
			{
				fail(std::move(name_path), "must be a string");
				return std::nullopt;
			}

			const auto& text = it.value().get_ref<const nlohmann::json::string_t&>();
			const auto e = utility::from_string<EnumType>(text);
			if (!e.has_value())
			{
				fail(std::move(name_path), std::format("unknown {} `{}`", name, text));
				return std::nullopt;
			}

			return TriggerSource{Result{*e}};
		};

		if (const auto it = json.find("event");
			it != json.end())
		{
			return do_parse.operator()<domain::EventCode, domain::EventCode>("event", it);
		}

		if (const auto it = json.find("phase");
			it != json.end())
		{
			return do_parse.operator()<domain::Phase, PeriodRef>("phase", it);
		}

		if (const auto it = json.find("battle_step");
			it != json.end())
		{
			return do_parse.operator()<domain::BattleStep, PeriodRef>("battle_step", it);
		}

		if (const auto it = json.find("damage_step");
			it != json.end())
		{
			return do_parse.operator()<domain::DamageStep, PeriodRef>("damage_step", it);
		}

		fail(path, "trigger source requires one of `event` / `phase` / `battle_step` / `damage_step`");
		return std::nullopt;
	}

	auto Reader::read_static_effect_body(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<StaticEffect>
	{
		LIBASSERT_DEBUG_ASSERT(json.is_object());
		LIBASSERT_DEBUG_ASSERT(json.contains("type") && json.at("type").is_string() && json.at("type").get_ref<const nlohmann::json::string_t&>() == "static");
		LIBASSERT_DEBUG_ASSERT(types_.get().contains("bool"));

		if (!only_specified_keys(json, {"type", "apply_to", "condition", "modifiers"}))
		{
			fail(path, "static effect allows only `type` / `apply_to` / `condition` / `modifiers`");
			return std::nullopt;
		}

		auto apply_to_path = std::format("{}.apply_to", path);

		const auto apply_to_it = json.find("apply_to");
		if (apply_to_it == json.end())
		{
			fail(std::move(apply_to_path), "missing");
			return std::nullopt;
		}
		auto apply_to = read_expression(apply_to_it.value(), types_.get().find("bool")->id, apply_to_path);
		if (!apply_to.has_value())
		{
			return std::nullopt;
		}

		std::optional<Expression> condition;
		if (const auto it = json.find("condition");
			it != json.end())
		{
			const auto condition_path = std::format("{}.condition", path);

			condition = read_expression(it.value(), types_.get().find("bool")->id, condition_path);
			if (!condition.has_value())
			{
				return std::nullopt;
			}
		}

		StaticModifierList modifiers;
		if (const auto it = json.find("modifiers");
			it != json.end())
		{
			auto modifiers_path = std::format("{}.modifiers", path);

			if (!it.value().is_array())
			{
				fail(std::move(modifiers_path), "must be an array");
				return std::nullopt;
			}

			modifiers.reserve(it.value().size());

			for (const auto [index, item]: it.value() | std::views::enumerate)
			{
				const auto item_path = std::format("{}[{}]", modifiers_path, index);

				auto modifier = read_static_effect_modifier(item, item_path);
				if (!modifier.has_value())
				{
					return std::nullopt;
				}

				modifiers.push_back(*std::move(modifier));
			}
		}

		return StaticEffect{.apply_to = *std::move(apply_to), .condition = std::move(condition), .modifiers = std::move(modifiers)};
	}

	auto Reader::read_static_effect_modifier(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<StaticModifier>
	{
		if (!json.is_object())
		{
			fail(path, "must be an object");
			return std::nullopt;
		}

		if (!only_specified_keys(json, {"call", "args"}))
		{
			fail(path, "allows only `call` / `args`");
			return std::nullopt;
		}

		auto call_path = std::format("{}.call", path);

		const auto call_it = json.find("call");
		if (call_it == json.end() || !call_it.value().is_string())
		{
			fail(std::move(call_path), "missing or not a string");
			return std::nullopt;
		}

		const auto& call = call_it.value().get_ref<const nlohmann::json::string_t&>();
		const auto* def = vocabulary_.get().find(call);
		if (def == nullptr)
		{
			fail(std::move(call_path), std::format("unknown primitive `{}`", call));
			return std::nullopt;
		}

		if (def->kind != PrimitiveKind::STATIC)
		{
			fail(std::move(call_path), std::format("primitive `{}` is not a static modifier", def->name));
			return std::nullopt;
		}

		ExpressionList args;
		if (const auto it = json.find("args");
			it != json.end())
		{
			const auto args_path = std::format("{}.args", path);

			auto list = read_argument_list(*def, it, args_path);
			if (!list.has_value())
			{
				return std::nullopt;
			}

			args = *std::move(list);
		}

		return StaticModifier{.primitive = def->id, .args = std::move(args)};
	}

	auto Reader::read_usage_restriction_list(
		const nlohmann::json::const_iterator& usage_restrictions,
		const std::string_view path
	) noexcept -> std::optional<UsageRestrictionList>
	{
		if (!usage_restrictions.value().is_array())
		{
			fail(path, "must be an array");
			return std::nullopt;
		}

		UsageRestrictionList list{};
		list.reserve(usage_restrictions.value().size());

		for (const auto [index, item]: usage_restrictions.value() | std::views::enumerate)
		{
			const auto item_path = std::format("{}[{}]", path, index);

			auto usage_restriction = read_usage_restriction(item, item_path);
			if (!usage_restriction.has_value())
			{
				return std::nullopt;
			}

			list.push_back(*std::move(usage_restriction));
		}

		return list;
	}

	auto Reader::read_usage_restriction(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<UsageRestriction>
	{
		if (!json.is_object())
		{
			fail(path, "must be an object");
			return std::nullopt;
		}

		if (!only_specified_keys(json, {"scope", "basis", "verb", "count", "refs"}))
		{
			fail(path, "allows only `scope` / `basis` / `verb` / `count` / `refs`");
			return std::nullopt;
		}

		const auto do_parse = [&]<typename EnumType>(const std::string_view name, const nlohmann::json::const_iterator& it) noexcept -> std::optional<EnumType>
		{
			auto name_path = std::format("{}.{}", path, name);

			if (!it.value().is_string())
			{
				fail(std::move(name_path), "must be a string");
				return std::nullopt;
			}

			const auto& text = it.value().get_ref<const nlohmann::json::string_t&>();
			const auto e = utility::from_string<EnumType>(text);
			if (!e.has_value())
			{
				fail(std::move(name_path), std::format("unknown {} `{}`", name, text));
				return std::nullopt;
			}

			return e;
		};

		auto scope = UsageScope::TURN;
		if (const auto it = json.find("scope");
			it != json.end())
		{
			const auto e = do_parse.operator()<UsageScope>("scope", it);
			if (!e.has_value())
			{
				return std::nullopt;
			}

			scope = *e;
		}

		auto basis = UsageBasis::THIS_CARD;
		if (const auto it = json.find("basis");
			it != json.end())
		{
			const auto e = do_parse.operator()<UsageBasis>("basis", it);
			if (!e.has_value())
			{
				return std::nullopt;
			}

			basis = *e;
		}

		auto verb = RestrictionVerb::USE;
		if (const auto it = json.find("verb");
			it != json.end())
		{
			const auto e = do_parse.operator()<RestrictionVerb>("verb", it);
			if (!e.has_value())
			{
				return std::nullopt;
			}

			verb = *e;
		}

		auto count = UsageRestriction::size_type{1};
		if (const auto it = json.find("count");
			it != json.end())
		{
			auto count_path = std::format("{}.count", path);

			if (!it.value().is_number_unsigned())
			{
				fail(std::move(count_path), "must be a unsigned integer");
				return std::nullopt;
			}

			const auto v = it.value().get<nlohmann::json::number_unsigned_t>();
			if (v == 0 || v > std::numeric_limits<UsageRestriction::size_type>::max())
			{
				fail(std::move(count_path), std::format("must greater than 0 and less than {}", std::numeric_limits<UsageRestriction::size_type>::max()));
				return std::nullopt;
			}

			count = static_cast<UsageRestriction::size_type>(v);
		}

		auto refs_path = std::format("{}.refs", path);

		auto refs_it = json.find("refs");
		if (refs_it == json.end() || !refs_it.value().is_array())
		{
			fail(std::move(refs_path), "missing or not an array");
			return std::nullopt;
		}

		std::vector<UsageRestriction::index_type> refs;
		refs.reserve(refs_it.value().size());

		for (const auto [index, item]: refs_it.value() | std::views::enumerate)
		{
			auto item_path = std::format("{}[{}]", refs_path, index);

			if (!item.is_number_unsigned())
			{
				fail(std::move(item_path), "ref index must be a unsigned integer");
				return std::nullopt;
			}

			const auto v = item.get<nlohmann::json::number_unsigned_t>();
			if (v > std::numeric_limits<UsageRestriction::index_type>::max())
			{
				fail(std::move(item_path), "ref index out of range");
				return std::nullopt;
			}

			refs.push_back(static_cast<UsageRestriction::index_type>(v));
		}

		return UsageRestriction{.scope = scope, .basis = basis, .verb = verb, .count = count, .refs = std::move(refs)};
	}

	Reader::Reader(const TypeRegistry& types, const Vocabulary& vocabulary) noexcept
		: types_{types},
		  vocabulary_{vocabulary},
		  binding_id_{1}
	{
		// 根作用域
		scopes_.emplace_back();
	}

	auto Reader::diagnostics() const noexcept -> const Diagnostics&
	{
		return diagnostics_;
	}

	auto Reader::clear_diagnostics() noexcept -> void
	{
		diagnostics_.clear();
	}

	auto Reader::read_card_script(const nlohmann::json& json, const std::string_view path) noexcept -> std::optional<CardScript>
	{
		if (!json.is_object())
		{
			fail(path, "card script must be an object");
			return std::nullopt;
		}

		if (!only_specified_keys(json, {"identity", "summons", "effects", "usage_restrictions"}))
		{
			fail(path, "card script allows only `identity` / `summons` / `effects` / `usage_restrictions`");
			return std::nullopt;
		}

		auto identity_path = std::format("{}.identity", path);

		CardIdentity identity;
		if (const auto it = json.find("identity");
			it == json.end())
		{
			fail(std::move(identity_path), "missing");
			return std::nullopt;
		}
		else
		{
			auto i = read_identity(it, identity_path);
			if (!i.has_value())
			{
				return std::nullopt;
			}

			identity = *std::move(i);
		}

		SummonProcedureList summons;
		if (const auto it = json.find("summons");
			it != json.end())
		{
			const auto item_path = std::format("{}.summons", path);

			auto list = read_summon_list(it, item_path);
			if (!list.has_value())
			{
				return std::nullopt;
			}

			summons = *std::move(list);
		}

		EffectList effects;
		if (const auto it = json.find("effects");
			it != json.end())
		{
			const auto item_path = std::format("{}.effects", path);

			auto list = read_effect_list(it, item_path);
			if (!list.has_value())
			{
				return std::nullopt;
			}

			effects = *std::move(list);
		}

		UsageRestrictionList usage_restrictions;
		if (const auto it = json.find("usage_restrictions");
			it != json.end())
		{
			const auto item_path = std::format("{}.usage_restrictions", path);

			auto list = read_usage_restriction_list(it, item_path);
			if (!list.has_value())
			{
				return std::nullopt;
			}

			// 检查引用的效果是否存在
			bool error = false;
			for (const auto [usage_restriction_index,usage_restriction]: *list | std::views::enumerate)
			{
				for (const auto [ref_index, ref]: usage_restriction.refs | std::views::enumerate)
				{
					if (ref >= effects.size())
					{
						auto ref_path = std::format("{}[{}].refs[{}]", item_path, usage_restriction_index, ref_index);

						fail(std::move(ref_path), std::format("ref out of index, max is {}", effects.size()));
						error = true;
					}
				}
			}
			if (error)
			{
				return std::nullopt;
			}
		}

		return CardScript{.identity = std::move(identity), .summons = std::move(summons), .effects = std::move(effects), .usage_restrictions = std::move(usage_restrictions)};
	}
}

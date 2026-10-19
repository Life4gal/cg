#pragma once

#include <optional>
#include <variant>
#include <vector>

#include <cg/dsl/ids.hpp>
#include <cg/dsl/expression.hpp>

namespace cg::dsl
{
	class Statement;

	enum class Conjunction : std::uint8_t
	{
		NONE,
		AND,
		AND_IF_YOU_DO,
		THEN,
		ALSO,
		ALSO_AFTER_THAT,
	};

	// 动作/选择类原语调用
	class ActionCall
	{
	public:
		PrimitiveId primitive;
		// 位置参数
		std::vector<Expression> args;
		// 有返回值者可绑定
		std::optional<BindingId> binding;

		[[nodiscard]] constexpr auto operator==(const ActionCall&) const -> bool = default;
	};

	// 前置条件:不满足 → 中止本效果剩余处理
	class Guard
	{
	public:
		Expression condition;

		[[nodiscard]] constexpr auto operator==(const Guard&) const -> bool = default;
	};

	// 条件分支(条件不满足 → 走else或跳过)
	class If
	{
	public:
		Expression condition;
		std::vector<Statement> then_body;
		std::vector<Statement> else_body;

		[[nodiscard]] constexpr auto operator==(const If&) const -> bool = default;
	};

	// 遍历组(for_each <group> as <binding> { ... })
	class ForEach
	{
	public:
		Expression group;
		BindingId binding;
		std::vector<Statement> body;

		[[nodiscard]] constexpr auto operator==(const ForEach&) const -> bool = default;
	};

	// 语句 = 连接词 + 语句体
	class Statement
	{
	public:
		using body_type = std::variant<ActionCall, Guard, If, ForEach>;

		// 与前一语句的连接关系;首条 = Conjunction::NONE
		Conjunction conjunction;
		body_type body;

		[[nodiscard]] constexpr auto operator==(const Statement&) const -> bool = default;
	};

	class StatementList : public std::vector<Statement> {};
}

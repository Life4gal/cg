#pragma once

#include <functional>
#include <optional>

#include <cg/dsl/effect.hpp>

#include <cg/execution/context.hpp>

namespace cg::dsl
{
	class ActionCall;
	class Expression;
	class ExpressionList;
	class PrimitiveCall;
	class Vocabulary;
}

namespace cg::execution
{
	class Interpreter
	{
	public:
		//

	private:
		std::reference_wrapper<const dsl::Vocabulary> vocabulary_;

		[[nodiscard]] auto evaluate_arguments(const dsl::ExpressionList& args, Context& context) noexcept -> std::optional<ValueList>;
		[[nodiscard]] auto evaluate(const dsl::Expression& expression, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto invoke(const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;

		[[nodiscard]] auto execute_action(const dsl::ActionCall& call, Context& context) noexcept -> bool;

		// ===========================================================
		// 原语
		// ===========================================================

		[[nodiscard]] static auto find_cards(const ValueList& args, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] static auto destroy(const ValueList& args, const Context& context)  noexcept -> bool;

	public:
		explicit Interpreter(const dsl::Vocabulary& vocabulary) noexcept;

		// 执行发动类效果的处理段(结算)
		[[nodiscard]] auto execute(const dsl::ActivatedEffect& effect, Context& context) noexcept -> bool;
	};
}

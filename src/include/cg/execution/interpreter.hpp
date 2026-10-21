#pragma once

#include <functional>
#include <optional>

#include <cg/execution/effect.hpp>
#include <cg/execution/context.hpp>

namespace cg::dsl
{
	// statement
	class ActionCall;

	// expression
	class PrimitiveCall;
	class Expression;
	class ExpressionList;

	// vocabulary
	class PrimitiveDef;
	class Vocabulary;
}

namespace cg::execution
{
	class ScriptRegistry;
	class Chain;

	class Interpreter
	{
	public:
		//

	private:
		std::reference_wrapper<const dsl::Vocabulary> vocabulary_;

		[[nodiscard]] auto evaluate_arguments(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<std::vector<std::optional<Value>>>;
		[[nodiscard]] auto evaluate_list(const dsl::ExpressionList& expressions, Context& context) noexcept -> std::optional<std::vector<Value>>;
		[[nodiscard]] auto evaluate_scoped(const dsl::Expression& expression, state::CardInstanceId element, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto evaluate(const dsl::Expression& expression, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto invoke(const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;

		[[nodiscard]] auto execute_action(const dsl::ActionCall& call, Context& context) noexcept -> bool;
		[[nodiscard]] auto execute_statements(const dsl::StatementList& statements, Context& context) noexcept -> bool;

		// ===========================================================
		// 原语
		// ===========================================================

		[[nodiscard]] auto find_cards(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto exists_matching(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		// pick_cost / pick_target / pick_resolution 共用
		[[nodiscard]] auto pick(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto destroy(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto discard(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;

		// 谓词
		[[nodiscard]] auto is_card_type(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto is_card_subtype(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto is_attribute(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto is_race(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto is_in_zone(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto is_face_up(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;

		// 查询
		[[nodiscard]] auto level(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto attack(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto defense(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto this_card(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto event_card(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto sole_card(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;

		// 动作
		[[nodiscard]] auto draw(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto add_to_hand(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto banish(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto effect_damage(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto recover_life(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto change_position(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto special_summon(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;

		[[nodiscard]] auto filter(Context& context, const dsl::Expression& predicate, const Group& candidates) noexcept -> std::optional<Group>;

	public:
		explicit Interpreter(const dsl::Vocabulary& vocabulary) noexcept;

		// 静态数值修正(场上卡脚本的 StaticEffect;累加匹配属性的add_numeric_property)
		[[nodiscard]] auto static_numeric_bonus(const ScriptRegistry& scripts, state::CardInstanceId card, domain::NumericProperty property, Context& context) noexcept -> Integer::value_type;

		// 触发效果是否匹配当前事件(触发源 + 触发时条件)
		[[nodiscard]] auto matches(const dsl::Trigger& trigger, Context& context) noexcept -> bool;

		// 收集当前事件触发的全部效果(SEGOC顺序:必发优先,回合玩家优先)
		[[nodiscard]] auto collect_triggered(const ScriptRegistry& scripts, const Context& context, domain::Player turn_player) noexcept -> TriggeredEffectList;

		// 激活(condition → cost → target);不执行效果处理
		[[nodiscard]] auto activate(const dsl::ActivatedEffect& effect, Context& context) noexcept -> bool;

		// 结算(仅效果处理段)
		[[nodiscard]] auto resolve(const dsl::ActivatedEffect& effect, Context& context) noexcept -> bool;

		// 由触发列表构建连锁(逐条激活)
		[[nodiscard]] auto build_chain(const TriggeredEffectList& triggered, const Context& context) noexcept -> Chain;

		// 逆序结算整条连锁
		[[nodiscard]] auto resolve(Chain& chain) noexcept -> bool;

		// 执行一张发动类效果:condition → cost → target → resolve
		[[nodiscard]] auto execute(const dsl::ActivatedEffect& effect, Context& context) noexcept -> bool;
	};
}

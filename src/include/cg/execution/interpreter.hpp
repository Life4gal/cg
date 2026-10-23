#pragma once

#include <span>

#include <cg/domain/card.hpp>

#include <cg/execution/value.hpp>
#include <cg/execution/event.hpp>
#include <cg/execution/effect.hpp>
#include <cg/execution/context.hpp>

namespace cg::dsl
{
	// statement
	class ActionCall;
	class Statement;
	class StatementList;

	// expression
	class PrimitiveCall;
	class Expression;
	class ExpressionList;

	// effect
	class StaticModifier;

	// vocabulary
	class PrimitiveDef;
	class Vocabulary;
}

namespace cg::execution
{
	class CardStateResolver;
	class ScriptRegistry;
	class Chain;

	class Interpreter
	{
	public:
		// 只向 CardStateResolver 暴露求值能力
		class Evaluator
		{
			friend class CardStateResolver;

			std::reference_wrapper<Interpreter> interpreter_;

			explicit Evaluator(Interpreter& interpreter) noexcept;

			[[nodiscard]] auto evaluate_list(const dsl::ExpressionList& expressions, Context& context) const noexcept -> std::optional<std::vector<Value>>;
			[[nodiscard]] auto evaluate_scoped(const dsl::Expression& expression, state::CardInstanceId element, Context& context) const noexcept -> std::optional<Value>;
			[[nodiscard]] auto evaluate(const dsl::Expression& expression, Context& context) const noexcept -> std::optional<Value>;
		};

	private:
		std::reference_wrapper<const dsl::Vocabulary> vocabulary_;

		PendingEventList pending_events_;

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

		// 无效化:作用于当前 link 的前一环(它直接响应的那一环)
		[[nodiscard]] auto negate_activation(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto negate_effect(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;

		[[nodiscard]] auto filter(Context& context, const dsl::Expression& predicate, const Group& candidates) noexcept -> std::optional<Group>;

	public:
		explicit Interpreter(const dsl::Vocabulary& vocabulary) noexcept;

		// 取出并清空结算过程中产生的事件
		[[nodiscard]] auto take_events() noexcept -> PendingEventList;

		// 触发效果是否匹配当前事件(触发源 + 触发时条件)
		[[nodiscard]] auto matches(const dsl::Trigger& trigger, Context& context) noexcept -> bool;

		// 收集当前事件触发的全部效果(SEGOC顺序:必发优先,回合玩家优先)
		// - 单窗口重载使用 context.trigger_source / context.event_card
		[[nodiscard]] auto collect_triggered(const ScriptRegistry& scripts, const Context& context, domain::Player turn_player) noexcept -> TriggeredEffectList;

		// 收集同一时点多个事件触发的全部效果
		// - 同一张卡的同一效果被多个事件命中只取首个;同一张卡只取一个效果
		[[nodiscard]] auto collect_triggered(const ScriptRegistry& scripts, const Context& context, std::span<const EventWindow> windows, domain::Player turn_player) noexcept -> TriggeredEffectList;

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

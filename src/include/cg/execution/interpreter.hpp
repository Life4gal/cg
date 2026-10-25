#pragma once

#include <random>
#include <span>

#include <cg/domain/card.hpp>

#include <cg/execution/value.hpp>
#include <cg/execution/event.hpp>
#include <cg/execution/effect.hpp>
#include <cg/execution/chain.hpp>
#include <cg/execution/context.hpp>
#include <cg/execution/script_registry.hpp>
#include <cg/execution/card_state_resolver.hpp>

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
	class Selector;

	class Interpreter
	{
	public:
		//

	private:
		// 引用flow::Duel的state::Game
		std::reference_wrapper<state::Game> game_;
		// 引用flow::Duel的std::mt19937_64
		std::reference_wrapper<std::mt19937_64> random_;
		// 引用flow::Duel的dsl::TypeRegistry
		std::reference_wrapper<const dsl::TypeRegistry> types_;
		// 引用flow::Duel的dsl::Vocabulary
		std::reference_wrapper<const dsl::Vocabulary> vocabulary_;
		// 引用创建flow::Duel时指定的Selector
		std::reference_wrapper<Selector> selector_;

		// 所有脚本
		ScriptRegistry scripts_;
		// 卡牌状态解析器
		CardStateResolver state_resolver_;

		PendingEventList pending_events_;

		[[nodiscard]] auto players_of(const Context& context, EnumMask mask) const noexcept -> std::vector<domain::Player>;
		[[nodiscard]] auto collect(std::span<const domain::Player> players, utility::EnumMask<domain::Zone> zones) const noexcept -> Group;
		[[nodiscard]] auto select(const Context& context, const Group& candidates, std::size_t min, std::size_t max) const noexcept -> std::optional<Group>;

		[[nodiscard]] auto evaluate_arguments(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<std::vector<std::optional<Value>>>;
		[[nodiscard]] auto invoke(const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;

		[[nodiscard]] auto execute_action(const dsl::ActionCall& call, Context& context) noexcept -> bool;
		[[nodiscard]] auto execute_statements(const dsl::StatementList& statements, Context& context) noexcept -> bool;

		// 结算(仅效果处理段);由 resolve(Chain&) / execute 内部使用
		[[nodiscard]] auto resolve(const dsl::ActivatedEffect& effect, Context& context) noexcept -> bool;

		// ===========================================================
		// 原语
		// ===========================================================

		[[nodiscard]] auto find_cards(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto exists_matching(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		// pick_cost / pick_target / pick_resolution 共用
		[[nodiscard]] auto pick(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto destroy(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto discard(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto send_to_graveyard(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto return_to_deck(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto release(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;

		// 谓词
		[[nodiscard]] auto is_code(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto is_card_type(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto is_card_subtype(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto is_attribute(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto is_race(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto is_archetype(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto is_level(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto can_change_to_face_down(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto is_in_zone(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto is_face_up(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;

		// 查询
		[[nodiscard]] auto level(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto attack(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto defense(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto this_card(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto candidate_card(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto chain_exists(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto event_card(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto event_amount(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto event_player(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto sole_card(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;

		// 动作
		[[nodiscard]] auto draw(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto add_to_hand(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto banish(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto effect_damage(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto recover_life(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto pay_life(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto pay_half_life(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto change_position(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto special_summon(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto ritual_summon(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;

		// 无效化:作用于当前 link 的前一环(它直接响应的那一环)
		[[nodiscard]] auto negate_activation(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto negate_activation_and_destroy(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;
		[[nodiscard]] auto negate_effect(const dsl::PrimitiveDef& definition, const dsl::PrimitiveCall& call, Context& context) noexcept -> std::optional<Value>;

	public:
		explicit Interpreter(
			state::Game& game,
			std::mt19937_64& random,
			const dsl::TypeRegistry& types,
			const dsl::Vocabulary& vocabulary,
			Selector& selector
		) noexcept;

		// ================================================================
		// 执行入口(flow 层)
		// ================================================================

		// 从目录加载卡脚本
		auto load_scripts(const std::filesystem::path& directory) noexcept -> bool;

		// 加载卡脚本
		auto load_script(const std::filesystem::path& file) noexcept -> bool;

		// 添加卡脚本
		auto add_script(domain::CardCode code, dsl::CardScript script) noexcept -> bool;

		[[nodiscard]] auto find_script(domain::CardCode code) const noexcept -> const dsl::CardScript*;

		// 全部已加载脚本
		[[nodiscard]] auto scripts() const noexcept -> const ScriptRegistry&;

		// 取出并清空脚本加载错误
		[[nodiscard]] auto take_script_errors() noexcept -> std::vector<std::string>;

		// 获取CardStateResolver
		[[nodiscard]] auto state_resolver() const noexcept -> const CardStateResolver&;

		// 取出并清空结算过程中产生的事件
		[[nodiscard]] auto take_events() noexcept -> PendingEventList;

		[[nodiscard]] auto evaluate_list(const dsl::ExpressionList& expressions, Context& context) noexcept -> std::optional<std::vector<Value>>;

		// flow 侧读取 procedure 的标量条件(如仪式等级)
		[[nodiscard]] auto evaluate(const dsl::Expression& expression, Context& context) noexcept -> std::optional<Value>;

		[[nodiscard]] auto evaluate_scoped(const dsl::Expression& expression, state::CardInstanceId element, Context& context) noexcept -> std::optional<Value>;

		// flow 侧执行 procedure 的 cost/条件语句
		[[nodiscard]] auto execute(const dsl::StatementList& statements, Context& context) noexcept -> bool;

		// 用当前上下文筛选候选卡;`$` 绑定候选(召唤素材等)
		[[nodiscard]] auto filter(const dsl::Expression& predicate, const Group& candidates, Context& context) noexcept -> std::optional<Group>;

		// 触发效果是否匹配当前事件(触发源 + 触发时条件)
		[[nodiscard]] auto matches(const dsl::Trigger& trigger, Context& context) noexcept -> bool;

		// 收集同一时点多个事件触发的全部效果
		// - 同一效果被多个事件命中只取首个;不同效果可以进入同一连锁
		// [[nodiscard]] auto collect_triggered(const EventWindowList& windows, domain::Player turn_player) noexcept -> TriggeredEffectList;
		[[nodiscard]] auto collect_triggered(std::span<const EventWindow> windows, domain::Player turn_player) noexcept -> TriggeredEffectList;

		// 激活(condition → cost → target);不执行效果处理
		[[nodiscard]] auto activate(const dsl::ActivatedEffect& effect, Context& context) noexcept -> bool;

		// 逆序结算整条连锁
		[[nodiscard]] auto resolve(Chain& chain) noexcept -> bool;

		// 构造单个连锁环(激活 + Context);SEGOC/选择/usage 由 Duel 负责
		[[nodiscard]] auto make_link(const TriggeredEffect& entry) noexcept -> std::optional<ChainLink>;

		// ================================================================
		// 测试 / 嵌入支持
		// - 不属于 flow 主路径;当前由 tests/interpreter.cpp 使用
		// ================================================================

		// 单窗口便捷重载:使用 context.event;flow 使用 span 版本
		[[nodiscard]] auto collect_triggered(const EventWindow& window, domain::Player turn_player) noexcept -> TriggeredEffectList;

		// 由触发列表构建连锁(逐条make_link)
		[[nodiscard]] auto build_chain(const TriggeredEffectList& triggered) noexcept -> Chain;

		// 执行一张发动类效果:condition → cost → target → resolve
		[[nodiscard]] auto execute(const dsl::ActivatedEffect& effect, Context& context) noexcept -> bool;
	};
}

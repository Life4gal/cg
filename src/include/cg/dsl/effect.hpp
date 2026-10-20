#pragma once

#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

#include <cg/domain/card.hpp>
#include <cg/domain/effect.hpp>
#include <cg/domain/field.hpp>

#include <cg/dsl/expression.hpp>
#include <cg/dsl/statement.hpp>
#include <cg/dsl/trigger.hpp>

#include <cg/utility/enum.hpp>

namespace cg::dsl
{
	// ===========================================
	// 激活窗口
	// ===========================================

	// 卡自身的发动(魔法/陷阱)
	class CardActivation
	{
	public:
		[[nodiscard]] auto operator==(const CardActivation&) const -> bool = default;
	};

	// 启动効果
	class Ignition
	{
	public:
		[[nodiscard]] auto operator==(const Ignition&) const -> bool = default;
	};

	// 诱发效果(事件/期间触发;含flip等子类)
	class Trigger
	{
	public:
		// 触发源(或关系;至少一个)
		TriggerSourceList sources;
		// WHEN|IF
		TriggerTiming timing;
		// true = 任意 / false = 必发
		bool optional;

		[[nodiscard]] auto operator==(const Trigger&) const -> bool = default;
	};

	// 诱发即时效果
	// 可发动窗口 = 快效果时点系统
	class Quick
	{
	public:
		[[nodiscard]] auto operator==(const Quick&) const -> bool = default;
	};

	class ActivationKind : public std::variant<CardActivation, Ignition, Trigger, Quick> {};

	// ===========================================
	// 咒文速度
	// ===========================================

	// 由卡类型/子类型与激活窗口推导:
	// - `CardActivation` = 卡自身的发动(魔法/陷阱),速度由卡类型决定;
	// - `Ignition` / `Trigger` = 1,`Quick` = 2
	[[nodiscard]] auto speed_of(domain::CardType type, utility::EnumMask<domain::CardSubtype> subtypes, const ActivationKind& activation) noexcept -> domain::EffectSpeed;

	// ===========================================
	// 使用次数限制
	// ===========================================

	enum class UsageScope : std::uint8_t
	{
		// 按回合计算
		TURN,
		// 按决斗计算
		DUEL,
	};

	enum class UsageBasis : std::uint8_t
	{
		// 按卡本身(如该卡一回合一次)
		THIS_CARD,
		// 按卡名(如该卡名一回合一次)
		CARD_NAME,
	};

	enum class RestrictionVerb : std::uint8_t
	{
		// 「use」: 使用,发动被无效也当使用过
		USE,
		// 「activate」: 发动,发动被无效当没有发动过
		ACTIVATE,
		// 「apply」: 适用,效果被无效或者被改动当没有发动过
		APPLY,
	};

	class UsageRestriction
	{
	public:
		// 要么没有限制,有限制也不会超过255次 (同时可以减少填充)
		using size_type = std::uint8_t;
		// 效果索引
		// FIXME(OPT): 将它定义在合适的位置
		using index_type = size_type;

		UsageScope scope;
		UsageBasis basis;
		RestrictionVerb verb;
		// 次数上限(按卡名共享时为卡名合计上限)
		size_type count;
		// 受限效果下标(CardScript::effects[index])
		std::vector<index_type> refs;

		[[nodiscard]] auto operator==(const UsageRestriction&) const -> bool = default;
	};

	class UsageRestrictionList : public std::vector<UsageRestriction> {};

	// ===========================================
	// 发动类效果
	// ===========================================

	class ActivatedEffect
	{
	public:
		// 激活窗口
		ActivationKind activation;
		// 可发动所在区域
		utility::EnumMask<domain::Zone> activation_zones;
		// 发动条件(冒号前;仅发动时需满足)
		std::optional<Expression> condition;
		// 代价(分号前;发动时)
		StatementList cost;
		// 取对象(分号前;发动时)
		StatementList target;
		// 效果处理(结算时)
		StatementList effect;

		[[nodiscard]] auto operator==(const ActivatedEffect&) const -> bool = default;
	};

	// ===========================================
	// 持续类效果
	// ===========================================

	// 静态修饰符(static 类原语调用)
	class StaticModifier
	{
	public:
		PrimitiveId primitive;
		// 位置参数
		ExpressionList args;

		[[nodiscard]] auto operator==(const StaticModifier&) const -> bool = default;
	};

	class StaticModifierList : public std::vector<StaticModifier> {};

	class StaticEffect
	{
	public:
		// 适用对象(对候选卡的谓词)
		Expression apply_to;
		// 生效条件
		std::optional<Expression> condition;
		// 修饰符列表
		StaticModifierList modifiers;

		[[nodiscard]] auto operator==(const StaticEffect& other) const -> bool = default;
	};

	// ===========================================
	// 效果(稳定标识 + 定义体)
	// ===========================================

	class EffectBody : public std::variant<ActivatedEffect, StaticEffect> {};

	class Effect
	{
	public:
		// 卡内稳定标识
		EffectId id;
		// 效果定义体
		EffectBody body;

		[[nodiscard]] auto operator==(const Effect&) const -> bool = default;
	};

	class EffectList : public std::vector<Effect> {};
}

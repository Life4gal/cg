#pragma once

#include <optional>
#include <variant>
#include <vector>

#include <cg/domain/card.hpp>
#include <cg/domain/effect.hpp>
#include <cg/domain/field.hpp>

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
		// 事件型发动时点(可选);空 = 开放状态/连锁响应
		std::optional<TriggerSpec> timing;

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
		TriggerSpec spec;
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

	enum class UsageKind : std::uint8_t
	{
		// 一回合一次(每张卡)
		TURN_PER_CARD,
		// 卡名一回合一次
		TURN_PER_NAME,
		// 决斗一次
		DUEL_PER_NAME,
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
		// 要么没有限制,有限制也不会超过255次(同时可以减少填充)
		using size_type = std::uint8_t;

		// 次数上限
		UsageKind kind;
		size_type count;
		RestrictionVerb verb;

		[[nodiscard]] auto operator==(const UsageRestriction&) const -> bool = default;
	};

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
		// 适用对象(作用域表达式:对候选卡的谓词)
		ScopedExpression apply_to;
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
		// 使用次数限制(可选;没有 = 无限制)
		std::optional<UsageRestriction> usage;
		// 互斥组代表(按卡名、按回合)
		std::optional<EffectId> mutex;

		[[nodiscard]] auto operator==(const Effect&) const -> bool = default;
	};

	class EffectList : public std::vector<Effect> {};
}

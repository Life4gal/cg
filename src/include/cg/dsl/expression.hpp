#pragma once

#include <array>
#include <string_view>
#include <variant>
#include <vector>

#include <cg/domain/player.hpp>

#include <cg/dsl/id.hpp>

namespace cg::dsl
{
	class Expression;

	class ExpressionList : public std::vector<Expression> {};

	// 字面量(整数/布尔/符号常量)
	class Literal
	{
	public:
		using number_type = std::int32_t;

		// 符号常量(取值域确定;校验期按type校验)
		// - 闭枚举:value = 枚举序数(经TypeRegistry解析);
		// - 开放名域(系列/字段等):value = ArchetypeId
		class Symbol
		{
		public:
			TypeId type;
			std::uint64_t value;
		};

		using value_type = std::variant<number_type, bool, Symbol>;

		value_type value;

		[[nodiscard]] constexpr auto operator==(const Literal& other) const -> bool = default;
	};

	// 枚举集合常量(位掩码;如区域集合)
	// - 元素类型由type给出(如zone);掩码位 = 元素枚举序数(utility::EnumMask<Enum>::value())
	class EnumSetConstant
	{
	public:
		using value_type = std::uint64_t;

		TypeId type;
		value_type mask;

		[[nodiscard]] constexpr auto operator==(const EnumSetConstant&) const -> bool = default;
	};

	// 绑定引用
	class BindingRef
	{
	public:
		BindingId id;

		[[nodiscard]] constexpr auto operator==(const BindingRef&) const -> bool = default;
	};

	// 隐式候选
	class CandidateRef
	{
	public:
		constexpr static std::string_view ref = "$";

		[[nodiscard]] constexpr auto operator==(const CandidateRef&) const -> bool = default;
	};

	// 原语调用(谓词/查询类;位置参数)
	class PrimitiveCall
	{
	public:
		PrimitiveId primitive;
		ExpressionList args;

		[[nodiscard]] constexpr auto operator==(const PrimitiveCall&) const -> bool = default;
	};

	enum class BooleanOperator : std::uint8_t
	{
		AND = 0,
		OR = 1,
		NOT = 2,
	};

	constexpr std::array<std::string_view, 3> boolean_operator_names
	{
			"and",
			"or",
			"not",
	};

	// 布尔算子
	class BooleanOperation
	{
	public:
		BooleanOperator op;
		ExpressionList operands;

		[[nodiscard]] constexpr auto operator==(const BooleanOperation&) const -> bool = default;
	};

	enum class CompareOperator : std::uint8_t
	{
		EQUAL,
		NOT_EQUAL,
		LESS_THAN,
		LESS_EQUAL,
		GREATER_THAN,
		GREATER_EQUAL,
	};

	constexpr std::array<std::string_view, 6> compare_operator_names
	{
			"eq",
			"ne",
			"lt",
			"le",
			"gt",
			"ge",
	};

	// 比较算子
	class CompareOperation
	{
	public:
		CompareOperator op;
		// 恰 2 个
		ExpressionList operands;

		[[nodiscard]] constexpr auto operator==(const CompareOperation&) const -> bool = default;
	};

	enum class ArithmeticOperator : std::uint8_t
	{
		ADD,
		SUB,
		MUL,
		DIV,
		MIN,
		MAX,
	};

	constexpr std::array<std::string_view, 6> arithmetic_operator_names
	{
			"add",
			"sub",
			"mul",
			"div",
			"min",
			"max",
	};

	// 算术算子
	class ArithmeticOperation
	{
	public:
		ArithmeticOperator op;
		// 恰 2 个
		ExpressionList operands;

		[[nodiscard]] constexpr auto operator==(const ArithmeticOperation&) const -> bool = default;
	};

	// 任意表达式
	class Expression : public std::variant<
				Literal,
				EnumSetConstant,
				BindingRef,
				CandidateRef,
				PrimitiveCall,
				BooleanOperation,
				CompareOperation,
				ArithmeticOperation
			> {};
}

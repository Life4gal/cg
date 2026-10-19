#pragma once

#include <variant>
#include <vector>

#include <cg/domain/player.hpp>

#include <cg/dsl/ids.hpp>

namespace cg::dsl
{
	class Expression;

	// 字面量(整数/布尔/符号常量)
	class Literal
	{
	public:
		// 符号常量(取值域确定;校验期按type校验)
		// - 闭枚举:value = 枚举序数(经TypeRegistry解析);
		// - 开放名域(系列/字段等):value = ArchetypeId
		class Symbol
		{
		public:
			TypeId type;
			std::uint64_t value;
		};

		using value_type = std::variant<std::int32_t, bool, Symbol>;

		value_type value;

		[[nodiscard]] constexpr auto operator==(const Literal& other) const -> bool = default;
	};

	// 枚举集合常量(位掩码;如区域集合)
	// - 元素类型由type给出(如zone);掩码位 = 元素枚举序数(utility::EnumMask<Enum>::value())
	class EnumSetConstant
	{
	public:
		TypeId type;
		std::uint64_t mask;

		[[nodiscard]] constexpr auto operator==(const EnumSetConstant&) const -> bool = default;
	};

	class PlayerRef
	{
	public:
		domain::PlayerSelector selector;

		[[nodiscard]] constexpr auto operator==(const PlayerRef&) const -> bool = default;
	};

	class BindingRef
	{
	public:
		BindingId id;

		[[nodiscard]] constexpr auto operator==(const BindingRef&) const -> bool = default;
	};

	class CandidateRef
	{
	public:
		[[nodiscard]] constexpr auto operator==(const CandidateRef&) const -> bool = default;
	};

	// 原语调用(谓词/查询类;位置参数)
	class PrimitiveCall
	{
	public:
		PrimitiveId primitive;
		std::vector<Expression> args;

		[[nodiscard]] constexpr auto operator==(const PrimitiveCall&) const -> bool = default;
	};

	enum class BoolOperator : std::uint8_t
	{
		AND,
		OR,
		NOT,
	};

	class BooleanOperation
	{
	public:
		BoolOperator op;
		std::vector<Expression> operands;

		[[nodiscard]] constexpr auto operator==(const BooleanOperation&) const -> bool = default;
	};

	enum class CompareOperator : std::uint8_t
	{
		EQUAL,
		NOT_EQUAL,
		LESS,
		LESS_EQUAL,
		GREATER,
		GREATER_EQUAL,
	};

	class CompareOperation
	{
	public:
		CompareOperator op;
		// 恰 2 个
		std::vector<Expression> operands;

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

	class ArithmeticOperation
	{
	public:
		ArithmeticOperator op;
		// 恰 2 个
		std::vector<Expression> operands;

		[[nodiscard]] constexpr auto operator==(const ArithmeticOperation&) const -> bool = default;
	};

	// 任意表达式
	class Expression : public std::variant<
				Literal,
				EnumSetConstant,
				PlayerRef,
				BindingRef,
				CandidateRef,
				PrimitiveCall,
				BooleanOperation,
				CompareOperation,
				ArithmeticOperation
			> {};
}

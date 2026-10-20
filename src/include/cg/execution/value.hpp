#pragma once

#include <variant>
#include <vector>

#include <cg/state/id.hpp>

namespace cg::execution
{
	class Void {};

	class Boolean
	{
	public:
		// dsl::Literal::boolean_type
		using value_type = bool;

		value_type value;
	};

	class Integer
	{
	public:
		// dsl::Literal::number_type
		// dsl::Literal::Symbol
		using value_type = std::int64_t;

		value_type value;
	};

	class Card
	{
	public:
		using value_type = state::CardInstanceId;

		value_type value;
	};

	class Group
	{
	public:
		using value_type = std::vector<state::CardInstanceId>;

		value_type value;
	};

	class EnumMask
	{
	public:
		// dsl::TypeRegistry::symbol_underlying_type
		using value_type = std::uint64_t;

		value_type value;
	};

	// 表达式求值结果
	class Value : public std::variant<
				// dsl::ValueClass::VOID
				Void,
				// dsl::ValueClass::BOOL
				Boolean,
				// dsl::ValueClass::INT
				Integer,
				// dsl::ValueClass::CARD
				Card,
				// dsl::ValueClass::GROUP
				Group,
				// dsl::ValueClass::ENUM_SET
				EnumMask
			> {};

	class ValueList : public std::vector<Value> {};
}

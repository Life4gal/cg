#pragma once

#include <string>
#include <vector>
#include <variant>

#include <cg/dsl/id.hpp>

#include <cg/utility/enum.hpp>

namespace cg::dsl
{
	class TypeRegistry;

	enum class Phase : std::uint8_t
	{
		// 检查是否满足条件
		REQUIRE,
		// 支付代价
		COST,
		// 选择对象
		TARGET,
		// 解析/处理
		RESOLVE,
		// 静态修饰符(非连锁段;MODEL 决策 3.3)
		STATIC,
	};

	enum class PrimitiveKind : std::uint8_t
	{
		// 条件过滤 -- 如检查属性/种族
		PREDICATE,
		// 查询状态 -- 如查询当前攻击力
		QUERY,
		// 选择 -- 如选择一个区域,选择一张卡牌
		SELECT,
		// 行为 -- 如进行特殊召唤
		ACTION,
		// 控制 -- 执行类控制原语(If/ForEach/Guard/连词为语法,不占此类)
		CONTROL,
		// 静态修饰符(非连锁段)
		STATIC,
	};

	class ParamDef
	{
	public:
		// 普通值参数:一个表达式
		class Value
		{
		public:
			TypeId type;

			[[nodiscard]] constexpr auto operator==(const Value&) const -> bool = default;
		};

		// 作用域参数:表达式在一个隐式元素(`$`)的作用域内求值
		// - element = `$`的类型
		// - type    = 表达式类型(过滤/条件 = bool;投影/映射 = 任意结果类型)
		class Scoped
		{
		public:
			TypeId element;
			TypeId type;

			[[nodiscard]] constexpr auto operator==(const Scoped&) const -> bool = default;
		};

		// 参数形态
		class Kind : public std::variant<Value, Scoped> {};

		// 参数名(仅诊断/生成视图;调用处为位置参数)
		std::string name;
		// 参数形态(普通值/作用域内求值)
		Kind kind;
		// 是否必须提供
		bool required;

		[[nodiscard]] constexpr auto operator==(const ParamDef&) const -> bool = default;
	};

	class PrimitiveDef
	{
	public:
		PrimitiveId id;
		// 规范名(表示层按此名解析;如"destroy")
		std::string name;
		PrimitiveKind kind;
		// 允许出现的阶段
		utility::EnumMask<Phase> phases;
		// 参数定义
		std::vector<ParamDef> params;
		// 返回值类型(无返回值 = ValueClass::VOID类型id)
		TypeId returns;

		// 说明(自由文本)
		std::string doc;

		[[nodiscard]] auto operator==(const PrimitiveDef&) const -> bool = default;
	};

	// 词汇表 = 有序条目集合;构建期校验 name/id 唯一性与签名一致性。
	class Vocabulary
	{
	public:
		using primitives_type = std::vector<PrimitiveDef>;
		using size_type = primitives_type::size_type;

	private:
		primitives_type primitives_;

		[[nodiscard]] auto generate_id() const noexcept -> PrimitiveId;

		auto do_register(
			std::string name,
			PrimitiveKind kind,
			utility::EnumMask<Phase> phases,
			std::vector<ParamDef> params,
			TypeId returns,
			std::string doc
		) noexcept -> void;

	public:
		explicit Vocabulary(const TypeRegistry& types) noexcept;

		// 词表校验:name/id 唯一、类型已注册
		// 返回错误列表(空 = 通过)。
		[[nodiscard]] auto validate(const TypeRegistry& types) const noexcept -> std::vector<std::string>;

		[[nodiscard]] auto find(PrimitiveId id) const noexcept -> const PrimitiveDef*;
		[[nodiscard]] auto find(std::string_view name) const noexcept -> const PrimitiveDef*;

		[[nodiscard]] auto empty() const noexcept -> bool;
		[[nodiscard]] auto size() const noexcept -> size_type;
	};
}

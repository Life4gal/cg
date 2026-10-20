#pragma once

#include <optional>
#include <variant>
#include <vector>

#include <cg/domain/card.hpp>
#include <cg/domain/field.hpp>

#include <cg/dsl/expression.hpp>
#include <cg/dsl/statement.hpp>

#include <cg/utility/enum.hpp>

namespace cg::dsl
{
	// 素材说明
	class MaterialSpec
	{
	public:
		using selector_type = std::variant<
			// 必须是指定卡作为素材
			domain::CardCode,
			// 满足条件即可
			Expression
		>;

		using size_type = domain::DeckLocation::size_type;

		selector_type selector;
		size_type count;

		[[nodiscard]] constexpr auto operator==(const MaterialSpec&) const -> bool = default;
	};

	class MaterialSpecList : public std::vector<MaterialSpec> {};

	// 融合
	class FusionProcedure
	{
	public:
		MaterialSpecList materials;

		[[nodiscard]] constexpr auto operator==(const FusionProcedure&) const -> bool = default;
	};

	// 同调
	class SynchroProcedure
	{
	public:
		// 调整
		Expression tuner;
		// 非调整
		Expression non_tuner;

		[[nodiscard]] constexpr auto operator==(const SynchroProcedure&) const -> bool = default;
	};

	// 超量
	class XyzProcedure
	{
	public:
		using size_type = MaterialSpec::size_type;

		Expression material;
		size_type count;

		[[nodiscard]] constexpr auto operator==(const XyzProcedure&) const -> bool = default;
	};

	// 连接
	class LinkProcedure
	{
	public:
		using size_type = MaterialSpec::size_type;

		Expression material;
		size_type count;

		[[nodiscard]] constexpr auto operator==(const LinkProcedure&) const -> bool = default;
	};

	// 仪式(等级/素材,随原语与语料细化)
	class RitualProcedure
	{
	public:
		Expression material;
		Expression level;

		[[nodiscard]] constexpr auto operator==(const RitualProcedure&) const -> bool = default;
	};

	// 卡自身手续特招,不需要发动效果,不开连锁
	class SelfSpecialProcedure
	{
	public:
		std::optional<Expression> condition;
		StatementList cost;
		utility::EnumMask<domain::Zone> source_zones;

		[[nodiscard]] constexpr auto operator==(const SelfSpecialProcedure&) const -> bool = default;
	};

	class SummonProcedure : public std::variant<
				FusionProcedure,
				SynchroProcedure,
				XyzProcedure,
				LinkProcedure,
				RitualProcedure,
				SelfSpecialProcedure
			> {};

	class SummonProcedureList : public std::vector<SummonProcedure> {};
}

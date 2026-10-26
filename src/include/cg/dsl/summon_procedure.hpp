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

	// 规则特招(效果外文本):卡自身的手续,不需要发动、不入连锁,也不属于效果
	// 其条件/代价/来源区域以及文本明确写出的限制(如once_per_turn)都属于召唤条件的一部分
	// 例如:青眼亚白龙[38517737]的效果外文本`这张卡不能通常召唤.向对手展示手牌的1只「青眼の白龍／青眼白龙」的场合可以特殊召唤.「青眼の亜白龍／青眼亚白龙」的以这个方法所作的特殊召唤1回合只能1次.`
	class RuleSpecialProcedure
	{
	public:
		std::optional<Expression> condition;
		StatementList cost;
		utility::EnumMask<domain::Zone> source_zones;
		// 同一卡名每回合只能通过此手续成功特殊召唤一次(显式文本限制)
		bool once_per_turn;

		[[nodiscard]] constexpr auto operator==(const RuleSpecialProcedure&) const -> bool = default;
	};

	class SummonProcedure : public std::variant<
				FusionProcedure,
				SynchroProcedure,
				XyzProcedure,
				LinkProcedure,
				RitualProcedure,
				RuleSpecialProcedure
			> {};

	class SummonProcedureList : public std::vector<SummonProcedure> {};
}

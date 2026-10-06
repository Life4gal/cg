#pragma once

#include <variant>
#include <vector>
#include <optional>

#include <domain/summon.hpp>
#include <domain/player.hpp>
#include <domain/zone.hpp>

#include <engine/state/id.hpp>
#include <engine/state/handle.hpp>

namespace cg::engine
{
	class NormalSummonSpec
	{
	public:
		[[nodiscard]] constexpr auto operator==(const NormalSummonSpec& other) const noexcept -> bool = default;
	};

	class AdvanceSummonSpec
	{
	public:
		[[nodiscard]] constexpr auto operator==(const AdvanceSummonSpec& other) const noexcept -> bool = default;
	};

	class RitualSummonSpec
	{
	public:
		domain::RitualSummonLevel level;

		[[nodiscard]] constexpr auto operator==(const RitualSummonSpec& other) const noexcept -> bool = default;
	};

	class FusionSummonSpec
	{
	public:
		using size_type = domain::DeckLocation::size_type;

		class Material
		{
		public:
			class Any {};

			using selector_type = std::variant<
				// 任意素材
				Any,
				// 必须是指定卡牌
				CardCode,
				// 必须满足指定条件
				ExpressionHandle
			>;

			selector_type selector;
			// 需要的数量
			size_type count;
			// 种族/属性?
		};

		std::vector<Material> materials;

		[[nodiscard]] constexpr auto operator==(const FusionSummonSpec& other) const noexcept -> bool = default;
	};

	class SynchroSummonSpec
	{
	public:
		using size_type = domain::DeckLocation::size_type;

		ExpressionHandle tuner;
		ExpressionHandle non_tuner;
		// 最少数量
		std::optional<size_type> min;

		[[nodiscard]] constexpr auto operator==(const SynchroSummonSpec& other) const noexcept -> bool = default;
	};

	class XyzSummonSpec
	{
	public:
		using size_type = domain::DeckLocation::size_type;

		ExpressionHandle filter;
		// 数量
		std::optional<size_type> count;

		[[nodiscard]] constexpr auto operator==(const XyzSummonSpec& other) const noexcept -> bool = default;
	};

	class PendulumSummonSpec
	{
	public:
		[[nodiscard]] constexpr auto operator==(const PendulumSummonSpec& other) const noexcept -> bool = default;
	};

	class LinkSummonSpec
	{
	public:
		using size_type = domain::DeckLocation::size_type;

		ExpressionHandle filter;
		// 最少数量
		std::optional<size_type> min;

		[[nodiscard]] constexpr auto operator==(const LinkSummonSpec& other) const noexcept -> bool = default;
	};

	class SpecialSummonSpec
	{
	public:
		// 条件
		ExpressionHandle condition;
		// 代价
		ExpressionHandle cost;
		// 控制者(可被特招到对方场上)
		domain::RelativePlayer controller;
		// 来源
		domain::Zone source;
		// 落点限制
		ExpressionHandle zone;

		[[nodiscard]] constexpr auto operator==(const SpecialSummonSpec& other) const noexcept -> bool = default;
	};

	class SummonSpec : public std::variant<
				NormalSummonSpec,
				AdvanceSummonSpec,
				RitualSummonSpec,
				FusionSummonSpec,
				SynchroSummonSpec,
				XyzSummonSpec,
				PendulumSummonSpec,
				LinkSummonSpec,
				SpecialSummonSpec
			>
	{
	public:
		using base = variant;

		[[nodiscard]] constexpr auto method() const noexcept -> domain::SummonMethod
		{
			return static_cast<domain::SummonMethod>(index());
		}
	};

	static_assert(std::is_same_v<std::variant_alternative_t<std::to_underlying(domain::SummonMethod::NORMAL), SummonSpec::base>, NormalSummonSpec>);
	static_assert(std::is_same_v<std::variant_alternative_t<std::to_underlying(domain::SummonMethod::ADVANCE), SummonSpec::base>, AdvanceSummonSpec>);
	static_assert(std::is_same_v<std::variant_alternative_t<std::to_underlying(domain::SummonMethod::RITUAL), SummonSpec::base>, RitualSummonSpec>);
	static_assert(std::is_same_v<std::variant_alternative_t<std::to_underlying(domain::SummonMethod::FUSION), SummonSpec::base>, FusionSummonSpec>);
	static_assert(std::is_same_v<std::variant_alternative_t<std::to_underlying(domain::SummonMethod::SYNCHRO), SummonSpec::base>, SynchroSummonSpec>);
	static_assert(std::is_same_v<std::variant_alternative_t<std::to_underlying(domain::SummonMethod::XYZ), SummonSpec::base>, XyzSummonSpec>);
	static_assert(std::is_same_v<std::variant_alternative_t<std::to_underlying(domain::SummonMethod::PENDULUM), SummonSpec::base>, PendulumSummonSpec>);
	static_assert(std::is_same_v<std::variant_alternative_t<std::to_underlying(domain::SummonMethod::LINK), SummonSpec::base>, LinkSummonSpec>);
	static_assert(std::is_same_v<std::variant_alternative_t<std::to_underlying(domain::SummonMethod::SPECIAL), SummonSpec::base>, SpecialSummonSpec>);

	class SummonInfo
	{
	public:
		SummonSpec spec;
		ExpressionHandle material_check;

		[[nodiscard]] constexpr auto operator==(const SummonInfo& other) const noexcept -> bool = default;
	};
}

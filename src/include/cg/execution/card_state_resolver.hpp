#pragma once

#include <functional>
#include <optional>
#include <vector>

#include <cg/domain/card.hpp>
#include <cg/domain/effect.hpp>

#include <cg/execution/value.hpp>

#include <cg/utility/enum.hpp>

namespace cg::dsl
{
	class StaticModifier;
	class Vocabulary;
}

namespace cg::state
{
	class CardInstance;
	class Game;
}

namespace cg::execution
{
	class Context;
	class Interpreter;
	class ScriptRegistry;

	// 有效值:印刷值 + 场上持续效果修正
	class CardStateResolver
	{
	public:
		//

	private:
		std::reference_wrapper<state::Game> game_;
		std::reference_wrapper<const ScriptRegistry> scripts_;
		std::reference_wrapper<const dsl::Vocabulary> vocabulary_;
		std::reference_wrapper<Interpreter> interpreter_;

		// 收集作用于目标卡的静态修饰(已通过 apply_to / condition 筛选)
		[[nodiscard]] auto collect_static_modifiers(
			const state::CardInstance& card,
			Context& context
		) const noexcept -> std::vector<std::reference_wrapper<const dsl::StaticModifier>>;

		[[nodiscard]] auto static_code(const state::CardInstance& card, Context& context) const noexcept -> std::optional<domain::CardCode>;
		[[nodiscard]] auto static_card_types(const state::CardInstance& card, Context& context) const noexcept -> utility::EnumMask<domain::CardType>;
		[[nodiscard]] auto static_card_subtypes(const state::CardInstance& card, Context& context) const noexcept -> utility::EnumMask<domain::CardSubtype>;
		[[nodiscard]] auto static_attributes(const state::CardInstance& card, Context& context) const noexcept -> utility::EnumMask<domain::Attribute>;
		[[nodiscard]] auto static_level_bonus(const state::CardInstance& card, Context& context) const noexcept -> Integer::value_type;
		[[nodiscard]] auto static_numeric_bonus(const state::CardInstance& card, domain::NumericProperty property, Context& context) const noexcept -> Integer::value_type;
		[[nodiscard]] auto static_protections(const state::CardInstance& card, Context& context) const noexcept -> utility::EnumMask<domain::Protection>;

	public:
		CardStateResolver(
			state::Game& game,
			const ScriptRegistry& scripts,
			const dsl::Vocabulary& vocabulary,
			Interpreter& interpreter
		) noexcept;

		[[nodiscard]] auto effective_code(const state::CardInstance& card) const noexcept -> domain::CardCode;

		[[nodiscard]] auto is_card_type(const state::CardInstance& card, domain::CardType type) const noexcept -> bool;
		[[nodiscard]] auto is_card_subtype(const state::CardInstance& card, domain::CardSubtype subtype) const noexcept -> bool;
		[[nodiscard]] auto is_card_subtype(const state::CardInstance& card, utility::EnumMask<domain::CardSubtype> subtypes) const noexcept -> bool;

		[[nodiscard]] auto attack(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>;
		[[nodiscard]] auto defense(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>;

		[[nodiscard]] auto attributes(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::Attribute>;
		[[nodiscard]] auto race(const state::CardInstance& card) const noexcept -> domain::Race;
		[[nodiscard]] auto level(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::LevelRank>;

		[[nodiscard]] auto protections(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::Protection>;
	};
}

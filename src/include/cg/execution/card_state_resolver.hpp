#pragma once

#include <functional>

#include <cg/domain/card.hpp>
#include <cg/domain/effect.hpp>

#include <cg/utility/enum.hpp>

namespace cg::state
{
	class CardInstance;
	class Game;
}

namespace cg::execution
{
	class Interpreter;
	class ScriptRegistry;

	// 有效值查询:印刷值 + 持续效果修正
	class CardStateResolver
	{
	public:
		//

	private:
		std::reference_wrapper<state::Game> game_;
		std::reference_wrapper<const ScriptRegistry> scripts_;
		std::reference_wrapper<Interpreter> interpreter_;

	public:
		CardStateResolver(state::Game& game, const ScriptRegistry& scripts, Interpreter& interpreter) noexcept;

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

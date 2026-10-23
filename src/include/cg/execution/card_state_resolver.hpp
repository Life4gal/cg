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
	class Selector;

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
		std::reference_wrapper<Selector> selector_;

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
			Interpreter& interpreter,
			Selector& selector
		) noexcept;

		// 获取卡密 = 印刷值 + 静态修饰
		[[nodiscard]] auto effective_code(const state::CardInstance& card) const noexcept -> domain::CardCode;

		// 获取类型 = 印刷值 + 静态修饰
		[[nodiscard]] auto card_types(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::CardType>;

		// 获取子类型 = 印刷值 + 静态修饰
		[[nodiscard]] auto card_subtypes(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::CardSubtype>;

		// 检查是否包含指定类型 = 印刷值 + 静态修饰
		[[nodiscard]] auto is_card_type(const state::CardInstance& card, domain::CardType type) const noexcept -> bool;

		// 检查是否包含指定子类型 = 印刷值 + 静态修饰
		[[nodiscard]] auto is_card_subtype(const state::CardInstance& card, domain::CardSubtype subtype) const noexcept -> bool;

		// 检查是否包含指定子类型 = 印刷值 + 静态修饰
		[[nodiscard]] auto is_card_subtype(const state::CardInstance& card, utility::EnumMask<domain::CardSubtype> subtypes) const noexcept -> bool;

		// 获取攻击力 = 印刷值 + 静态修饰
		[[nodiscard]] auto attack(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>;

		// 获取守备力 = 印刷值 + 静态修饰
		[[nodiscard]] auto defense(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::AttackDefense>;

		// 获取属性 = 印刷值 + 静态修饰
		[[nodiscard]] auto attributes(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::Attribute>;

		// 获取种族
		[[nodiscard]] auto race(const state::CardInstance& card) const noexcept -> domain::Race;

		// 获取等级/阶级 = 印刷值 + 静态修饰
		[[nodiscard]] auto level(const state::CardInstance& card) const noexcept -> std::underlying_type_t<domain::LevelRank>;

		// 获取防护能力 = 印刷值 + 静态修饰
		[[nodiscard]] auto protections(const state::CardInstance& card) const noexcept -> utility::EnumMask<domain::Protection>;
	};
}

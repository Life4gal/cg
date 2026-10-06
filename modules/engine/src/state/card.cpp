#include <engine/state/card.hpp>

#include <engine/state/state.hpp>

namespace cg::engine
{
	auto CardDefinition::code_of() const noexcept -> CardCode
	{
		return code;
	}

	auto CardDefinition::is_monster_card() const noexcept -> bool
	{
		return kind == domain::CardType::MONSTER;
	}

	auto CardDefinition::is_spell_card() const noexcept -> bool
	{
		return kind == domain::CardType::SPELL;
	}

	auto CardDefinition::is_trap_card() const noexcept -> bool
	{
		return kind == domain::CardType::TRAP;
	}

	auto CardDefinition::has_subtype(const domain::CardType subtype) const noexcept -> bool
	{
		return subtypes & subtype;
	}

	auto CardDefinition::is_extra_deck_monster_card() const noexcept -> bool
	{
		if (!is_monster_card())
		{
			return false;
		}

		return has_subtype(domain::CardType::FUSION) || has_subtype(domain::CardType::SYNCHRO) || has_subtype(domain::CardType::XYZ) || has_subtype(domain::CardType::LINK);
	}

	auto CardInstance::is_monster_card() const noexcept -> bool
	{
		// 如果当前形态是怪兽
		if (form.monster.has_value())
		{
			return true;
		}

		// 否则基于定义
		return definition.get().is_monster_card();
	}

	auto CardInstance::is_spell_card() const noexcept -> bool
	{
		// 如果当前形态是永续魔法
		if (form.continuous_spell)
		{
			return true;
		}

		// 否则基于定义
		return definition.get().is_spell_card();
	}

	auto CardInstance::is_trap_card() const noexcept -> bool
	{
		return definition.get().is_trap_card();
	}

	auto CardInstance::code_of() const noexcept -> CardCode
	{
		// TODO
		return definition.get().code_of();
	}

	auto CardInstance::owner_of() const noexcept -> domain::Player
	{
		return owner;
	}

	auto CardInstance::controller_of() const noexcept -> domain::Player
	{
		return controller;
	}

	auto CardInstance::position_of() const noexcept -> domain::Position
	{
		return position;
	}

	auto code_of(const GameState& state, const CardInstanceId card) noexcept -> std::optional<CardCode>
	{
		const auto* instance = find_instance(state, card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		return instance->code_of();
	}

	auto owner_of(const GameState& state, const CardInstanceId card) noexcept -> std::optional<domain::Player>
	{
		const auto* instance = find_instance(state, card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		return instance->owner_of();
	}

	auto controller_of(const GameState& state, const CardInstanceId card) noexcept -> std::optional<domain::Player>
	{
		const auto* instance = find_instance(state, card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		return instance->controller_of();
	}

	auto position_of(const GameState& state, const CardInstanceId card) noexcept -> std::optional<domain::Position>
	{
		const auto* instance = find_instance(state, card);
		if (instance == nullptr)
		{
			return std::nullopt;
		}

		return instance->position_of();
	}
}

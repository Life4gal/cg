#include <cg/dsl/types.hpp>

#include <algorithm>
#include <ranges>

#include <cg/domain/card.hpp>
#include <cg/domain/field.hpp>
#include <cg/domain/player.hpp>
#include <cg/domain/summon.hpp>
#include <cg/domain/phase.hpp>
#include <cg/domain/reason.hpp>
#include <cg/domain/event.hpp>
#include <cg/dsl/archetype.hpp>

namespace cg::dsl
{
	auto TypeRegistry::generate_id() const noexcept -> TypeId
	{
		const auto id = types_.size() + 1;
		return TypeId{static_cast<TypeId::value_type>(id)};
	}

	auto TypeRegistry::do_register(
		const std::string_view name,
		const ValueClass value_class,
		const SymbolResolver symbol_resolver
	) noexcept -> void
	{
		const auto id = generate_id();
		types_.emplace_back(id, name, value_class, symbol_resolver);
	}

	TypeRegistry::TypeRegistry() noexcept
	{
		// ====================
		// 基础类别 -- 全小写,不解析
		// ====================

		do_register("void", ValueClass::VOID, none_symbol_resolver);
		do_register("bool", ValueClass::BOOL, none_symbol_resolver);
		do_register("int", ValueClass::INT, none_symbol_resolver);
		do_register("card", ValueClass::CARD, none_symbol_resolver);
		do_register("group", ValueClass::GROUP, none_symbol_resolver);

		// ====================
		// 开放名域
		// ====================

		// 系列/字段
		do_register(
			"archetype",
			ValueClass::SYMBOL,
			make_resolver(
				[](const std::string_view name) noexcept -> std::optional<symbol_underlying_type>
				{
					const auto id = ArchetypeRegistry::id_of(name);
					return id.raw();
				},
				[](const symbol_underlying_type symbol) noexcept -> std::string_view
				{
					const auto v = static_cast<ArchetypeId::value_type>(symbol);
					const auto id = ArchetypeId{v};
					return ArchetypeRegistry::name_of(id);
				}
			)
		);

		// ====================
		// 枚举值
		// ====================

		// domain/player.hpp
		do_register("player", ValueClass::ENUM, make_resolver<domain::PlayerSelector>());
		do_register("players", ValueClass::ENUM_SET, make_resolver<domain::PlayerSelector>());
		// domain/card.hpp
		do_register("card_type", ValueClass::ENUM, make_resolver<domain::CardType>());
		do_register("card_subtype", ValueClass::ENUM, make_resolver<domain::CardSubtype>());
		do_register("attribute", ValueClass::ENUM, make_resolver<domain::Attribute>());
		do_register("race", ValueClass::ENUM, make_resolver<domain::Race>());
		do_register("link_marker", ValueClass::ENUM, make_resolver<domain::LinkMarker>());
		do_register("numeric_property", ValueClass::ENUM, make_resolver<domain::NumericProperty>());
		// domain/field.hpp
		do_register("position", ValueClass::ENUM, make_resolver<domain::Position>());
		do_register("zone", ValueClass::ENUM, make_resolver<domain::Zone>());
		do_register("zones", ValueClass::ENUM_SET, make_resolver<domain::Zone>());
		// domain/summon.hpp
		do_register("summon_method", ValueClass::ENUM, make_resolver<domain::SummonMethod>());
		// domain/phase.hpp
		do_register("phase", ValueClass::ENUM, make_resolver<domain::Phase>());
		do_register("battle_step", ValueClass::ENUM, make_resolver<domain::BattleStep>());
		do_register("damage_step", ValueClass::ENUM, make_resolver<domain::DamageStep>());
		// domain/reason.hpp
		do_register("reason", ValueClass::ENUM, make_resolver<domain::Reason>());
		// domain/event.hpp
		do_register("event_code", ValueClass::ENUM, make_resolver<domain::EventCode>());
	}

	auto TypeRegistry::contains(const TypeId id) const noexcept -> bool
	{
		return std::ranges::contains(types_, id, &Def::id);
	}

	auto TypeRegistry::contains(const std::string_view name) const noexcept -> bool
	{
		return std::ranges::contains(types_, name, &Def::name);
	}

	auto TypeRegistry::find(const TypeId id) const noexcept -> const Def*
	{
		const auto it = std::ranges::find(types_, id, &Def::id);
		if (it == types_.end())
		{
			return nullptr;
		}

		return &*it;
	}

	auto TypeRegistry::find(const std::string_view name) const noexcept -> const Def*
	{
		const auto it = std::ranges::find(types_, name, &Def::name);
		if (it == types_.end())
		{
			return nullptr;
		}

		return &*it;
	}
}

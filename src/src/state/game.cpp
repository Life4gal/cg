#include <cg/state/game.hpp>

#include <algorithm>

#include <libassert/assert.hpp>

namespace cg::state
{
	auto Game::has_definition(const domain::CardCode code) const noexcept -> bool
	{
		return card_definitions.contains(code);
	}

	auto Game::find_definition(const domain::CardCode code) const noexcept -> const domain::CardDefinition*
	{
		if (const auto it = card_definitions.find(code);
			it != card_definitions.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	auto Game::has_instance(const CardInstanceId card) const noexcept -> bool
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return std::ranges::contains(card_instances, card, &CardInstance::id);
	}

	auto Game::find_instance(const CardInstanceId card) const noexcept -> const CardInstance*
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		if (const auto it = std::ranges::find(card_instances, card, &CardInstance::id);
			it != card_instances.end())
		{
			return &*it;
		}

		return nullptr;
	}

	auto Game::owner(const CardInstanceId card) const noexcept -> std::optional<domain::Player>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return instance_invoke(card, &CardInstance::owner);
	}

	auto Game::controller(const CardInstanceId card) const noexcept -> std::optional<domain::Player>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return instance_invoke(card, &CardInstance::controller);
	}

	auto Game::placement(const CardInstanceId card) const noexcept -> std::optional<domain::Placement>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return instance_invoke(card, &CardInstance::placement);
	}

	auto Game::code(const CardInstanceId card) const noexcept -> std::optional<domain::CardCode>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return instance_invoke(card, &CardInstance::code);
	}

	auto Game::is_monster_card(const CardInstanceId card) const noexcept -> std::optional<bool>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return instance_invoke(card, &CardInstance::is_monster_card);
	}

	auto Game::is_spell_card(const CardInstanceId card) const noexcept -> std::optional<bool>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return instance_invoke(card, &CardInstance::is_spell_card);
	}

	auto Game::is_trap_card(const CardInstanceId card) const noexcept -> std::optional<bool>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return instance_invoke(card, &CardInstance::is_trap_card);
	}

	auto Game::zone_of(const CardInstanceId card) const noexcept -> domain::Zone
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return field_invoke(&Field::zone_of, card);
	}

	auto Game::location_of(const CardInstanceId card) const noexcept -> domain::Location
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return field_invoke(&Field::location_of, card);
	}

	auto Game::is_deck_zones_empty(const domain::Player player) const noexcept -> bool
	{
		return field_invoke(&Field::is_deck_zones_empty, player);
	}

	auto Game::is_extra_deck_zones_empty(const domain::Player player) const noexcept -> bool
	{
		return field_invoke(&Field::is_extra_deck_zones_empty, player);
	}

	auto Game::is_hand_zones_empty(const domain::Player player) const noexcept -> bool
	{
		return field_invoke(&Field::is_hand_zones_empty, player);
	}

	auto Game::is_graveyard_zones_empty(const domain::Player player) const noexcept -> bool
	{
		return field_invoke(&Field::is_graveyard_zones_empty, player);
	}

	auto Game::is_banished_zones_empty(const domain::Player player) const noexcept -> bool
	{
		return field_invoke(&Field::is_banished_zones_empty, player);
	}

	auto Game::is_monster_zones_empty(const domain::Player player) const noexcept -> bool
	{
		return field_invoke(&Field::is_monster_zones_empty, player);
	}

	auto Game::is_extra_monster_zones_empty(const domain::Player player) const noexcept -> bool
	{
		return field_invoke(&Field::is_extra_monster_zones_empty, player);
	}

	auto Game::is_spell_trap_zones_empty(const domain::Player player) const noexcept -> bool
	{
		return field_invoke(&Field::is_spell_trap_zones_empty, player);
	}

	auto Game::is_field_spell_zones_empty(const domain::Player player) const noexcept -> bool
	{
		return field_invoke(&Field::is_field_spell_zones_empty, player);
	}

	auto Game::is_zones_empty(const domain::Player player, const utility::EnumMask<domain::Zone> zone) const noexcept -> bool
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> bool>(&Field::is_zones_empty),
			player,
			zone
		);
	}

	auto Game::is_zones_empty(utility::EnumMask<domain::Zone> zone) const noexcept -> bool
	{
		return field_invoke(
			static_cast<auto(Field::*)(utility::EnumMask<domain::Zone> zone) const noexcept -> bool>(&Field::is_zones_empty),
			zone
		);
	}

	auto Game::count_deck_zones_card(const domain::Player player) const noexcept -> Field::size_type
	{
		return field_invoke(&Field::count_deck_zones_card, player);
	}

	auto Game::count_extra_deck_zones_card(const domain::Player player) const noexcept -> Field::size_type
	{
		return field_invoke(&Field::count_extra_deck_zones_card, player);
	}

	auto Game::count_hand_zones_card(const domain::Player player) const noexcept -> Field::size_type
	{
		return field_invoke(&Field::count_hand_zones_card, player);
	}

	auto Game::count_graveyard_zones_card(const domain::Player player) const noexcept -> Field::size_type
	{
		return field_invoke(&Field::count_graveyard_zones_card, player);
	}

	auto Game::count_banished_zones_card(const domain::Player player) const noexcept -> Field::size_type
	{
		return field_invoke(&Field::count_banished_zones_card, player);
	}

	auto Game::count_monster_zones_card(const domain::Player player) const noexcept -> Field::size_type
	{
		return field_invoke(&Field::count_monster_zones_card, player);
	}

	auto Game::count_extra_monster_zones_card() const noexcept -> Field::size_type
	{
		return field_invoke(
			static_cast<auto (Field::*)() const noexcept -> Field::size_type>(&Field::count_extra_monster_zones_card)
		);
	}

	auto Game::count_extra_monster_zones_card(const domain::Player player) const noexcept -> Field::size_type
	{
		return field_invoke(
			static_cast<auto(Field::*)(const Game& game, domain::Player player) const noexcept -> Field::size_type>(&Field::count_extra_monster_zones_card),
			std::cref(*this),
			player
		);
	}

	auto Game::count_spell_trap_zones_card(const domain::Player player) const noexcept -> Field::size_type
	{
		return field_invoke(&Field::count_spell_trap_zones_card, player);
	}

	auto Game::count_field_spell_zones_card(const domain::Player player) const noexcept -> Field::size_type
	{
		return field_invoke(&Field::count_field_spell_zones_card, player);
	}

	auto Game::count_zones_card(const domain::Player player, const utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t>(&Field::count_zones_card),
			player,
			zone
		);
	}

	auto Game::count_zones_card_ex(const domain::Player player, const utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t
	{
		return field_invoke(
			static_cast<auto(Field::*)(const Game& game, domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t>(&Field::count_zones_card),
			std::cref(*this),
			player,
			zone
		);
	}

	auto Game::count_zones_card(utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t
	{
		return field_invoke(
			static_cast<auto(Field::*)(utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t>(&Field::count_zones_card),
			zone
		);
	}

	auto Game::collect_deck_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(&Field::collect_deck_zones_card, player);
	}

	auto Game::collect_extra_deck_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(&Field::collect_extra_deck_zones_card, player);
	}

	auto Game::collect_hand_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(&Field::collect_hand_zones_card, player);
	}

	auto Game::collect_graveyard_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(&Field::collect_graveyard_zones_card, player);
	}

	auto Game::collect_banished_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(&Field::collect_banished_zones_card, player);
	}

	auto Game::collect_monster_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(&Field::collect_monster_zones_card, player);
	}

	auto Game::collect_extra_monster_zones_card() const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(
			static_cast<auto(Field::*)() const noexcept -> std::vector<CardInstanceId>>(&Field::collect_extra_monster_zones_card)
		);
	}

	auto Game::collect_extra_monster_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(
			static_cast<auto(Field::*)(const Game& game, domain::Player player) const noexcept -> std::vector<CardInstanceId>>(&Field::collect_extra_monster_zones_card),
			std::cref(*this),
			player
		);
	}

	auto Game::collect_spell_trap_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(&Field::collect_spell_trap_zones_card, player);
	}

	auto Game::collect_field_spell_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(&Field::collect_field_spell_zones_card, player);
	}

	auto Game::collect_zones_card(const domain::Player player, const utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>>(&Field::collect_zones_card),
			player,
			zone
		);
	}

	auto Game::collect_zones_card_ex(const domain::Player player, const utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(
			static_cast<auto(Field::*)(const Game& game, domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>>(&Field::collect_zones_card),
			std::cref(*this),
			player,
			zone
		);
	}

	auto Game::collect_zones_card(utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(
			static_cast<auto(Field::*)(utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>>(&Field::collect_zones_card),
			zone
		);
	}

	auto Game::is_location_empty(const domain::Player player, const domain::MonsterLocation location) const noexcept -> bool
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, domain::MonsterLocation location) const noexcept -> bool>(&Field::is_location_empty),
			player,
			location
		);
	}

	auto Game::is_location_empty(const domain::ExtraMonsterLocation location) const noexcept -> bool
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::ExtraMonsterLocation location) const noexcept -> bool>(&Field::is_location_empty),
			location
		);
	}

	auto Game::is_location_empty(const domain::Player player, const domain::SpellTrapLocation location) const noexcept -> bool
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, domain::SpellTrapLocation location) const noexcept -> bool>(&Field::is_location_empty),
			player,
			location
		);
	}

	auto Game::is_location_empty(const domain::Player player, const domain::FieldSpellLocation location) const noexcept -> bool
	{
		return field_invoke(
			static_cast<auto (Field::*)(domain::Player player, domain::FieldSpellLocation location) const noexcept -> bool>(&Field::is_location_empty),
			player,
			location
		);
	}

	auto Game::is_location_empty(const domain::Player player, const domain::Location location) const noexcept -> bool
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, domain::Location location) const noexcept -> bool>(&Field::is_location_empty),
			player,
			location
		);
	}

	auto Game::is_location_empty(const domain::Location location) const noexcept -> bool
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Location location) const noexcept -> bool>(&Field::is_location_empty),
			location
		);
	}

	auto Game::select_location_card(const domain::Player player, const domain::DeckLocation location) const noexcept -> CardInstanceId
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, domain::DeckLocation location) const noexcept -> CardInstanceId>(&Field::select_location_card),
			player,
			location
		);
	}

	auto Game::select_location_card(const domain::Player player, const domain::ExtraDeckLocation location) const noexcept -> CardInstanceId
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, domain::ExtraDeckLocation location) const noexcept -> CardInstanceId>(&Field::select_location_card),
			player,
			location
		);
	}

	auto Game::select_location_card(const domain::Player player, const domain::HandLocation location) const noexcept -> CardInstanceId
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, domain::HandLocation location) const noexcept -> CardInstanceId>(&Field::select_location_card),
			player,
			location
		);
	}

	auto Game::select_location_card(const domain::Player player, const domain::GraveyardLocation location) const noexcept -> CardInstanceId
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, domain::GraveyardLocation location) const noexcept -> CardInstanceId>(&Field::select_location_card),
			player,
			location
		);
	}

	auto Game::select_location_card(const domain::Player player, const domain::BanishedLocation location) const noexcept -> CardInstanceId
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, domain::BanishedLocation location) const noexcept -> CardInstanceId>(&Field::select_location_card),
			player,
			location
		);
	}

	auto Game::select_location_card(const domain::Player player, const domain::MonsterLocation location) const noexcept -> CardInstanceId
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, domain::MonsterLocation location) const noexcept -> CardInstanceId>(&Field::select_location_card),
			player,
			location
		);
	}

	auto Game::select_location_card(const domain::ExtraMonsterLocation location) const noexcept -> CardInstanceId
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::ExtraMonsterLocation location) const noexcept -> CardInstanceId>(&Field::select_location_card),
			location
		);
	}

	auto Game::select_location_card(const domain::Player player, const domain::SpellTrapLocation location) const noexcept -> CardInstanceId
	{
		return field_invoke(
			static_cast<auto (Field::*)(domain::Player player, domain::SpellTrapLocation location) const noexcept -> CardInstanceId>(&Field::select_location_card),
			player,
			location
		);
	}

	auto Game::select_location_card(const domain::Player player, const domain::FieldSpellLocation location) const noexcept -> CardInstanceId
	{
		return field_invoke(
			static_cast<auto (Field::*)(domain::Player player, domain::FieldSpellLocation location) const noexcept -> CardInstanceId>(&Field::select_location_card),
			player,
			location
		);
	}

	auto Game::select_location_card(const domain::Player player, const domain::Location location) const noexcept -> CardInstanceId
	{
		return field_invoke(
			static_cast<auto (Field::*)(domain::Player player, domain::Location location) const noexcept -> CardInstanceId>(&Field::select_location_card),
			player,
			location
		);
	}
}

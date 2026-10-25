#include <cg/state/game.hpp>

#include <algorithm>

#include <libassert/assert.hpp>

namespace cg::state
{
	auto Game::generate_instance_id() const noexcept -> CardInstanceId
	{
		const auto id = card_instances_.size();
		return CardInstanceId{static_cast<CardInstanceId::value_type>(id)};
	}

	auto Game::is_valid_instance_id(const CardInstanceId card) const noexcept -> bool
	{
		return card.valid() && card.raw() < card_instances_.size();
	}

	auto Game::find_definition(const domain::CardCode code) const noexcept -> const CardDefinition*
	{
		if (const auto it = card_definitions_.find(code);
			it != card_definitions_.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	auto Game::register_definition(const domain::CardCode code, const CardDefinition& definition) noexcept -> const CardDefinition&
	{
		return card_definitions_.insert_or_assign(code, definition).first->second;
	}

	auto Game::has_definition(const domain::CardCode code) const noexcept -> bool
	{
		return card_definitions_.contains(code);
	}

	auto Game::create_instance(const domain::CardCode code, const domain::Player owner) noexcept -> CardInstanceId
	{
		const auto* definition = find_definition(code);
		if (definition == nullptr)
		{
			return CardInstanceId{CardInstanceId::invalid_id};
		}

		return card_instances_.emplace_back(generate_instance_id(), owner, *definition).id();
	}

	auto Game::instances() const noexcept -> std::span<const CardInstance>
	{
		return card_instances_;
	}

	auto Game::has_instance(const CardInstanceId card) const noexcept -> bool
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return std::ranges::contains(card_instances_, card, &CardInstance::id);
	}

	auto Game::find_instance(const CardInstanceId card) const noexcept -> const CardInstance*
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		if (const auto it = std::ranges::find(card_instances_, card, &CardInstance::id);
			it != card_instances_.end())
		{
			return &*it;
		}

		return nullptr;
	}

	auto Game::find_instance_mutable(const CardInstanceId card) noexcept -> CardInstance*
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		if (const auto it = std::ranges::find(card_instances_, card, &CardInstance::id);
			it != card_instances_.end())
		{
			return &*it;
		}

		return nullptr;
	}

	auto Game::set_life_points(const domain::Player player, const std::underlying_type_t<domain::AttackDefense> value) noexcept -> void
	{
		return field_invoke(&Field::set_life_points, player, value);
	}

	auto Game::life_points(const domain::Player player) const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		return field_invoke(&Field::life_points, player);
	}

	auto Game::change_life_points(const domain::Player player, const std::underlying_type_t<domain::AttackDefense> delta) noexcept -> void
	{
		return field_invoke(&Field::change_life_points, player, delta);
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


	auto Game::is_zone_empty(domain::Player player, domain::Zone zone) const noexcept -> bool
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, domain::Zone zone) const noexcept -> bool>(&Field::is_zone_empty),
			player,
			zone
		);
	}

	auto Game::is_zone_empty(const domain::Zone zone) const noexcept -> bool
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Zone zone) const noexcept -> bool>(&Field::is_zone_empty),
			zone
		);
	}

	auto Game::is_zones_empty(const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> bool
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> bool>(&Field::is_zones_empty),
			player,
			zones
		);
	}

	auto Game::is_zones_empty(const utility::EnumMask<domain::Zone> zones) const noexcept -> bool
	{
		return field_invoke(
			static_cast<auto(Field::*)(utility::EnumMask<domain::Zone> zone) const noexcept -> bool>(&Field::is_zones_empty),
			zones
		);
	}

	auto Game::count_zone_empty_card(const domain::Player player, const domain::Zone zone) const noexcept -> Field::size_type
	{
		return field_invoke(
			&Field::count_zone_empty_card,
			player,
			zone
		);
	}

	auto Game::count_zones_empty_card(const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> Field::size_type
	{
		return field_invoke(
			&Field::count_zones_empty_card,
			player,
			zones
		);
	}

	auto Game::count_zone_card(const domain::Player player, const domain::Zone zone) const noexcept -> Field::size_type
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, domain::Zone zone) const noexcept -> Field::size_type>(&Field::count_zone_card),
			player,
			zone
		);
	}

	auto Game::count_zone_card_ex(const domain::Player player, const domain::Zone zone) const noexcept -> Field::size_type
	{
		return field_invoke(
			static_cast<auto(Field::*)(const Game& game, domain::Player player, domain::Zone zone) const noexcept -> Field::size_type>(&Field::count_zone_card),
			std::ref(*this),
			player,
			zone
		);
	}

	auto Game::count_zones_card(const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> Field::size_type
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, utility::EnumMask<domain::Zone> zones) const noexcept -> Field::size_type>(&Field::count_zones_card),
			player,
			zones
		);
	}

	auto Game::count_zones_card_ex(const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> Field::size_type
	{
		return field_invoke(
			static_cast<auto(Field::*)(const Game& game, domain::Player player, utility::EnumMask<domain::Zone> zones) const noexcept -> Field::size_type>(&Field::count_zones_card),
			std::ref(*this),
			player,
			zones
		);
	}

	auto Game::count_zones_card(const utility::EnumMask<domain::Zone> zones) const noexcept -> Field::size_type
	{
		return field_invoke(
			static_cast<auto(Field::*)(utility::EnumMask<domain::Zone> zones) const noexcept -> Field::size_type>(&Field::count_zones_card),
			zones
		);
	}

	auto Game::collect_empty_zone(const domain::Player player, const domain::Zone zone) const noexcept -> std::vector<domain::Location>
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, domain::Zone zone) const noexcept -> std::vector<domain::Location>>(&Field::collect_empty_zone),
			player,
			zone
		);
	}

	auto Game::collect_empty_zones(const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<domain::Location>
	{
		return field_invoke(
			&Field::collect_empty_zones,
			player,
			zones
		);
	}

	auto Game::collect_monster_empty_zone(const domain::Player player) const noexcept -> std::vector<domain::MonsterLocation>
	{
		return field_invoke(
			&Field::collect_monster_empty_zone,
			player
		);
	}

	auto Game::collect_extra_monster_empty_zone(const domain::Player player) const noexcept -> std::vector<domain::ExtraMonsterLocation>
	{
		return field_invoke(
			&Field::collect_extra_monster_empty_zone,
			player
		);
	}

	auto Game::collect_any_monster_empty_zone(const domain::Player player) const noexcept -> std::vector<domain::AnyMonsterLocation>
	{
		return field_invoke(
			&Field::collect_any_monster_empty_zone,
			player
		);
	}

	auto Game::collect_spell_trap_empty_zone(const domain::Player player) const noexcept -> std::vector<domain::SpellTrapLocation>
	{
		return field_invoke(
			&Field::collect_spell_trap_empty_zone,
			player
		);
	}

	auto Game::collect_zone_card(const domain::Player player, const domain::Zone zone) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, domain::Zone zone) const noexcept -> std::vector<CardInstanceId>>(&Field::collect_zone_card),
			player,
			zone
		);
	}

	auto Game::collect_zone_card_ex(const domain::Player player, const domain::Zone zone) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(
			static_cast<auto(Field::*)(const Game& game, domain::Player player, domain::Zone zone) const noexcept -> std::vector<CardInstanceId>>(&Field::collect_zone_card),
			std::ref(*this),
			player,
			zone
		);
	}

	auto Game::collect_zones_card(const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(
			static_cast<auto(Field::*)(domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>>(&Field::collect_zones_card),
			player,
			zones
		);
	}

	auto Game::collect_zones_card_ex(const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(
			static_cast<auto(Field::*)(const Game& game, domain::Player player, utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>>(&Field::collect_zones_card),
			std::cref(*this),
			player,
			zones
		);
	}

	auto Game::collect_zones_card(utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<CardInstanceId>
	{
		return field_invoke(
			static_cast<auto(Field::*)(utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>>(&Field::collect_zones_card),
			zones
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

	auto Game::select_location_card(const domain::Player player, const domain::Location location) const noexcept -> CardInstanceId
	{
		return field_invoke(
			&Field::select_location_card,
			player,
			location
		);
	}

	auto Game::place(CardInstance& card, const domain::Player controller, const domain::Placement placement) noexcept -> void
	{
		field_.remove(card.controller(), card.id(), card.placement());

		CardInstance::Modifier modifier{card};
		modifier.set_controller(controller);
		modifier.set_placement(placement);

		field_.place(controller, card.id(), placement);
	}

	auto Game::place(CardInstance& card, const domain::Placement placement) noexcept -> void
	{
		place(card, card.controller(), placement);
	}

	auto Game::move_to(CardInstance& card, const domain::Player controller, const utility::EnumMask<domain::Reason> reason, const domain::Placement placement) noexcept -> void
	{
		CardInstance::Modifier modifier{card};

		modifier.set_move_provenance(
			{
					.controller = card.controller(),
					.reason = reason,
					.placement = card.placement(),
					.effect = dsl::EffectId{dsl::EffectId::invalid_id}
			}
		);

		this->place(card, controller, placement);
	}

	auto Game::move_to_deck(CardInstance& card, const utility::EnumMask<domain::Reason> reason) noexcept -> void
	{
		const auto index = count_zone_card(card.owner(), domain::Zone::DECK);
		const domain::DeckPlacement placement{.index = index};

		move_to(card, card.owner(), reason, placement);
	}

	auto Game::move_to_extra_deck(CardInstance& card, const utility::EnumMask<domain::Reason> reason, const domain::Position position) noexcept -> void
	{
		const auto index = count_zone_card(card.owner(), domain::Zone::EXTRA_DECK);
		const domain::ExtraDeckPlacement placement{.index = index, .position = position};

		move_to(card, card.owner(), reason, placement);
	}

	auto Game::move_to_hand(CardInstance& card, const utility::EnumMask<domain::Reason> reason, const domain::Position position) noexcept -> void
	{
		const auto index = count_zone_card(card.owner(), domain::Zone::HAND);
		const domain::HandPlacement placement{.index = index, .position = position};

		move_to(card, card.owner(), reason, placement);
	}

	auto Game::move_to_graveyard(CardInstance& card, const utility::EnumMask<domain::Reason> reason) noexcept -> void
	{
		const auto index = count_zone_card(card.owner(), domain::Zone::GRAVEYARD);
		const domain::GraveyardPlacement placement{.index = index};

		move_to(card, card.owner(), reason, placement);
	}

	auto Game::move_to_banished(CardInstance& card, const utility::EnumMask<domain::Reason> reason, const domain::Position position) noexcept -> void
	{
		const auto index = count_zone_card(card.owner(), domain::Zone::BANISHED);
		const domain::BanishedPlacement placement{.index = index, .position = position};

		move_to(card, card.owner(), reason, placement);
	}

	auto Game::move_to_monster(CardInstance& card, const utility::EnumMask<domain::Reason> reason, const domain::MonsterPlacement placement) noexcept -> void
	{
		move_to(card, card.controller(), reason, placement);
	}

	auto Game::move_to_monster(CardInstance& card, const domain::Player controller, const utility::EnumMask<domain::Reason> reason, const domain::MonsterPlacement placement) noexcept -> void
	{
		move_to(card, controller, reason, placement);
	}

	auto Game::move_to_extra_monster(CardInstance& card, const utility::EnumMask<domain::Reason> reason, const domain::ExtraMonsterPlacement placement) noexcept -> void
	{
		move_to(card, card.controller(), reason, placement);
	}

	auto Game::move_to_spell_trap(CardInstance& card, const utility::EnumMask<domain::Reason> reason, const domain::SpellTrapPlacement placement) noexcept -> void
	{
		move_to(card, card.controller(), reason, placement);
	}

	auto Game::move_to_field_spell(CardInstance& card, const utility::EnumMask<domain::Reason> reason, const domain::FieldSpellPlacement placement) noexcept -> void
	{
		move_to(card, card.controller(), reason, placement);
	}

	auto Game::set_summon_record(CardInstance& card, CardInstance::SummonRecord record) noexcept -> void
	{
		std::ignore = this;

		CardInstance::Modifier modifier{card};
		modifier.set_summon_record(std::move(record));
	}

	auto Game::add_overlay(CardInstance& card, const CardInstance& material) noexcept -> void
	{
		std::ignore = this;

		CardInstance::Modifier modifier{card};
		modifier.add_overlay(material);
	}

	auto Game::remove_overlay(CardInstance& card, const CardInstance& material) noexcept -> void
	{
		std::ignore = this;

		CardInstance::Modifier modifier{card};
		modifier.remove_overlay(material);
	}
}

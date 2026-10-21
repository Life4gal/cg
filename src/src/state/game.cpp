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

	auto Game::register_definition(const domain::CardCode code, const CardDefinition& definition) noexcept -> const CardDefinition&
	{
		return card_definitions_.insert_or_assign(code, definition).first->second;
	}

	auto Game::has_definition(const domain::CardCode code) const noexcept -> bool
	{
		return card_definitions_.contains(code);
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

	auto Game::create_instance(const domain::CardCode code, const domain::Player owner) noexcept -> CardInstanceId
	{
		const auto* definition = find_definition(code);
		if (definition == nullptr)
		{
			return CardInstanceId{CardInstanceId::invalid_id};
		}

		const auto id = generate_instance_id();
		card_instances_.emplace_back(id, owner, *definition);

		return id;
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

	auto Game::definition(const CardInstanceId card) const noexcept -> std::optional<std::reference_wrapper<const CardDefinition>>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return instance_invoke(card, &CardInstance::definition);
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

	auto Game::is_card_type(const CardInstanceId card, domain::CardType type) const noexcept -> std::optional<bool>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return instance_invoke(card, &CardInstance::is_card_type, type);
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

	auto Game::is_card_subtype(const CardInstanceId card, const domain::CardSubtype subtype) const noexcept -> std::optional<bool>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return instance_invoke(
			card,
			static_cast<auto(CardInstance::*)(domain::CardSubtype subtype) const noexcept -> bool>(&CardInstance::is_card_subtype),
			subtype
		);
	}

	auto Game::is_card_subtype(const CardInstanceId card, const utility::EnumMask<domain::CardSubtype> subtypes) const noexcept -> std::optional<bool>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		return instance_invoke(
			card,
			static_cast<auto(CardInstance::*)(utility::EnumMask<domain::CardSubtype> subtypes) const noexcept -> bool>(&CardInstance::is_card_subtype),
			subtypes
		);
	}

	auto Game::attribute(const CardInstanceId card) const noexcept -> std::optional<domain::Attribute>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		if (const auto monster = is_monster_card(card);
			!monster.has_value() || !*monster)
		{
			return std::nullopt;
		}

		return instance_invoke(card, &CardInstance::attribute);
	}

	auto Game::is_attribute(const CardInstanceId card, const domain::Attribute attribute) const noexcept -> std::optional<bool>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		if (const auto monster = is_monster_card(card);
			!monster.has_value() || !*monster)
		{
			return std::nullopt;
		}

		return instance_invoke(card, &CardInstance::is_attribute, attribute);
	}

	auto Game::race(const CardInstanceId card) const noexcept -> std::optional<domain::Race>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		if (const auto monster = is_monster_card(card);
			!monster.has_value() || !*monster)
		{
			return std::nullopt;
		}

		return instance_invoke(card, &CardInstance::race);
	}

	auto Game::is_race(const CardInstanceId card, const domain::Race race) const noexcept -> std::optional<bool>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		if (const auto monster = is_monster_card(card);
			!monster.has_value() || !*monster)
		{
			return std::nullopt;
		}

		return instance_invoke(card, &CardInstance::is_race, race);
	}

	auto Game::level(const CardInstanceId card) const noexcept -> std::optional<std::underlying_type_t<domain::LevelRank>>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		if (const auto monster = is_monster_card(card);
			!monster.has_value() || !*monster)
		{
			return std::nullopt;
		}

		return instance_invoke(card, &CardInstance::level);
	}

	auto Game::is_level(const CardInstanceId card, const std::underlying_type_t<domain::LevelRank> level) const noexcept -> std::optional<bool>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		if (const auto monster = is_monster_card(card);
			!monster.has_value() || !*monster)
		{
			return std::nullopt;
		}

		return instance_invoke(card, &CardInstance::is_level, level);
	}

	auto Game::attack(const CardInstanceId card) const noexcept -> std::optional<std::underlying_type_t<domain::AttackDefense>>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		if (const auto monster = is_monster_card(card);
			!monster.has_value() || !*monster)
		{
			return std::nullopt;
		}

		return instance_invoke(card, &CardInstance::attack);
	}

	auto Game::is_attack(const CardInstanceId card, const std::underlying_type_t<domain::AttackDefense> attack) const noexcept -> std::optional<bool>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		if (const auto monster = is_monster_card(card);
			!monster.has_value() || !*monster)
		{
			return std::nullopt;
		}

		return instance_invoke(card, &CardInstance::is_attack, attack);
	}

	auto Game::defense(const CardInstanceId card) const noexcept -> std::optional<std::underlying_type_t<domain::AttackDefense>>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		if (const auto monster = is_monster_card(card);
			!monster.has_value() || !*monster)
		{
			return std::nullopt;
		}

		return instance_invoke(card, &CardInstance::defense);
	}

	auto Game::is_defense(const CardInstanceId card, const std::underlying_type_t<domain::AttackDefense> defense) const noexcept -> std::optional<bool>
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		if (const auto monster = is_monster_card(card);
			!monster.has_value() || !*monster)
		{
			return std::nullopt;
		}

		return instance_invoke(card, &CardInstance::is_defense, defense);
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

	auto Game::place(const CardInstanceId card, const domain::DeckPlacement placement) noexcept -> bool
	{
		return do_place(card, placement);
	}

	auto Game::place(const CardInstanceId card, const domain::ExtraDeckPlacement placement) noexcept -> bool
	{
		return do_place(card, placement);
	}

	auto Game::place(const CardInstanceId card, const domain::HandPlacement placement) noexcept -> bool
	{
		return do_place(card, placement);
	}

	auto Game::place(const CardInstanceId card, const domain::GraveyardPlacement placement) noexcept -> bool
	{
		return do_place(card, placement);
	}

	auto Game::place(const CardInstanceId card, const domain::BanishedPlacement placement) noexcept -> bool
	{
		return do_place(card, placement);
	}

	auto Game::place(const CardInstanceId card, const domain::MonsterPlacement placement) noexcept -> bool
	{
		return do_place(card, placement);
	}

	auto Game::place(const CardInstanceId card, const domain::ExtraMonsterPlacement placement) noexcept -> bool
	{
		return do_place(card, placement);
	}

	auto Game::place(const CardInstanceId card, const domain::SpellTrapPlacement placement) noexcept -> bool
	{
		return do_place(card, placement);
	}

	auto Game::place(const CardInstanceId card, const domain::FieldSpellPlacement placement) noexcept -> bool
	{
		return do_place(card, placement);
	}

	auto Game::place(const CardInstanceId card, const domain::Placement placement) noexcept -> bool
	{
		return do_place(card, placement);
	}

	auto Game::move_to_deck(const CardInstanceId card, const utility::EnumMask<domain::Reason> reason) noexcept -> bool
	{
		return do_move_to(
			card,
			reason,
			[&](const domain::Player player) noexcept -> domain::DeckPlacement
			{
				const auto index = count_deck_zones_card(player);
				return domain::DeckPlacement{.index = index};
			}
		);
	}

	auto Game::move_to_extra_deck(const CardInstanceId card, const utility::EnumMask<domain::Reason> reason) noexcept -> bool
	{
		return do_move_to(
			card,
			reason,
			[&](const domain::Player player) noexcept -> domain::ExtraDeckPlacement
			{
				const auto index = count_extra_deck_zones_card(player);
				return domain::ExtraDeckPlacement{.index = index, .position = domain::Position::FACE_UP_ATTACK};
			}
		);
	}

	auto Game::move_to_hand(const CardInstanceId card, const utility::EnumMask<domain::Reason> reason, const domain::Position position) noexcept -> bool
	{
		return do_move_to(
			card,
			reason,
			[&](const domain::Player player) noexcept -> domain::HandPlacement
			{
				const auto index = count_hand_zones_card(player);
				return domain::HandPlacement{.index = index, .position = position};
			}
		);
	}

	auto Game::move_to_graveyard(const CardInstanceId card, const utility::EnumMask<domain::Reason> reason) noexcept -> bool
	{
		return do_move_to(
			card,
			reason,
			[&](const domain::Player player) noexcept -> domain::GraveyardPlacement
			{
				const auto index = count_graveyard_zones_card(player);
				return domain::GraveyardPlacement{.index = index};
			}
		);
	}

	auto Game::move_to_banished(const CardInstanceId card, const utility::EnumMask<domain::Reason> reason, const domain::Position position) noexcept -> bool
	{
		return do_move_to(
			card,
			reason,
			[&](const domain::Player player) noexcept -> domain::BanishedPlacement
			{
				const auto index = count_banished_zones_card(player);
				return domain::BanishedPlacement{.index = index, .position = position};
			}
		);
	}

	auto Game::move_to_monster(const CardInstanceId card, const utility::EnumMask<domain::Reason> reason, const domain::MonsterPlacement placement) noexcept -> bool
	{
		return do_move_to(
			card,
			reason,
			[&]([[maybe_unused]] const domain::Player player) noexcept -> domain::MonsterPlacement
			{
				return placement;
			}
		);
	}

	auto Game::move_to_extra_monster(const CardInstanceId card, const utility::EnumMask<domain::Reason> reason, const domain::ExtraMonsterPlacement placement) noexcept -> bool
	{
		return do_move_to(
			card,
			reason,
			[&]([[maybe_unused]] const domain::Player player) noexcept -> domain::ExtraMonsterPlacement
			{
				return placement;
			}
		);
	}

	auto Game::move_to_spell_trap(const CardInstanceId card, const utility::EnumMask<domain::Reason> reason, const domain::SpellTrapPlacement placement) noexcept -> bool
	{
		return do_move_to(
			card,
			reason,
			[&]([[maybe_unused]] const domain::Player player) noexcept -> domain::SpellTrapPlacement
			{
				return placement;
			}
		);
	}

	auto Game::move_to_field_spell(const CardInstanceId card, const utility::EnumMask<domain::Reason> reason, const domain::FieldSpellPlacement placement) noexcept -> bool
	{
		return do_move_to(
			card,
			reason,
			[&]([[maybe_unused]] const domain::Player player) noexcept -> domain::FieldSpellPlacement
			{
				return placement;
			}
		);
	}
}

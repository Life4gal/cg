#include <engine/state/zone.hpp>

#include <algorithm>
#include <ranges>

#include <utility/functional.hpp>
#include <engine/state/state.hpp>

#include <libassert/assert.hpp>

namespace cg::engine
{
	auto opponent_location(const domain::MonsterLocation location) noexcept -> domain::MonsterLocation
	{
		const auto slot = std::to_underlying(location.slot);
		const auto opponent_slot = domain::MonsterLocation::count - 1 - slot;

		return {.slot = static_cast<domain::MonsterLocation::Slot>(opponent_slot)};
	}

	auto opponent_location(const domain::SpellTrapLocation location) noexcept -> domain::SpellTrapLocation
	{
		// 注意,仅主要魔陷区存在映射,场地魔法区域不映射
		if (location.slot == domain::SpellTrapLocation::Slot::FIELD_SPELL)
		{
			return location;
		}

		const auto slot = std::to_underlying(location.slot);
		const auto opponent_slot = domain::SpellTrapLocation::main_count - 1 - slot;
		return {.slot = static_cast<domain::SpellTrapLocation::Slot>(opponent_slot)};
	}

	auto player_zone_of(const GameState& state, const domain::Player player) noexcept -> const PlayerZone&
	{
		const auto& playground = state.playground;

		LIBASSERT_DEBUG_ASSERT(std::to_underlying(player) < playground.player_zones.size());

		return playground.player_zones[std::to_underlying(player)];
	}

	auto zone_of(const GameState& state, const CardInstanceId card) noexcept -> domain::Zone
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		const auto& playground = state.playground;

		// 先检查在不在额外怪兽区
		if (std::ranges::contains(playground.extra_monster_zones, card))
		{
			return domain::Zone::EXTRA_MONSTER;
		}

		// 检查双方场地
		for (const auto& player_zone: playground.player_zones)
		{
			// 怪兽区
			if (std::ranges::contains(player_zone.monster_zones, card))
			{
				return domain::Zone::MONSTER;
			}
			// 魔陷区+场地魔法区
			if (std::ranges::contains(player_zone.spell_trap_zones, card))
			{
				return domain::Zone::SPELL_TRAP;
			}
			// 卡组
			if (std::ranges::contains(player_zone.deck_zones, card))
			{
				return domain::Zone::EXTRA_DECK;
			}
			// 额外卡组
			if (std::ranges::contains(player_zone.extra_deck_zones, card))
			{
				return domain::Zone::EXTRA_DECK;
			}
			// 手牌
			if (std::ranges::contains(player_zone.hand_zones, card))
			{
				return domain::Zone::HAND;
			}
			// 墓地
			if (std::ranges::contains(player_zone.graveyard_zones, card))
			{
				return domain::Zone::GRAVEYARD;
			}
			// 除外区
			if (std::ranges::contains(player_zone.banished_zones, card))
			{
				return domain::Zone::BANISHED;
			}
		}

		return domain::Zone::NOT_EXIST;
	}

	auto location_of(const GameState& state, const CardInstanceId card) noexcept -> domain::Location
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		const auto& playground = state.playground;

		// 先检查在不在额外怪兽区
		for (const auto slot: domain::ExtraMonsterLocation::slots)
		{
			if (playground.extra_monster_zones[std::to_underlying(slot)] == card)
			{
				const auto location = domain::ExtraMonsterLocation{.slot = slot};

				return {location};
			}
		}

		// 检查双方场地
		for (const auto& player_zone: playground.player_zones)
		{
			// 怪兽区
			for (const auto slot: domain::MonsterLocation::slots)
			{
				if (player_zone.monster_zones[std::to_underlying(slot)] == card)
				{
					const auto location = domain::MonsterLocation{.slot = slot};

					return {location};
				}
			}
			// 魔陷区+场地魔法区
			for (const auto slot: domain::SpellTrapLocation::slots)
			{
				if (player_zone.spell_trap_zones[std::to_underlying(slot)] == card)
				{
					const auto location = domain::SpellTrapLocation{.slot = slot};

					return {location};
				}
			}

			// 卡组/额外卡组/手牌/墓地/除外区
			if (const auto it = std::ranges::find(player_zone.deck_zones, card);
				it != player_zone.deck_zones.end())
			{
				const auto index = std::ranges::distance(player_zone.deck_zones.begin(), it);
				const auto location = domain::DeckLocation{.index = static_cast<domain::DeckLocation::size_type>(index)};

				return {location};
			}
			if (const auto it = std::ranges::find(player_zone.extra_deck_zones, card);
				it != player_zone.extra_deck_zones.end())
			{
				const auto index = std::ranges::distance(player_zone.extra_deck_zones.begin(), it);
				const auto location = domain::ExtraDeckLocation{.index = static_cast<domain::ExtraDeckLocation::size_type>(index)};

				return {location};
			}
			if (const auto it = std::ranges::find(player_zone.hand_zones, card);
				it != player_zone.hand_zones.end())
			{
				const auto index = std::ranges::distance(player_zone.hand_zones.begin(), it);
				const auto location = domain::HandLocation{.index = static_cast<domain::HandLocation::size_type>(index)};

				return {location};
			}
			if (const auto it = std::ranges::find(player_zone.graveyard_zones, card);
				it != player_zone.graveyard_zones.end())
			{
				const auto index = std::ranges::distance(player_zone.graveyard_zones.begin(), it);
				const auto location = domain::GraveyardLocation{.index = static_cast<domain::GraveyardLocation::size_type>(index)};

				return {location};
			}
			if (const auto it = std::ranges::find(player_zone.banished_zones, card);
				it != player_zone.banished_zones.end())
			{
				const auto index = std::ranges::distance(player_zone.banished_zones.begin(), it);
				const auto location = domain::BanishedLocation{.index = static_cast<domain::BanishedLocation::size_type>(index)};

				return {location};
			}
		}

		return {domain::NotExistLocation{}};
	}

	namespace
	{
		template<std::same_as<PlayerZone::bag_zones_type> Zones>
		[[nodiscard]] auto check_bag_zones_empty(const Zones& zones) noexcept -> bool
		{
			return zones.empty();
		}
	}

	auto is_deck_zones_empty(const GameState& state, const domain::Player player) noexcept -> bool
	{
		const auto& player_zone = player_zone_of(state, player);

		return check_bag_zones_empty(player_zone.deck_zones);
	}

	auto is_extra_deck_zones_empty(const GameState& state, const domain::Player player) noexcept -> bool
	{
		const auto& player_zone = player_zone_of(state, player);

		return check_bag_zones_empty(player_zone.extra_deck_zones);
	}

	auto is_hand_zones_empty(const GameState& state, const domain::Player player) noexcept -> bool
	{
		const auto& player_zone = player_zone_of(state, player);

		return check_bag_zones_empty(player_zone.hand_zones);
	}

	auto is_graveyard_zones_empty(const GameState& state, const domain::Player player) noexcept -> bool
	{
		const auto& player_zone = player_zone_of(state, player);

		return check_bag_zones_empty(player_zone.graveyard_zones);
	}

	auto is_banished_zones_empty(const GameState& state, const domain::Player player) noexcept -> bool
	{
		const auto& player_zone = player_zone_of(state, player);

		return check_bag_zones_empty(player_zone.banished_zones);
	}

	auto is_monster_zones_empty(const GameState& state, const domain::Player player) noexcept -> bool
	{
		const auto& player_zone = player_zone_of(state, player);

		return std::ranges::none_of(player_zone.monster_zones, &CardInstanceId::valid);
	}

	auto is_extra_monster_zones_empty(const GameState& state, const domain::Player player) noexcept -> bool
	{
		return std::ranges::none_of(state.playground.extra_monster_zones, &CardInstanceId::valid);
	}

	auto is_spell_trap_zones_empty(const GameState& state, const domain::Player player) noexcept -> bool
	{
		const auto& player_zone = player_zone_of(state, player);

		return std::ranges::none_of(player_zone.spell_trap_zones, &CardInstanceId::valid);
	}

	auto is_zones_empty(const GameState& state, const domain::Player player, const domain::Zone zone) noexcept -> bool
	{
		// 卡组
		if (zone & domain::Zone::DECK)
		{
			if (!is_deck_zones_empty(state, player))
			{
				return false;
			}
		}
		// 额外卡组
		if (zone & domain::Zone::EXTRA_DECK)
		{
			if (!is_extra_deck_zones_empty(state, player))
			{
				return false;
			}
		}
		// 手牌
		if (zone & domain::Zone::HAND)
		{
			if (!is_hand_zones_empty(state, player))
			{
				return false;
			}
		}
		// 墓地
		if (zone & domain::Zone::GRAVEYARD)
		{
			if (!is_graveyard_zones_empty(state, player))
			{
				return false;
			}
		}
		// 除外区
		if (zone & domain::Zone::BANISHED)
		{
			if (!is_banished_zones_empty(state, player))
			{
				return false;
			}
		}
		// 怪兽区
		if (zone & domain::Zone::MONSTER)
		{
			if (!is_monster_zones_empty(state, player))
			{
				return false;
			}
		}
		// 额外怪兽区
		if (zone & domain::Zone::EXTRA_MONSTER)
		{
			if (!is_extra_monster_zones_empty(state, player))
			{
				return false;
			}
		}
		// 魔陷区
		if (zone & domain::Zone::SPELL_TRAP)
		{
			if (!is_spell_trap_zones_empty(state, player))
			{
				return false;
			}
		}

		return true;
	}

	auto is_zones_empty(const GameState& state, const domain::Zone zone) noexcept -> bool
	{
		return is_zones_empty(state, domain::Player::FIRST, zone) && is_zones_empty(state, domain::Player::SECOND, zone);
	}

	namespace
	{
		template<std::same_as<PlayerZone::bag_zones_type> Zones>
		[[nodiscard]] auto count_bag_zones(const Zones& zones) noexcept -> domain::DeckLocation::size_type
		{
			// bag中所有实例都是合法的
			LIBASSERT_DEBUG_ASSERT(
				std::ranges::all_of(
					zones,
					[](const CardInstanceId instace) noexcept -> bool
					{
					return instace.valid();
					}
				)
			);

			const auto size = zones.size();
			LIBASSERT_DEBUG_ASSERT(size <= std::numeric_limits<domain::DeckLocation::size_type>::max());

			return static_cast<domain::DeckLocation::size_type>(size);
		}
	}

	auto count_deck_zones_card(const GameState& state, const domain::Player player) noexcept -> domain::DeckLocation::size_type
	{
		const auto& player_zone = player_zone_of(state, player);

		return count_bag_zones(player_zone.deck_zones);
	}

	auto count_extra_deck_zones_card(const GameState& state, const domain::Player player) noexcept -> domain::DeckLocation::size_type
	{
		const auto& player_zone = player_zone_of(state, player);

		return count_bag_zones(player_zone.extra_deck_zones);
	}

	auto count_hand_zones_card(const GameState& state, const domain::Player player) noexcept -> domain::DeckLocation::size_type
	{
		const auto& player_zone = player_zone_of(state, player);

		return count_bag_zones(player_zone.hand_zones);
	}

	auto count_graveyard_zones_card(const GameState& state, const domain::Player player) noexcept -> domain::DeckLocation::size_type
	{
		const auto& player_zone = player_zone_of(state, player);

		return count_bag_zones(player_zone.graveyard_zones);
	}

	auto count_banished_zones_card(const GameState& state, const domain::Player player) noexcept -> domain::DeckLocation::size_type
	{
		const auto& player_zone = player_zone_of(state, player);

		return count_bag_zones(player_zone.banished_zones);
	}

	auto count_monster_zones_card(const GameState& state, const domain::Player player) noexcept -> domain::DeckLocation::size_type
	{
		const auto& player_zone = player_zone_of(state, player);

		const auto size = std::ranges::count_if(player_zone.monster_zones, &CardInstanceId::valid);
		LIBASSERT_DEBUG_ASSERT(size <= domain::MonsterLocation::count);

		return static_cast<domain::DeckLocation::size_type>(size);
	}

	auto count_extra_monster_zones_card(const GameState& state, const domain::Player player) noexcept -> domain::DeckLocation::size_type
	{
		const auto size = std::ranges::count_if(
			state.playground.extra_monster_zones,
			[&state, player](const CardInstanceId id) noexcept -> bool
			{
				// 额外怪兽区要过滤掉非法区域
				if (!id.valid())
				{
					return false;
				}

				// 实例控制者必须是指定玩家
				const auto controller = controller_of(state, id);
				return controller.has_value() && *controller == player;
			}
		);
		LIBASSERT_DEBUG_ASSERT(size <= domain::ExtraMonsterLocation::count);

		return static_cast<domain::DeckLocation::size_type>(size);
	}

	auto count_spell_trap_zones_card(const GameState& state, const domain::Player player) noexcept -> domain::DeckLocation::size_type
	{
		const auto& player_zone = player_zone_of(state, player);

		const auto size = std::ranges::count_if(player_zone.spell_trap_zones, &CardInstanceId::valid);
		LIBASSERT_DEBUG_ASSERT(size <= domain::SpellTrapLocation::count);

		return static_cast<domain::DeckLocation::size_type>(size);
	}

	auto count_zones_card(const GameState& state, const domain::Player player, const domain::Zone zone) noexcept -> std::size_t
	{
		std::size_t count = 0;

		// 卡组
		if (zone & domain::Zone::DECK)
		{
			count += count_deck_zones_card(state, player);
		}
		// 额外卡组
		if (zone & domain::Zone::EXTRA_DECK)
		{
			count += count_extra_deck_zones_card(state, player);
		}
		// 手牌
		if (zone & domain::Zone::HAND)
		{
			count += count_hand_zones_card(state, player);
		}
		// 墓地
		if (zone & domain::Zone::GRAVEYARD)
		{
			count += count_graveyard_zones_card(state, player);
		}
		// 除外区
		if (zone & domain::Zone::BANISHED)
		{
			count += count_banished_zones_card(state, player);
		}
		// 怪兽区
		if (zone & domain::Zone::MONSTER)
		{
			count += count_monster_zones_card(state, player);
		}
		// 额外怪兽区
		if (zone & domain::Zone::EXTRA_MONSTER)
		{
			count += count_extra_monster_zones_card(state, player);
		}
		// 魔陷区
		if (zone & domain::Zone::SPELL_TRAP)
		{
			count += count_spell_trap_zones_card(state, player);
		}

		return count;
	}

	auto count_zones_card(const GameState& state, const domain::Zone zone) noexcept -> std::size_t
	{
		return count_zones_card(state, domain::Player::FIRST, zone) + count_zones_card(state, domain::Player::SECOND, zone);
	}

	namespace
	{
		template<std::same_as<PlayerZone::bag_zones_type> Zones>
		auto collect_bag_zones(const Zones& zones, std::vector<CardInstanceId>& out) noexcept -> void
		{
			// bag中所有实例都是合法的
			LIBASSERT_DEBUG_ASSERT(
				std::ranges::all_of(
					zones,
					[](const CardInstanceId instace) noexcept -> bool
					{
					return instace.valid();
					}
				)
			);

			// FIXME: 除外区的里侧卡牌是否需要?
			out.append_range(zones);
		}

		auto collect_monster_zones(const PlayerZone::monster_zones_type& monster_zones, std::vector<CardInstanceId>& out) noexcept -> void
		{
			auto valid_zones = monster_zones |
			                   // 怪兽区要过滤掉非法区域
			                   std::views::filter(&CardInstanceId::valid);

			out.append_range(valid_zones);
		}

		auto collect_extra_monster_zones(
			const GameState& state,
			const Playground::extra_monster_zones_type& extra_monster_zones,
			const domain::Player player,
			std::vector<CardInstanceId>& out
		) noexcept -> void
		{
			auto valid_zones = extra_monster_zones |
			                   // 额外怪兽区要过滤掉非法区域
			                   std::views::filter(&CardInstanceId::valid) |
			                   // 实例控制者必须是指定玩家
			                   std::views::filter(
				                   [&state, player](const CardInstanceId id) noexcept -> bool
				                   {
					                   const auto controller = controller_of(state, id);
					                   return controller.has_value() && *controller == player;
				                   }
			                   );

			out.append_range(valid_zones);
		}

		auto collect_spell_trap_zones(const PlayerZone::spell_trap_zones_type& spell_trap_zones, std::vector<CardInstanceId>& out) noexcept -> void
		{
			auto valid_zones = spell_trap_zones |
			                   // 魔陷区要过滤掉非法区域
			                   std::views::filter(&CardInstanceId::valid);

			out.append_range(valid_zones);
		}

		auto collect_zones(const GameState& state, const domain::Player player, const domain::Zone zone, std::vector<CardInstanceId>& out) noexcept -> void
		{
			const auto& player_zone = player_zone_of(state, player);

			// 卡组
			if (zone & domain::Zone::DECK)
			{
				collect_bag_zones(player_zone.deck_zones, out);
			}
			// 额外卡组
			if (zone & domain::Zone::EXTRA_DECK)
			{
				collect_bag_zones(player_zone.extra_deck_zones, out);
			}
			// 手牌
			if (zone & domain::Zone::HAND)
			{
				collect_bag_zones(player_zone.hand_zones, out);
			}
			// 墓地
			if (zone & domain::Zone::GRAVEYARD)
			{
				collect_bag_zones(player_zone.graveyard_zones, out);
			}
			// 除外区
			if (zone & domain::Zone::BANISHED)
			{
				collect_bag_zones(player_zone.banished_zones, out);
			}
			// 怪兽区
			if (zone & domain::Zone::MONSTER)
			{
				collect_monster_zones(player_zone.monster_zones, out);
			}
			// 额外怪兽区
			if (zone & domain::Zone::EXTRA_MONSTER)
			{
				collect_extra_monster_zones(state, state.playground.extra_monster_zones, player, out);
			}
			// 魔陷区
			if (zone & domain::Zone::SPELL_TRAP)
			{
				collect_spell_trap_zones(player_zone.spell_trap_zones, out);
			}
		}
	}

	auto collect_deck_zones_card(const GameState& state, const domain::Player player) noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(state, player);

		const auto count = count_deck_zones_card(state, player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_bag_zones(player_zone.deck_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto collect_extra_deck_zones_card(const GameState& state, const domain::Player player) noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(state, player);

		const auto count = count_extra_deck_zones_card(state, player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_bag_zones(player_zone.extra_deck_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto collect_hand_zones_card(const GameState& state, const domain::Player player) noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(state, player);

		const auto count = count_hand_zones_card(state, player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_bag_zones(player_zone.hand_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto collect_graveyard_zones_card(const GameState& state, const domain::Player player) noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(state, player);

		const auto count = count_graveyard_zones_card(state, player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_bag_zones(player_zone.graveyard_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto collect_banished_zones_card(const GameState& state, const domain::Player player) noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(state, player);

		const auto count = count_banished_zones_card(state, player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_bag_zones(player_zone.banished_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto collect_monster_zones_card(const GameState& state, const domain::Player player) noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(state, player);

		const auto count = count_monster_zones_card(state, player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_monster_zones(player_zone.monster_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto collect_extra_monster_zones_card(const GameState& state, const domain::Player player) noexcept -> std::vector<CardInstanceId>
	{
		const auto count = count_extra_monster_zones_card(state, player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_extra_monster_zones(state, state.playground.extra_monster_zones, player, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto collect_spell_trap_zones_card(const GameState& state, const domain::Player player) noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(state, player);

		const auto count = count_spell_trap_zones_card(state, player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_spell_trap_zones(player_zone.spell_trap_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto collect_zones_card(const GameState& state, const domain::Player player, const domain::Zone zone) noexcept -> std::vector<CardInstanceId>
	{
		const auto count = count_zones_card(state, player, zone);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_zones(state, player, zone, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto collect_zones_card(const GameState& state, const domain::Zone zone) noexcept -> std::vector<CardInstanceId>
	{
		const auto count = count_zones_card(state, zone);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_zones(state, domain::Player::FIRST, zone, out);
		collect_zones(state, domain::Player::SECOND, zone, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto is_location_empty(const GameState& state, const domain::Player player, const domain::MonsterLocation location) noexcept -> bool
	{
		const auto& player_zone = player_zone_of(state, player);
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(location.slot) < player_zone.monster_zones.size());

		return !player_zone.monster_zones[std::to_underlying(location.slot)].valid();
	}

	auto is_location_empty(const GameState& state, const domain::Player player, const domain::ExtraMonsterLocation location) noexcept -> bool
	{
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(location.slot) < state.playground.extra_monster_zones.size());

		return !state.playground.extra_monster_zones[std::to_underlying(location.slot)].valid();
	}

	auto is_location_empty(const GameState& state, const domain::Player player, const domain::SpellTrapLocation location) noexcept -> bool
	{
		const auto& player_zone = player_zone_of(state, player);
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(location.slot) < player_zone.spell_trap_zones.size());

		return !player_zone.spell_trap_zones[std::to_underlying(location.slot)].valid();
	}

	auto is_location_empty(const GameState& state, const domain::Player player, const domain::Location location) noexcept -> bool
	{
		return std::visit(
			utility::Overloaded
			{
					[&]<typename Location>(const Location& l) noexcept -> bool //
						requires(std::is_same_v<Location, domain::MonsterLocation> || std::is_same_v<Location, domain::ExtraMonsterLocation> || std::is_same_v<Location, domain::SpellTrapLocation>)
					{
						return engine::is_location_empty(state, player, l);
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> bool
					{
						return false;
					}
			},
			location
		);
	}

	auto is_location_empty(const GameState& state, const domain::Location location) noexcept -> bool
	{
		return is_location_empty(state, domain::Player::FIRST, location) && is_location_empty(state, domain::Player::SECOND, location);
	}

	auto select_location_card(const GameState& state, const domain::Player player, const domain::DeckLocation location) noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(state, player);
		LIBASSERT_DEBUG_ASSERT(location.index < player_zone.deck_zones.size());

		return player_zone.deck_zones[location.index];
	}

	auto select_location_card(const GameState& state, const domain::Player player, const domain::ExtraDeckLocation location) noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(state, player);
		LIBASSERT_DEBUG_ASSERT(location.index < player_zone.extra_deck_zones.size());

		return player_zone.extra_deck_zones[location.index];
	}

	auto select_location_card(const GameState& state, const domain::Player player, const domain::HandLocation location) noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(state, player);
		LIBASSERT_DEBUG_ASSERT(location.index < player_zone.hand_zones.size());

		return player_zone.hand_zones[location.index];
	}

	auto select_location_card(const GameState& state, const domain::Player player, const domain::GraveyardLocation location) noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(state, player);
		LIBASSERT_DEBUG_ASSERT(location.index < player_zone.graveyard_zones.size());

		return player_zone.graveyard_zones[location.index];
	}

	auto select_location_card(const GameState& state, const domain::Player player, const domain::BanishedLocation location) noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(state, player);
		LIBASSERT_DEBUG_ASSERT(location.index < player_zone.banished_zones.size());

		return player_zone.banished_zones[location.index];
	}

	auto select_location_card(const GameState& state, const domain::Player player, const domain::MonsterLocation location) noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(state, player);
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(location.slot) < player_zone.monster_zones.size());

		return player_zone.monster_zones[std::to_underlying(location.slot)];
	}

	auto select_location_card(const GameState& state, const domain::Player player, const domain::ExtraMonsterLocation location) noexcept -> CardInstanceId
	{
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(location.slot) < state.playground.extra_monster_zones.size());

		return state.playground.extra_monster_zones[std::to_underlying(location.slot)];
	}

	auto select_location_card(const GameState& state, const domain::Player player, const domain::SpellTrapLocation location) noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(state, player);
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(location.slot) < player_zone.spell_trap_zones.size());

		return player_zone.spell_trap_zones[std::to_underlying(location.slot)];
	}

	auto select_location_card(const GameState& state, const domain::Player player, const domain::Location location) noexcept -> CardInstanceId
	{
		return std::visit(
			[&](const auto& l) noexcept -> CardInstanceId
			{
				return engine::select_location_card(state, player, l);
			},
			location
		);
	}
}

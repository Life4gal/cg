#include <cg/state/field.hpp>

#include <algorithm>
#include <ranges>

#include <cg/utility/functional.hpp>
#include <cg/state/game.hpp>

#include <libassert/assert.hpp>

namespace cg::state
{
	auto Field::player_zone_of(const domain::Player player) const noexcept -> const PlayerZone&
	{
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(player) < player_zones_.size());

		return player_zones_[std::to_underlying(player)];
	}

	Field::Field() noexcept
		: player_zones_{}
	// extra_monster_zones_{}
	{}

	auto Field::zone_of(const CardInstanceId card) const noexcept -> domain::Zone
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		// 先检查在不在额外怪兽区
		if (std::ranges::contains(extra_monster_zones_, card))
		{
			return domain::Zone::EXTRA_MONSTER;
		}

		// 检查双方场地
		for (const auto& player_zone: player_zones_)
		{
			// 怪兽区
			if (std::ranges::contains(player_zone.monster_zones, card))
			{
				return domain::Zone::MONSTER;
			}
			// 魔陷区
			if (std::ranges::contains(player_zone.spell_trap_zones, card))
			{
				return domain::Zone::SPELL_TRAP;
			}
			//场地魔法区
			if (player_zone.field_spell_zone == card)
			{
				return domain::Zone::FIELD_SPELL;
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

	auto Field::location_of(const CardInstanceId card) const noexcept -> domain::Location
	{
		LIBASSERT_DEBUG_ASSERT(card.valid());

		// 先检查在不在额外怪兽区
		for (const auto column: domain::extra_columns)
		{
			if (extra_monster_zones_[std::to_underlying(column)] == card)
			{
				const auto location = domain::ExtraMonsterLocation{.column = column};

				return {location};
			}
		}

		// 检查双方场地
		for (const auto& player_zone: player_zones_)
		{
			// 怪兽区 + 魔陷区
			for (const auto column: domain::field_columns)
			{
				if (player_zone.monster_zones[std::to_underlying(column)] == card)
				{
					const auto location = domain::MonsterLocation{.column = column};

					return {location};
				}

				if (player_zone.spell_trap_zones[std::to_underlying(column)] == card)
				{
					const auto location = domain::SpellTrapLocation{.column = column};

					return {location};
				}
			}

			// 场地魔法区
			if (player_zone.field_spell_zone == card)
			{
				constexpr auto location = domain::FieldSpellLocation{};

				return {location};
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

	auto Field::is_deck_zones_empty(const domain::Player player) const noexcept -> bool
	{
		const auto& player_zone = player_zone_of(player);

		return check_bag_zones_empty(player_zone.deck_zones);
	}

	auto Field::is_extra_deck_zones_empty(const domain::Player player) const noexcept -> bool
	{
		const auto& player_zone = player_zone_of(player);

		return check_bag_zones_empty(player_zone.extra_deck_zones);
	}

	auto Field::is_hand_zones_empty(const domain::Player player) const noexcept -> bool
	{
		const auto& player_zone = player_zone_of(player);

		return check_bag_zones_empty(player_zone.hand_zones);
	}

	auto Field::is_graveyard_zones_empty(const domain::Player player) const noexcept -> bool
	{
		const auto& player_zone = player_zone_of(player);

		return check_bag_zones_empty(player_zone.graveyard_zones);
	}

	auto Field::is_banished_zones_empty(const domain::Player player) const noexcept -> bool
	{
		const auto& player_zone = player_zone_of(player);

		return check_bag_zones_empty(player_zone.banished_zones);
	}

	auto Field::is_monster_zones_empty(const domain::Player player) const noexcept -> bool
	{
		const auto& player_zone = player_zone_of(player);

		return std::ranges::none_of(player_zone.monster_zones, &CardInstanceId::valid);
	}

	auto Field::is_extra_monster_zones_empty(const domain::Player player) const noexcept -> bool
	{
		return std::ranges::none_of(extra_monster_zones_, &CardInstanceId::valid);
	}

	auto Field::is_spell_trap_zones_empty(const domain::Player player) const noexcept -> bool
	{
		const auto& player_zone = player_zone_of(player);

		return std::ranges::none_of(player_zone.spell_trap_zones, &CardInstanceId::valid);
	}

	auto Field::is_field_spell_zones_empty(const domain::Player player) const noexcept -> bool
	{
		const auto& player_zone = player_zone_of(player);

		return !player_zone.field_spell_zone.valid();
	}

	auto Field::is_zones_empty(const domain::Player player, const utility::EnumMask<domain::Zone> zone) const noexcept -> bool
	{
		// 卡组
		if (zone.contains(domain::Zone::DECK))
		{
			if (!is_deck_zones_empty(player))
			{
				return false;
			}
		}
		// 额外卡组
		if (zone.contains(domain::Zone::EXTRA_DECK))
		{
			if (!is_extra_deck_zones_empty(player))
			{
				return false;
			}
		}
		// 手牌
		if (zone.contains(domain::Zone::HAND))
		{
			if (!is_hand_zones_empty(player))
			{
				return false;
			}
		}
		// 墓地
		if (zone.contains(domain::Zone::GRAVEYARD))
		{
			if (!is_graveyard_zones_empty(player))
			{
				return false;
			}
		}
		// 除外区
		if (zone.contains(domain::Zone::BANISHED))
		{
			if (!is_banished_zones_empty(player))
			{
				return false;
			}
		}
		// 怪兽区
		if (zone.contains(domain::Zone::MONSTER))
		{
			if (!is_monster_zones_empty(player))
			{
				return false;
			}
		}
		// 额外怪兽区
		if (zone.contains(domain::Zone::EXTRA_MONSTER))
		{
			if (!is_extra_monster_zones_empty(player))
			{
				return false;
			}
		}
		// 魔陷区
		if (zone.contains(domain::Zone::SPELL_TRAP))
		{
			if (!is_spell_trap_zones_empty(player))
			{
				return false;
			}
		}

		// 场地魔法区
		if (zone.contains(domain::Zone::FIELD_SPELL))
		{
			if (!is_field_spell_zones_empty(player))
			{
				return false;
			}
		}

		return true;
	}

	auto Field::is_zones_empty(const utility::EnumMask<domain::Zone> zone) const noexcept -> bool
	{
		return is_zones_empty(domain::Player::FIRST, zone) && is_zones_empty(domain::Player::SECOND, zone);
	}

	namespace
	{
		template<std::same_as<PlayerZone::bag_zones_type> Zones>
		[[nodiscard]] auto count_bag_zones(const Zones& zones) noexcept -> domain::DeckLocation::size_type
		{
			// bag中所有实例都是合法的
			LIBASSERT_DEBUG_ASSERT(std::ranges::all_of(zones,&CardInstanceId::valid));

			const auto size = zones.size();
			LIBASSERT_DEBUG_ASSERT(size <= std::numeric_limits<domain::DeckLocation::size_type>::max());

			return static_cast<domain::DeckLocation::size_type>(size);
		}
	}

	auto Field::count_deck_zones_card(const domain::Player player) const noexcept -> size_type
	{
		const auto& player_zone = player_zone_of(player);

		return count_bag_zones(player_zone.deck_zones);
	}

	auto Field::count_extra_deck_zones_card(const domain::Player player) const noexcept -> size_type
	{
		const auto& player_zone = player_zone_of(player);

		return count_bag_zones(player_zone.extra_deck_zones);
	}

	auto Field::count_hand_zones_card(const domain::Player player) const noexcept -> size_type
	{
		const auto& player_zone = player_zone_of(player);

		return count_bag_zones(player_zone.hand_zones);
	}

	auto Field::count_graveyard_zones_card(const domain::Player player) const noexcept -> size_type
	{
		const auto& player_zone = player_zone_of(player);

		return count_bag_zones(player_zone.graveyard_zones);
	}

	auto Field::count_banished_zones_card(const domain::Player player) const noexcept -> size_type
	{
		const auto& player_zone = player_zone_of(player);

		return count_bag_zones(player_zone.banished_zones);
	}

	auto Field::count_monster_zones_card(const domain::Player player) const noexcept -> size_type
	{
		const auto& player_zone = player_zone_of(player);

		const auto size = std::ranges::count_if(player_zone.monster_zones, &CardInstanceId::valid);
		LIBASSERT_DEBUG_ASSERT(size <= domain::total_field_columns);

		return static_cast<domain::DeckLocation::size_type>(size);
	}

	auto Field::count_extra_monster_zones_card() const noexcept -> size_type
	{
		const auto size = std::ranges::count_if(extra_monster_zones_, &CardInstanceId::valid);
		LIBASSERT_DEBUG_ASSERT(size <= domain::total_extra_columns);

		return static_cast<domain::DeckLocation::size_type>(size);
	}

	auto Field::count_extra_monster_zones_card(const Game& game, const domain::Player player) const noexcept -> size_type
	{
		const auto size = std::ranges::count_if(
			extra_monster_zones_,
			[&game, player](const CardInstanceId id) noexcept -> bool
			{
				// 额外怪兽区要过滤掉非法区域
				if (!id.valid())
				{
					return false;
				}

				// 实例控制者必须是指定玩家
				const auto controller = game.controller(id);
				return controller.has_value() && *controller == player;
			}
		);
		LIBASSERT_DEBUG_ASSERT(size <= domain::total_extra_columns);

		return static_cast<domain::DeckLocation::size_type>(size);
	}

	auto Field::count_spell_trap_zones_card(const domain::Player player) const noexcept -> size_type
	{
		const auto& player_zone = player_zone_of(player);

		const auto size = std::ranges::count_if(player_zone.spell_trap_zones, &CardInstanceId::valid);
		LIBASSERT_DEBUG_ASSERT(size <= domain::total_extra_columns);

		return static_cast<domain::DeckLocation::size_type>(size);
	}

	auto Field::count_field_spell_zones_card(const domain::Player player) const noexcept -> size_type
	{
		if (const auto& player_zone = player_zone_of(player);
			!player_zone.field_spell_zone.valid())
		{
			return 0;
		}

		return 1;
	}

	auto Field::count_zones_card(const domain::Player player, const utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t
	{
		std::size_t count = 0;

		// 卡组
		if (zone.contains(domain::Zone::DECK))
		{
			count += count_deck_zones_card(player);
		}
		// 额外卡组
		if (zone.contains(domain::Zone::EXTRA_DECK))
		{
			count += count_extra_deck_zones_card(player);
		}
		// 手牌
		if (zone.contains(domain::Zone::HAND))
		{
			count += count_hand_zones_card(player);
		}
		// 墓地
		if (zone.contains(domain::Zone::GRAVEYARD))
		{
			count += count_graveyard_zones_card(player);
		}
		// 除外区
		if (zone.contains(domain::Zone::BANISHED))
		{
			count += count_banished_zones_card(player);
		}
		// 怪兽区
		if (zone.contains(domain::Zone::MONSTER))
		{
			count += count_monster_zones_card(player);
		}
		// 额外怪兽区
		if (zone.contains(domain::Zone::EXTRA_MONSTER))
		{
			count += count_extra_monster_zones_card();
		}
		// 魔陷区
		if (zone.contains(domain::Zone::SPELL_TRAP))
		{
			count += count_spell_trap_zones_card(player);
		}
		// 场地魔法区
		if (zone.contains(domain::Zone::FIELD_SPELL))
		{
			count += count_field_spell_zones_card(player);
		}

		return count;
	}

	auto Field::count_zones_card(const Game& game, const domain::Player player, const utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t
	{
		std::size_t count = 0;

		// 卡组
		if (zone.contains(domain::Zone::DECK))
		{
			count += count_deck_zones_card(player);
		}
		// 额外卡组
		if (zone.contains(domain::Zone::EXTRA_DECK))
		{
			count += count_extra_deck_zones_card(player);
		}
		// 手牌
		if (zone.contains(domain::Zone::HAND))
		{
			count += count_hand_zones_card(player);
		}
		// 墓地
		if (zone.contains(domain::Zone::GRAVEYARD))
		{
			count += count_graveyard_zones_card(player);
		}
		// 除外区
		if (zone.contains(domain::Zone::BANISHED))
		{
			count += count_banished_zones_card(player);
		}
		// 怪兽区
		if (zone.contains(domain::Zone::MONSTER))
		{
			count += count_monster_zones_card(player);
		}
		// 额外怪兽区
		if (zone.contains(domain::Zone::EXTRA_MONSTER))
		{
			count += count_extra_monster_zones_card(game, player);
		}
		// 魔陷区
		if (zone.contains(domain::Zone::SPELL_TRAP))
		{
			count += count_spell_trap_zones_card(player);
		}
		// 场地魔法区
		if (zone.contains(domain::Zone::FIELD_SPELL))
		{
			count += count_field_spell_zones_card(player);
		}

		return count;
	}

	auto Field::count_zones_card(const utility::EnumMask<domain::Zone> zone) const noexcept -> std::size_t
	{
		// 如果不包含额外怪兽区则直接统计
		if (!zone.contains(domain::Zone::EXTRA_MONSTER))
		{
			return count_zones_card(domain::Player::FIRST, zone) + count_zones_card(domain::Player::SECOND, zone);
		}

		// 先统计非额外怪兽区
		const auto non_ex_zone = zone.remove(domain::Zone::EXTRA_MONSTER);
		auto count = count_zones_card(domain::Player::FIRST, non_ex_zone) + count_zones_card(domain::Player::SECOND, non_ex_zone);

		// 再累加额外怪兽区
		count += count_extra_monster_zones_card();

		return count;
	}

	namespace
	{
		template<std::same_as<PlayerZone::bag_zones_type> Zones>
		auto collect_bag_zones(const Zones& zones, std::vector<CardInstanceId>& out) noexcept -> void
		{
			// bag中所有实例都是合法的
			LIBASSERT_DEBUG_ASSERT(std::ranges::all_of(zones,&CardInstanceId::valid));

			// FIXME: 除外区的里侧卡牌是否需要?
			out.append_range(zones);
		}

		auto collect_field_zones(const PlayerZone::field_zones_type& field_zones, std::vector<CardInstanceId>& out) noexcept -> void
		{
			auto valid_zones = field_zones |
			                   // 要过滤掉非法区域
			                   std::views::filter(&CardInstanceId::valid);

			out.append_range(valid_zones);
		}

		auto collect_extra_monster_zones(const PlayerZone::field_zones_type& extra_monster_zones, std::vector<CardInstanceId>& out) noexcept -> void
		{
			for (const auto column: domain::extra_columns)
			{
				if (const auto card = extra_monster_zones[std::to_underlying(column)];
					card.valid())
				{
					out.push_back(card);
				}
			}
		}

		auto collect_extra_monster_zones(
			const Game& game,
			const PlayerZone::field_zones_type& extra_monster_zones,
			const domain::Player player,
			std::vector<CardInstanceId>& out
		) noexcept -> void
		{
			for (const auto column: domain::extra_columns)
			{
				if (const auto card = extra_monster_zones[std::to_underlying(column)];
					card.valid())
				{
					// 实例控制者必须是指定玩家
					if (const auto controller = game.controller(card);
						controller.has_value() && *controller == player)
					{
						out.push_back(card);
					}
				}
			}
		}
	}

	auto Field::collect_zones(const domain::Player player, const utility::EnumMask<domain::Zone> zone, std::vector<CardInstanceId>& out) const noexcept -> void
	{
		const auto& player_zone = player_zone_of(player);

		// 卡组
		if (zone.contains(domain::Zone::DECK))
		{
			collect_bag_zones(player_zone.deck_zones, out);
		}
		// 额外卡组
		if (zone.contains(domain::Zone::EXTRA_DECK))
		{
			collect_bag_zones(player_zone.extra_deck_zones, out);
		}
		// 手牌
		if (zone.contains(domain::Zone::HAND))
		{
			collect_bag_zones(player_zone.hand_zones, out);
		}
		// 墓地
		if (zone.contains(domain::Zone::GRAVEYARD))
		{
			collect_bag_zones(player_zone.graveyard_zones, out);
		}
		// 除外区
		if (zone.contains(domain::Zone::BANISHED))
		{
			collect_bag_zones(player_zone.banished_zones, out);
		}
		// 怪兽区
		if (zone.contains(domain::Zone::MONSTER))
		{
			collect_field_zones(player_zone.monster_zones, out);
		}
		// 额外怪兽区
		if (zone.contains(domain::Zone::EXTRA_MONSTER))
		{
			collect_extra_monster_zones(extra_monster_zones_, out);
		}
		// 魔陷区
		if (zone.contains(domain::Zone::SPELL_TRAP))
		{
			collect_field_zones(player_zone.spell_trap_zones, out);
		}
		// 场地魔法区
		if (zone.contains(domain::Zone::FIELD_SPELL))
		{
			if (player_zone.field_spell_zone.valid())
			{
				out.push_back(player_zone.field_spell_zone);
			}
		}
	}

	auto Field::collect_zones(const Game& game, const domain::Player player, const utility::EnumMask<domain::Zone> zone, std::vector<CardInstanceId>& out) const noexcept -> void
	{
		const auto& player_zone = player_zone_of(player);

		// 卡组
		if (zone.contains(domain::Zone::DECK))
		{
			collect_bag_zones(player_zone.deck_zones, out);
		}
		// 额外卡组
		if (zone.contains(domain::Zone::EXTRA_DECK))
		{
			collect_bag_zones(player_zone.extra_deck_zones, out);
		}
		// 手牌
		if (zone.contains(domain::Zone::HAND))
		{
			collect_bag_zones(player_zone.hand_zones, out);
		}
		// 墓地
		if (zone.contains(domain::Zone::GRAVEYARD))
		{
			collect_bag_zones(player_zone.graveyard_zones, out);
		}
		// 除外区
		if (zone.contains(domain::Zone::BANISHED))
		{
			collect_bag_zones(player_zone.banished_zones, out);
		}
		// 怪兽区
		if (zone.contains(domain::Zone::MONSTER))
		{
			collect_field_zones(player_zone.monster_zones, out);
		}
		// 额外怪兽区
		if (zone.contains(domain::Zone::EXTRA_MONSTER))
		{
			collect_extra_monster_zones(game, extra_monster_zones_, player, out);
		}
		// 魔陷区
		if (zone.contains(domain::Zone::SPELL_TRAP))
		{
			collect_field_zones(player_zone.spell_trap_zones, out);
		}
		// 场地魔法区
		if (zone.contains(domain::Zone::FIELD_SPELL))
		{
			if (player_zone.field_spell_zone.valid())
			{
				out.push_back(player_zone.field_spell_zone);
			}
		}
	}

	auto Field::collect_deck_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(player);

		const auto count = count_deck_zones_card(player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_bag_zones(player_zone.deck_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_extra_deck_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(player);

		const auto count = count_extra_deck_zones_card(player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_bag_zones(player_zone.extra_deck_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_hand_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(player);

		const auto count = count_hand_zones_card(player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_bag_zones(player_zone.hand_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_graveyard_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(player);

		const auto count = count_graveyard_zones_card(player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_bag_zones(player_zone.graveyard_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_banished_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(player);

		const auto count = count_banished_zones_card(player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_bag_zones(player_zone.banished_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_monster_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(player);

		const auto count = count_monster_zones_card(player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_field_zones(player_zone.monster_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_extra_monster_zones_card() const noexcept -> std::vector<CardInstanceId>
	{
		const auto count = count_extra_monster_zones_card();
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_extra_monster_zones(extra_monster_zones_, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_extra_monster_zones_card(const Game& game, const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		const auto count = count_extra_monster_zones_card(game, player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_extra_monster_zones(game, extra_monster_zones_, player, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_spell_trap_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		const auto& player_zone = player_zone_of(player);

		const auto count = count_spell_trap_zones_card(player);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_field_zones(player_zone.spell_trap_zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_field_spell_zones_card(const domain::Player player) const noexcept -> std::vector<CardInstanceId>
	{
		if (const auto& player_zone = player_zone_of(player);
			player_zone.field_spell_zone.valid())
		{
			return {player_zone.field_spell_zone};
		}

		return {};
	}

	auto Field::collect_zones_card(const domain::Player player, const utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>
	{
		const auto count = count_zones_card(player, zone);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_zones(player, zone, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_zones_card(const Game& game, const domain::Player player, const utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>
	{
		const auto count = count_zones_card(game, player, zone);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_zones(game, domain::Player::FIRST, zone, out);
		collect_zones(game, domain::Player::SECOND, zone, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_zones_card(const utility::EnumMask<domain::Zone> zone) const noexcept -> std::vector<CardInstanceId>
	{
		const auto count = count_zones_card(zone);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		// 如果不包含额外怪兽区则直接收集
		if (!zone.contains(domain::Zone::EXTRA_MONSTER))
		{
			collect_zones(domain::Player::FIRST, zone, out);
			collect_zones(domain::Player::SECOND, zone, out);
		}
		// 先收集非额外怪兽区,然后再收集额外怪兽区
		else
		{
			const auto non_ex_zone = zone.remove(domain::Zone::EXTRA_MONSTER);
			collect_zones(domain::Player::FIRST, non_ex_zone, out);
			collect_zones(domain::Player::SECOND, non_ex_zone, out);
			collect_extra_monster_zones(extra_monster_zones_, out);
		}

		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::is_location_empty([[maybe_unused]] const domain::Player player, const domain::ExtraMonsterLocation location) const noexcept -> bool
	{
		return is_location_empty(location);
	}

	auto Field::select_location_card([[maybe_unused]] const domain::Player player, const domain::ExtraMonsterLocation location) const noexcept -> CardInstanceId
	{
		return select_location_card(location);
	}

	auto Field::is_location_empty(const domain::Player player, const domain::MonsterLocation location) const noexcept -> bool
	{
		const auto& player_zone = player_zone_of(player);
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(location.column) < player_zone.monster_zones.size());

		return !player_zone.monster_zones[std::to_underlying(location.column)].valid();
	}

	auto Field::is_location_empty(const domain::ExtraMonsterLocation location) const noexcept -> bool
	{
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(location.column) < extra_monster_zones_.size());

		return !extra_monster_zones_[std::to_underlying(location.column)].valid();
	}

	auto Field::is_location_empty(const domain::Player player, const domain::SpellTrapLocation location) const noexcept -> bool
	{
		const auto& player_zone = player_zone_of(player);
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(location.column) < player_zone.spell_trap_zones.size());

		return !player_zone.spell_trap_zones[std::to_underlying(location.column)].valid();
	}

	auto Field::is_location_empty(const domain::Player player, [[maybe_unused]] const domain::FieldSpellLocation location) const noexcept -> bool
	{
		const auto& player_zone = player_zone_of(player);

		return !player_zone.field_spell_zone.valid();
	}

	auto Field::is_location_empty(const domain::Player player, const domain::Location location) const noexcept -> bool
	{
		return std::visit(
			utility::Overloaded
			{
					[&]<typename Location>(const Location& l) noexcept -> bool //
						requires(
							// 怪兽区
							std::is_same_v<Location, domain::MonsterLocation> ||
							// 额外怪兽区
							std::is_same_v<Location, domain::ExtraMonsterLocation> ||
							// 魔陷区
							std::is_same_v<Location, domain::SpellTrapLocation> ||
							// 场地魔法区
							std::is_same_v<Location, domain::FieldSpellLocation>
						)
					{
						return this->is_location_empty(player, l);
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> bool
					{
						return false;
					}
			},
			location
		);
	}

	auto Field::is_location_empty(const domain::Location location) const noexcept -> bool
	{
		return is_location_empty(domain::Player::FIRST, location) && is_location_empty(domain::Player::SECOND, location);
	}

	auto Field::select_location_card(const domain::Player player, const domain::DeckLocation location) const noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(player);
		LIBASSERT_DEBUG_ASSERT(location.index < player_zone.deck_zones.size());

		return player_zone.deck_zones[location.index];
	}

	auto Field::select_location_card(const domain::Player player, const domain::ExtraDeckLocation location) const noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(player);
		LIBASSERT_DEBUG_ASSERT(location.index < player_zone.extra_deck_zones.size());

		return player_zone.extra_deck_zones[location.index];
	}

	auto Field::select_location_card(const domain::Player player, const domain::HandLocation location) const noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(player);
		LIBASSERT_DEBUG_ASSERT(location.index < player_zone.hand_zones.size());

		return player_zone.hand_zones[location.index];
	}

	auto Field::select_location_card(const domain::Player player, const domain::GraveyardLocation location) const noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(player);
		LIBASSERT_DEBUG_ASSERT(location.index < player_zone.graveyard_zones.size());

		return player_zone.graveyard_zones[location.index];
	}

	auto Field::select_location_card(const domain::Player player, const domain::BanishedLocation location) const noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(player);
		LIBASSERT_DEBUG_ASSERT(location.index < player_zone.banished_zones.size());

		return player_zone.banished_zones[location.index];
	}

	auto Field::select_location_card(const domain::Player player, const domain::MonsterLocation location) const noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(player);
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(location.column) < player_zone.monster_zones.size());

		return player_zone.monster_zones[std::to_underlying(location.column)];
	}

	auto Field::select_location_card(const domain::ExtraMonsterLocation location) const noexcept -> CardInstanceId
	{
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(location.column) < extra_monster_zones_.size());

		return extra_monster_zones_[std::to_underlying(location.column)];
	}

	auto Field::select_location_card(const domain::Player player, const domain::SpellTrapLocation location) const noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(player);
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(location.column) < player_zone.spell_trap_zones.size());

		return player_zone.spell_trap_zones[std::to_underlying(location.column)];
	}

	auto Field::select_location_card(const domain::Player player, [[maybe_unused]] const domain::FieldSpellLocation location) const noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(player);

		return player_zone.field_spell_zone;
	}

	auto Field::select_location_card(const domain::Player player, const domain::Location location) const noexcept -> CardInstanceId
	{
		return std::visit(
			utility::Overloaded
			{
					[&]<typename Location>(const Location& l) noexcept -> CardInstanceId //
						requires(
							// 怪兽区
							std::is_same_v<Location, domain::MonsterLocation> ||
							// 额外怪兽区
							std::is_same_v<Location, domain::ExtraMonsterLocation> ||
							// 魔陷区
							std::is_same_v<Location, domain::SpellTrapLocation> ||
							// 场地魔法区
							std::is_same_v<Location, domain::FieldSpellLocation>
						)
					{
						return this->select_location_card(player, l);
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> CardInstanceId
					{
						return CardInstanceId{CardInstanceId::invalid_id};
					}
			},
			location
		);
	}
}

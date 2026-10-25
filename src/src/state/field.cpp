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

	auto Field::player_zone_of(const domain::Player player) noexcept -> PlayerZone&
	{
		LIBASSERT_DEBUG_ASSERT(std::to_underlying(player) < player_zones_.size());

		return player_zones_[std::to_underlying(player)];
	}

	Field::Field() noexcept
		: player_zones_{}
	// extra_monster_zones_{}
	{}

	auto Field::set_life_points(const domain::Player player, const std::underlying_type_t<domain::AttackDefense> value) noexcept -> void
	{
		auto& player_zone = player_zone_of(player);

		player_zone.life_points = value;
	}

	auto Field::life_points(const domain::Player player) const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		const auto& player_zone = player_zone_of(player);

		return player_zone.life_points;
	}

	auto Field::change_life_points(const domain::Player player, const std::underlying_type_t<domain::AttackDefense> delta) noexcept -> void
	{
		auto& player_zone = player_zone_of(player);

		player_zone.life_points += delta;
	}

	auto Field::shuffle_deck(const domain::Player player, std::mt19937_64& random) noexcept -> void
	{
		auto& player_zone = player_zone_of(player);

		std::ranges::shuffle(player_zone.deck_zones, random);
	}

	auto Field::zone_of(const CardInstanceId card) const noexcept -> domain::Zone
	{
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
				return domain::Zone::DECK;
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

	auto Field::is_zone_empty(const domain::Player player, const domain::Zone zone) const noexcept -> bool
	{
		const auto& player_zone = player_zone_of(player);

		switch (zone)
		{
			case domain::Zone::NOT_EXIST:
			{
				return true;
			}
			case domain::Zone::DECK:
			{
				return player_zone.deck_zones.empty();
			}
			case domain::Zone::EXTRA_DECK:
			{
				return player_zone.extra_deck_zones.empty();
			}
			case domain::Zone::HAND:
			{
				return player_zone.hand_zones.empty();
			}
			case domain::Zone::GRAVEYARD:
			{
				return player_zone.graveyard_zones.empty();
			}
			case domain::Zone::BANISHED:
			{
				return player_zone.banished_zones.empty();
			}
			case domain::Zone::MONSTER:
			{
				return std::ranges::none_of(player_zone.monster_zones, &CardInstanceId::valid);
			}
			case domain::Zone::EXTRA_MONSTER:
			{
				return std::ranges::none_of(extra_monster_zones_, &CardInstanceId::valid);
			}
			case domain::Zone::SPELL_TRAP:
			{
				return std::ranges::none_of(player_zone.spell_trap_zones, &CardInstanceId::valid);
			}
			case domain::Zone::FIELD_SPELL:
			{
				return !player_zone.field_spell_zone.valid();
			}
			default: // NOLINT(clang-diagnostic-covered-switch-default)
			{
				std::unreachable();
			}
		}
	}

	auto Field::is_zone_empty(const domain::Zone zone) const noexcept -> bool
	{
		return is_zone_empty(domain::Player::FIRST, zone) && is_zone_empty(domain::Player::SECOND, zone);
	}

	auto Field::is_zones_empty(const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> bool
	{
		return std::ranges::all_of(
			zones,
			// std::bind_front(&Field::is_zone_empty, player)
			[this, player](const domain::Zone z) noexcept -> bool
			{
				return is_zone_empty(player, z);
			}
		);
	}

	auto Field::is_zones_empty(const utility::EnumMask<domain::Zone> zone) const noexcept -> bool
	{
		return is_zones_empty(domain::Player::FIRST, zone) && is_zones_empty(domain::Player::SECOND, zone);
	}

	auto Field::count_zone_empty_card(const domain::Player player, const domain::Zone zone) const noexcept -> size_type
	{
		const auto& player_zone = player_zone_of(player);

		switch (zone)
		{
			case domain::Zone::NOT_EXIST:
			case domain::Zone::DECK:
			case domain::Zone::EXTRA_DECK:
			case domain::Zone::HAND:
			case domain::Zone::GRAVEYARD:
			case domain::Zone::BANISHED:
			{
				return 0;
			}
			case domain::Zone::MONSTER:
			{
				const auto size = std::ranges::count_if(player_zone.monster_zones, &CardInstanceId::valid);
				LIBASSERT_DEBUG_ASSERT(size <= domain::total_field_columns);

				return static_cast<size_type>(domain::total_field_columns - size);
			}
			case domain::Zone::EXTRA_MONSTER:
			{
				const auto size = std::ranges::count_if(extra_monster_zones_, &CardInstanceId::valid);
				LIBASSERT_DEBUG_ASSERT(size <= domain::total_extra_columns);

				return static_cast<size_type>(domain::total_extra_columns - size);
			}
			case domain::Zone::SPELL_TRAP:
			{
				const auto size = std::ranges::count_if(player_zone.spell_trap_zones, &CardInstanceId::valid);
				LIBASSERT_DEBUG_ASSERT(size <= domain::total_field_columns);

				return static_cast<size_type>(domain::total_field_columns - size);
			}
			case domain::Zone::FIELD_SPELL:
			{
				return player_zone.field_spell_zone.valid() ? 0 : 1;
			}
			default: // NOLINT(clang-diagnostic-covered-switch-default)
			{
				std::unreachable();
			}
		}
	}

	auto Field::count_zones_empty_card(const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> size_type
	{
		const auto size = std::ranges::fold_left(
			zones | std::views::transform(
				[this, player](const domain::Zone z) noexcept -> size_type
				{
					return count_zone_empty_card(player, z);
				}
			),
			std::size_t{0},
			std::plus{}
		);

		LIBASSERT_DEBUG_ASSERT(size <= std::numeric_limits<size_type>::max());
		return static_cast<size_type>(size);
	}

	auto Field::count_zone_card(const domain::Player player, const domain::Zone zone) const noexcept -> size_type
	{
		const auto& player_zone = player_zone_of(player);

		switch (zone)
		{
			case domain::Zone::NOT_EXIST:
			{
				return 0;
			}
			case domain::Zone::DECK:
			{
				const auto size = player_zone.deck_zones.size();
				LIBASSERT_DEBUG_ASSERT(size < std::numeric_limits<size_type>::max());

				return static_cast<size_type>(size);
			}
			case domain::Zone::EXTRA_DECK:
			{
				const auto size = player_zone.extra_deck_zones.size();
				LIBASSERT_DEBUG_ASSERT(size < std::numeric_limits<size_type>::max());

				return static_cast<size_type>(size);
			}
			case domain::Zone::HAND:
			{
				const auto size = player_zone.hand_zones.size();
				LIBASSERT_DEBUG_ASSERT(size < std::numeric_limits<size_type>::max());

				return static_cast<size_type>(size);
			}
			case domain::Zone::GRAVEYARD:
			{
				const auto size = player_zone.graveyard_zones.size();
				LIBASSERT_DEBUG_ASSERT(size < std::numeric_limits<size_type>::max());

				return static_cast<size_type>(size);
			}
			case domain::Zone::BANISHED:
			{
				const auto size = player_zone.banished_zones.size();
				LIBASSERT_DEBUG_ASSERT(size < std::numeric_limits<size_type>::max());

				return static_cast<size_type>(size);
			}
			case domain::Zone::MONSTER:
			{
				const auto size = std::ranges::count_if(player_zone.monster_zones, &CardInstanceId::valid);
				LIBASSERT_DEBUG_ASSERT(size <= domain::total_field_columns);

				return static_cast<size_type>(size);
			}
			case domain::Zone::EXTRA_MONSTER:
			{
				const auto size = std::ranges::count_if(extra_monster_zones_, &CardInstanceId::valid);
				LIBASSERT_DEBUG_ASSERT(size <= domain::total_extra_columns);

				return static_cast<size_type>(size);
			}
			case domain::Zone::SPELL_TRAP:
			{
				const auto size = std::ranges::count_if(player_zone.spell_trap_zones, &CardInstanceId::valid);
				LIBASSERT_DEBUG_ASSERT(size <= domain::total_field_columns);

				return static_cast<size_type>(size);
			}
			case domain::Zone::FIELD_SPELL:
			{
				return player_zone.field_spell_zone.valid() ? 1 : 0;
			}
			default: // NOLINT(clang-diagnostic-covered-switch-default)
			{
				std::unreachable();
			}
		}
	}

	auto Field::count_zone_card(const Game& game, const domain::Player player, const domain::Zone zone) const noexcept -> size_type
	{
		if (zone == domain::Zone::EXTRA_MONSTER)
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

					// 实例必须存在
					const auto* instance = game.find_instance(id);
					if (instance == nullptr)
					{
						return false;
					}

					// 实例控制者必须是指定玩家
					const auto controller = instance->controller();
					return controller == player;
				}
			);
			LIBASSERT_DEBUG_ASSERT(size <= domain::total_extra_columns);

			return static_cast<size_type>(size);
		}

		return count_zone_card(player, zone);
	}

	auto Field::count_zones_card(const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> size_type
	{
		const auto size = std::ranges::fold_left(
			zones | std::views::transform(
				[this, player](const domain::Zone z) noexcept -> size_type
				{
					return count_zone_card(player, z);
				}
			),
			std::size_t{0},
			std::plus{}
		);

		LIBASSERT_DEBUG_ASSERT(size <= std::numeric_limits<size_type>::max());
		return static_cast<size_type>(size);
	}

	auto Field::count_zones_card(const Game& game, const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> size_type
	{
		const auto size = std::ranges::fold_left(
			zones | std::views::transform(
				[this, &game, player](const domain::Zone z) noexcept -> size_type
				{
					return count_zone_card(game, player, z);
				}
			),
			std::size_t{0},
			std::plus{}
		);

		LIBASSERT_DEBUG_ASSERT(size <= std::numeric_limits<size_type>::max());
		return static_cast<size_type>(size);
	}

	auto Field::count_zones_card(const utility::EnumMask<domain::Zone> zones) const noexcept -> size_type
	{
		// 如果不包含额外怪兽区则直接统计
		if (!zones.contains(domain::Zone::EXTRA_MONSTER))
		{
			return count_zones_card(domain::Player::FIRST, zones) + count_zones_card(domain::Player::SECOND, zones);
		}

		// 先统计非额外怪兽区
		const auto non_ex_zone = zones.remove(domain::Zone::EXTRA_MONSTER);
		auto size = count_zones_card(domain::Player::FIRST, non_ex_zone) + count_zones_card(domain::Player::SECOND, non_ex_zone);

		// 再累加额外怪兽区
		size += count_zone_card(domain::Player::FIRST, domain::Zone::EXTRA_MONSTER);

		LIBASSERT_DEBUG_ASSERT(size <= std::numeric_limits<size_type>::max());
		return static_cast<size_type>(size);
	}

	auto Field::collect_empty_zone(const domain::Player player, const domain::Zone zone, std::vector<domain::Location>& out) const noexcept -> void
	{
		const auto& player_zone = player_zone_of(player);

		switch (zone)
		{
			case domain::Zone::NOT_EXIST:
			case domain::Zone::DECK:
			case domain::Zone::EXTRA_DECK:
			case domain::Zone::HAND:
			case domain::Zone::GRAVEYARD:
			case domain::Zone::BANISHED:
			{
				break;
			}
			case domain::Zone::MONSTER:
			{
				for (const auto column: domain::field_columns)
				{
					if (const auto card = player_zone.monster_zones[std::to_underlying(column)];
						!card.valid())
					{
						out.emplace_back(domain::MonsterLocation{.column = column});
					}
				}
				break;
			}
			case domain::Zone::EXTRA_MONSTER:
			{
				for (const auto column: domain::extra_columns)
				{
					if (const auto card = extra_monster_zones_[std::to_underlying(column)];
						!card.valid())
					{
						out.emplace_back(domain::ExtraMonsterLocation{.column = column});
					}
				}
				break;
			}
			case domain::Zone::SPELL_TRAP:
			{
				for (const auto column: domain::field_columns)
				{
					if (const auto card = player_zone.spell_trap_zones[std::to_underlying(column)];
						!card.valid())
					{
						out.emplace_back(domain::SpellTrapLocation{.column = column});
					}
				}
				break;
			}
			case domain::Zone::FIELD_SPELL:
			{
				if (const auto card = player_zone.field_spell_zone;
					!card.valid())
				{
					out.emplace_back(domain::FieldSpellLocation{});
				}
				break;
			}
			default: // NOLINT(clang-diagnostic-covered-switch-default)
			{
				std::unreachable();
			}
		}
	}

	auto Field::collect_empty_zone(const domain::Player player, const domain::Zone zone) const noexcept -> std::vector<domain::Location>
	{
		const auto count = count_zone_empty_card(player, zone);
		std::vector<domain::Location> out{};
		out.reserve(count);

		collect_empty_zone(player, zone, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_empty_zones(const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<domain::Location>
	{
		const auto count = count_zones_empty_card(player, zones);
		std::vector<domain::Location> out{};
		out.reserve(count);

		std::ranges::for_each(
			zones,
			[this, &out, player](const domain::Zone z) noexcept -> void
			{
				collect_empty_zone(player, z, out);
			}
		);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_monster_empty_zone(const domain::Player player) const noexcept -> std::vector<domain::MonsterLocation>
	{
		const auto count = count_zone_empty_card(player, domain::Zone::MONSTER);
		std::vector<domain::MonsterLocation> out{};
		out.reserve(count);

		for (const auto& player_zone = player_zone_of(player);
		     const auto column: domain::field_columns)
		{
			if (const auto card = player_zone.monster_zones[std::to_underlying(column)];
				!card.valid())
			{
				out.emplace_back(column);
			}
		}
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_extra_monster_empty_zone(const domain::Player player) const noexcept -> std::vector<domain::ExtraMonsterLocation>
	{
		const auto count = count_zone_empty_card(player, domain::Zone::EXTRA_MONSTER);
		std::vector<domain::ExtraMonsterLocation> out{};
		out.reserve(count);

		for (const auto column: domain::extra_columns)
		{
			if (const auto card = extra_monster_zones_[std::to_underlying(column)];
				!card.valid())
			{
				out.emplace_back(column);
			}
		}
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_any_monster_empty_zone(const domain::Player player) const noexcept -> std::vector<domain::AnyMonsterLocation>
	{
		const auto count = count_zones_empty_card(player, {domain::Zone::MONSTER, domain::Zone::EXTRA_MONSTER});
		std::vector<domain::AnyMonsterLocation> out{};
		out.reserve(count);

		for (const auto& player_zone = player_zone_of(player);
		     const auto column: domain::field_columns)
		{
			if (const auto card = player_zone.monster_zones[std::to_underlying(column)];
				!card.valid())
			{
				out.emplace_back(domain::MonsterLocation{.column = column});
			}
		}
		for (const auto column: domain::extra_columns)
		{
			if (const auto card = extra_monster_zones_[std::to_underlying(column)];
				!card.valid())
			{
				out.emplace_back(domain::ExtraMonsterLocation{.column = column});
			}
		}
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_spell_trap_empty_zone(const domain::Player player) const noexcept -> std::vector<domain::SpellTrapLocation>
	{
		const auto count = count_zone_empty_card(player, domain::Zone::SPELL_TRAP);
		std::vector<domain::SpellTrapLocation> out{};
		out.reserve(count);

		for (const auto& player_zone = player_zone_of(player);
		     const auto column: domain::field_columns)
		{
			if (const auto card = player_zone.spell_trap_zones[std::to_underlying(column)];
				!card.valid())
			{
				out.emplace_back(column);
			}
		}
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_zone_card(const domain::Player player, const domain::Zone zone, std::vector<CardInstanceId>& out) const noexcept -> void
	{
		const auto& player_zone = player_zone_of(player);

		switch (zone)
		{
			case domain::Zone::NOT_EXIST:
			{
				break;
			}
			case domain::Zone::DECK:
			{
				out.append_range(player_zone.deck_zones);
				break;
			}
			case domain::Zone::EXTRA_DECK:
			{
				out.append_range(player_zone.extra_deck_zones);
				break;
			}
			case domain::Zone::HAND:
			{
				out.append_range(player_zone.hand_zones);
				break;
			}
			case domain::Zone::GRAVEYARD:
			{
				out.append_range(player_zone.graveyard_zones);
				break;
			}
			case domain::Zone::BANISHED:
			{
				// FIXME: 除外区的里侧卡牌是否需要?
				out.append_range(player_zone.banished_zones);
				break;
			}
			case domain::Zone::MONSTER:
			{
				// 要过滤掉无效区域
				auto valid_zones = player_zone.monster_zones | std::views::filter(&CardInstanceId::valid);

				out.append_range(valid_zones);
				break;
			}
			case domain::Zone::EXTRA_MONSTER:
			{
				for (const auto column: domain::extra_columns)
				{
					if (const auto card = extra_monster_zones_[std::to_underlying(column)];
						card.valid())
					{
						out.push_back(card);
					}
				}
				break;
			}
			case domain::Zone::SPELL_TRAP:
			{
				// 要过滤掉无效区域
				auto valid_zones = player_zone.spell_trap_zones | std::views::filter(&CardInstanceId::valid);

				out.append_range(valid_zones);
				break;
			}
			case domain::Zone::FIELD_SPELL:
			{
				if (player_zone.field_spell_zone.valid())
				{
					out.push_back(player_zone.field_spell_zone);
				}
				break;
			}
			default: // NOLINT(clang-diagnostic-covered-switch-default)
			{
				std::unreachable();
			}
		}
	}

	auto Field::collect_zone_card(const Game& game, const domain::Player player, const domain::Zone zone, std::vector<CardInstanceId>& out) const noexcept -> void
	{
		if (zone == domain::Zone::EXTRA_MONSTER)
		{
			for (const auto column: domain::extra_columns)
			{
				if (const auto card = extra_monster_zones_[std::to_underlying(column)];
					card.valid())
				{
					// 实例必须存在
					const auto* instance = game.find_instance(card);
					if (instance == nullptr)
					{
						continue;
					}

					// 实例控制者必须是指定玩家
					if (const auto controller = instance->controller();
						controller == player)
					{
						out.push_back(card);
					}
				}
			}

			return;
		}

		collect_zone_card(player, zone, out);
	}

	auto Field::collect_zones_card(const domain::Player player, const utility::EnumMask<domain::Zone> zones, std::vector<CardInstanceId>& out) const noexcept -> void
	{
		std::ranges::for_each(
			zones,
			[this, &out, player](const domain::Zone z) noexcept -> void
			{
				collect_zone_card(player, z, out);
			}
		);
	}

	auto Field::collect_zones_card(const Game& game, const domain::Player player, const utility::EnumMask<domain::Zone> zones, std::vector<CardInstanceId>& out) const noexcept -> void
	{
		std::ranges::for_each(
			zones,
			[this, &game, &out, player](const domain::Zone z) noexcept -> void
			{
				collect_zone_card(game, player, z, out);
			}
		);
	}

	auto Field::collect_zone_card(const domain::Player player, const domain::Zone zone) const noexcept -> std::vector<CardInstanceId>
	{
		const auto count = count_zone_card(player, zone);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_zone_card(player, zone, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_zone_card(const Game& game, const domain::Player player, const domain::Zone zone) const noexcept -> std::vector<CardInstanceId>
	{
		const auto count = count_zone_card(game, player, zone);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_zone_card(game, player, zone, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_zones_card(const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<CardInstanceId>
	{
		const auto count = count_zones_card(player, zones);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_zones_card(player, zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_zones_card(const Game& game, const domain::Player player, const utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<CardInstanceId>
	{
		const auto count = count_zones_card(game, player, zones);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		collect_zones_card(game, player, zones, out);
		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::collect_zones_card(const utility::EnumMask<domain::Zone> zones) const noexcept -> std::vector<CardInstanceId>
	{
		const auto count = count_zones_card(zones);
		std::vector<CardInstanceId> out{};
		out.reserve(count);

		// 如果不包含额外怪兽区则直接收集
		if (!zones.contains(domain::Zone::EXTRA_MONSTER))
		{
			collect_zones_card(domain::Player::FIRST, zones, out);
			collect_zones_card(domain::Player::SECOND, zones, out);
		}
		// 先收集非额外怪兽区,然后再收集额外怪兽区
		else
		{
			const auto non_ex_zone = zones.remove(domain::Zone::EXTRA_MONSTER);
			collect_zones_card(domain::Player::FIRST, non_ex_zone, out);
			collect_zones_card(domain::Player::SECOND, non_ex_zone, out);
			collect_zone_card(domain::Player::FIRST, domain::Zone::EXTRA_MONSTER, out);
		}

		LIBASSERT_DEBUG_ASSERT(out.size() == count);

		return out;
	}

	auto Field::is_location_empty(const domain::Player player, const domain::Location location) const noexcept -> bool
	{
		const auto card = select_location_card(player, location);
		return !card.valid();
	}

	// auto Field::is_location_empty(const domain::Player player, const domain::AnyFieldLocation location) const noexcept -> bool
	// {
	// 	const auto card = select_location_card(player, location);
	// 	return !card.valid();
	// }

	auto Field::is_location_empty(const domain::Location location) const noexcept -> bool
	{
		return is_location_empty(domain::Player::FIRST, location) && is_location_empty(domain::Player::SECOND, location);
	}

	// auto Field::is_location_empty(const domain::AnyFieldLocation location) const noexcept -> bool
	// {
	// 	return is_location_empty(domain::Player::FIRST, location) && is_location_empty(domain::Player::SECOND, location);
	// }

	auto Field::select_location_card(const domain::Player player, const domain::Location location) const noexcept -> CardInstanceId
	{
		return std::visit(
			utility::Overloaded
			{
					[&]<typename L>(const L& l) noexcept -> CardInstanceId //
						requires(utility::variant_index_v<L, domain::AnyFieldLocation> != utility::invalid_variant_index)
					{
						return this->select_location_card(player, domain::AnyFieldLocation{l});
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> CardInstanceId
					{
						return CardInstanceId{CardInstanceId::invalid_id};
					},
			},
			location
		);
	}

	auto Field::select_location_card(const domain::Player player, const domain::AnyFieldLocation location) const noexcept -> CardInstanceId
	{
		const auto& player_zone = player_zone_of(player);

		return std::visit(
			utility::Overloaded
			{
					[&](const domain::MonsterLocation& l) noexcept -> CardInstanceId
					{
						LIBASSERT_DEBUG_ASSERT(std::to_underlying(l.column) < player_zone.monster_zones.size());
						const auto card = player_zone.monster_zones[std::to_underlying(l.column)];

						return card;
					},
					[&](const domain::ExtraMonsterLocation& l) noexcept -> CardInstanceId
					{
						LIBASSERT_DEBUG_ASSERT(std::to_underlying(l.column) < extra_monster_zones_.size());
						const auto card = extra_monster_zones_[std::to_underlying(l.column)];

						return card;
					},
					[&](const domain::SpellTrapLocation& l) noexcept -> CardInstanceId
					{
						LIBASSERT_DEBUG_ASSERT(std::to_underlying(l.column) < player_zone.spell_trap_zones.size());
						const auto card = player_zone.spell_trap_zones[std::to_underlying(l.column)];

						return card;
					},
					[&]([[maybe_unused]] const domain::FieldSpellLocation& l) noexcept -> CardInstanceId
					{
						const auto card = player_zone.field_spell_zone;

						return card;
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> CardInstanceId
					{
						return CardInstanceId{CardInstanceId::invalid_id};
					},
			},
			location
		);
	}

	auto Field::place(const domain::Player player, const CardInstanceId card, const domain::Location location) noexcept -> bool
	{
		return std::visit(
			utility::Overloaded
			{
					[&]<typename L>(const L& l) noexcept -> bool //
						requires (utility::variant_index_v<L, domain::AnyBagLocation> != utility::invalid_variant_index)
					{
						this->place(player, card, domain::AnyBagLocation{l}, domain::BagPlacement::BOTTOM);
						return true;
					},
					[&]<typename L>(const L& l) noexcept -> bool //
						requires(utility::variant_index_v<L, domain::AnyFieldLocation> != utility::invalid_variant_index)
					{
						return this->place(player, card, domain::AnyFieldLocation{l});
					},
					[](const domain::NotExistLocation&) noexcept -> bool
					{
						return false;
					},
			},
			location
		);
	}

	namespace
	{
		auto place_bag_zone(PlayerZone::bag_zones_type& zones, const CardInstanceId card, const domain::BagPlacement bag) noexcept -> void
		{
			if (bag == domain::BagPlacement::BOTTOM)
			{
				zones.push_back(card);
			}
			else
			{
				zones.insert(zones.begin(), card);
			}
		}
	}

	auto Field::place(const domain::Player player, const CardInstanceId card, const domain::AnyBagLocation location, const domain::BagPlacement bag) noexcept -> void
	{
		auto& player_zone = player_zone_of(player);

		return std::visit(
			utility::Overloaded
			{
					[&]([[maybe_unused]] const domain::DeckLocation& l) noexcept -> void
					{
						place_bag_zone(player_zone.deck_zones, card, bag);
					},
					[&]([[maybe_unused]] const domain::ExtraDeckLocation& l) noexcept -> void
					{
						place_bag_zone(player_zone.extra_deck_zones, card, bag);
					},
					[&]([[maybe_unused]] const domain::HandLocation& l) noexcept -> void
					{
						place_bag_zone(player_zone.hand_zones, card, bag);
					},
					[&]([[maybe_unused]] const domain::GraveyardLocation& l) noexcept -> void
					{
						place_bag_zone(player_zone.graveyard_zones, card, bag);
					},
					[&]([[maybe_unused]] const domain::BanishedLocation& l) noexcept -> void
					{
						place_bag_zone(player_zone.banished_zones, card, bag);
					},
			},
			location
		);
	}

	auto Field::place(const domain::Player player, const CardInstanceId card, const domain::AnyFieldLocation location) noexcept -> bool
	{
		auto& player_zone = player_zone_of(player);

		return std::visit(
			utility::Overloaded
			{
					[&](const domain::MonsterLocation& l) noexcept -> bool
					{
						LIBASSERT_DEBUG_ASSERT(std::to_underlying(l.column) < player_zone.monster_zones.size());

						const auto prev = std::exchange(player_zone.monster_zones[std::to_underlying(l.column)], card);
						return prev.valid();
					},
					[&](const domain::ExtraMonsterLocation& l) noexcept -> bool
					{
						LIBASSERT_DEBUG_ASSERT(std::to_underlying(l.column) < extra_monster_zones_.size());

						const auto prev = std::exchange(extra_monster_zones_[std::to_underlying(l.column)], card);
						return prev.valid();
					},
					[&](const domain::SpellTrapLocation& l) noexcept -> bool
					{
						LIBASSERT_DEBUG_ASSERT(std::to_underlying(l.column) < player_zone.spell_trap_zones.size());

						const auto prev = std::exchange(player_zone.spell_trap_zones[std::to_underlying(l.column)], card);
						return prev.valid();
					},
					[&]([[maybe_unused]] const domain::FieldSpellLocation& l) noexcept -> bool
					{
						const auto prev = std::exchange(player_zone.field_spell_zone, card);
						return prev.valid();
					},
			},
			location
		);
	}

	namespace
	{
		[[nodiscard]] auto erase_bag_zones(PlayerZone::bag_zones_type& zones, const CardInstanceId card) noexcept -> bool
		{
			const auto it = std::ranges::find(zones, card);
			if (it == zones.end())
			{
				return false;
			}

			zones.erase(it);
			return true;
		}

		[[nodiscard]] auto erase_field_zones(PlayerZone::field_zones_type& zones, const domain::FieldColumn column) noexcept -> bool
		{
			constexpr CardInstanceId empty{CardInstanceId::invalid_id};

			LIBASSERT_DEBUG_ASSERT(std::to_underlying(column) < zones.size());

			const auto prev = std::exchange(zones[std::to_underlying(column)], empty);
			return prev.valid();
		}
	}

	auto Field::remove(const domain::Player player, const CardInstanceId card, const domain::Location location) noexcept -> bool
	{
		return std::visit(
			utility::Overloaded
			{
					[&]<typename L>(const L& l) noexcept -> bool //
						requires (utility::variant_index_v<L, domain::AnyBagLocation> != utility::invalid_variant_index)
					{
						return this->remove(player, card, domain::AnyBagLocation{l});
					},
					[&]<typename L>(const L& l) noexcept -> bool //
						requires(utility::variant_index_v<L, domain::AnyFieldLocation> != utility::invalid_variant_index)
					{
						return this->remove(player, domain::AnyFieldLocation{l});
					},
					[](const domain::NotExistLocation&) noexcept -> bool
					{
						return false;
					},
			},
			location
		);
	}

	auto Field::remove(const domain::Player player, const CardInstanceId card, const domain::AnyBagLocation location) noexcept -> bool
	{
		auto& player_zone = player_zone_of(player);

		return std::visit(
			utility::Overloaded
			{
					[&]([[maybe_unused]] const domain::DeckLocation& l) noexcept -> bool
					{
						return erase_bag_zones(player_zone.deck_zones, card);
					},
					[&]([[maybe_unused]] const domain::ExtraDeckLocation& l) noexcept -> bool
					{
						return erase_bag_zones(player_zone.extra_deck_zones, card);
					},
					[&]([[maybe_unused]] const domain::HandLocation& l) noexcept -> bool
					{
						return erase_bag_zones(player_zone.hand_zones, card);
					},
					[&]([[maybe_unused]] const domain::GraveyardLocation& l) noexcept -> bool
					{
						return erase_bag_zones(player_zone.graveyard_zones, card);
					},
					[&]([[maybe_unused]] const domain::BanishedLocation& l) noexcept -> bool
					{
						return erase_bag_zones(player_zone.banished_zones, card);
					},
			},
			location
		);
	}

	auto Field::remove(const domain::Player player, const domain::AnyFieldLocation location) noexcept -> bool
	{
		auto& player_zone = player_zone_of(player);

		return std::visit(
			utility::Overloaded
			{
					[&](const domain::MonsterLocation& l) noexcept -> bool
					{
						return erase_field_zones(player_zone.monster_zones, l.column);
					},
					[&](const domain::ExtraMonsterLocation& l) noexcept -> bool
					{
						return erase_field_zones(extra_monster_zones_, l.column);
					},
					[&](const domain::SpellTrapLocation& l) noexcept -> bool
					{
						return erase_field_zones(player_zone.spell_trap_zones, l.column);
					},
					[&]([[maybe_unused]] const domain::FieldSpellLocation& l) noexcept -> bool
					{
						const auto prev = std::exchange(player_zone.field_spell_zone, CardInstanceId{CardInstanceId::invalid_id});
						return prev.valid();
					},
			},
			location
		);
	}

	auto Field::place(const domain::Player player, const CardInstanceId card, const domain::Placement placement) noexcept -> bool
	{
		const auto location = domain::direct_location_of(placement);
		return place(player, card, location);
	}

	auto Field::place(const domain::Player player, const CardInstanceId card, const domain::AnyBagPlacement placement, const domain::BagPlacement bag) noexcept -> void
	{
		const auto location = domain::direct_location_of(placement);
		place(player, card, location, bag);
	}

	auto Field::place(const domain::Player player, const CardInstanceId card, const domain::AnyFieldPlacement placement) noexcept -> bool
	{
		const auto location = domain::direct_location_of(placement);
		return place(player, card, location);
	}

	auto Field::remove(const domain::Player player, const CardInstanceId card, const domain::Placement placement) noexcept -> bool
	{
		const auto location = domain::direct_location_of(placement);
		return remove(player, card, location);
	}

	auto Field::remove(const domain::Player player, const CardInstanceId card, const domain::AnyBagPlacement placement) noexcept -> bool
	{
		const auto location = domain::direct_location_of(placement);
		return remove(player, card, location);
	}

	auto Field::remove(const domain::Player player, const domain::AnyFieldPlacement placement) noexcept -> bool
	{
		const auto location = domain::direct_location_of(placement);
		return remove(player, location);
	}
}

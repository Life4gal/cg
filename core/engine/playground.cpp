#include <core/engine/playground.hpp>

#include <core/utility/functional.hpp>
#include <core/engine/card.hpp>

namespace cg::engine
{
	namespace
	{
		[[nodiscard]] constexpr auto default_player_field() noexcept -> PlayerField
		{
			return
			{
					.life_point = 8000,
					.start_hand = 5,
					.draw_count = 1,
					.deck = {},
					.extra_deck = {},
					.hand = {},
					.graveyard = {},
					.removed = {},
					.monster = {},
					.spell_trap = {},
			};
		}

		// 重新计算指定区域所有卡的区域位置序号
		template<typename T>
		auto recalculate_zone(Sequence& sequence) noexcept -> void
		{
			for (auto begin = sequence.begin(), it = begin; it != sequence.end(); ++it)
			{
				auto card_state = it->get().state();

				const auto zone_index = std::ranges::distance(begin, it);
				card_state.set_zone(T{.index = static_cast<domain::Zone::size_type>(zone_index)});
			}
		}

		// 重新计算指定区域所有卡的区域位置序号
		template<domain::AutoZone Z>
		auto recalculate_zone(Sequence& sequence) noexcept -> void
		{
			using type = domain::zone_of_t<Z>;

			recalculate_zone<type>(sequence);
		}
	}

	Playground::Playground() noexcept
		: player_fields{default_player_field(), default_player_field()},
		  shared_extra_monster_field{nullptr, nullptr} {}

	auto Playground::field_of(const domain::Player player) noexcept -> PlayerField&
	{
		return player_fields[std::to_underlying(player)];
	}

	auto Playground::field_of(const domain::Player player) const noexcept -> const PlayerField&
	{
		return player_fields[std::to_underlying(player)];
	}

	auto Playground::set_life_point(const domain::Player player, const domain::life_point_type life_point) noexcept -> void
	{
		field_of(player).life_point = life_point;
	}

	auto Playground::set_start_hand(const domain::Player player, const size_type start_hand) noexcept -> void
	{
		field_of(player).start_hand = start_hand;
	}

	auto Playground::set_draw_count(const domain::Player player, const size_type draw_count) noexcept -> void
	{
		field_of(player).draw_count = draw_count;
	}

	auto Playground::life_point(const domain::Player player) const noexcept -> domain::life_point_type
	{
		return field_of(player).life_point;
	}

	auto Playground::start_hand(const domain::Player player) const noexcept -> size_type
	{
		return field_of(player).start_hand;
	}

	auto Playground::draw_count(const domain::Player player) const noexcept -> size_type
	{
		return field_of(player).draw_count;
	}

	auto Playground::affordable(const domain::Player player, const domain::life_point_type delta) const noexcept -> bool
	{
		auto& field = field_of(player);

		if (delta >= 0)
		{
			// 增加/不变生命值
			return true;
		}

		return std::cmp_greater(field.life_point, delta);
	}

	auto Playground::afford(const domain::Player player, const domain::life_point_type delta) noexcept -> void
	{
		auto& field = field_of(player);

		// 增加生命值
		if (delta > 0)
		{
			if (const auto remaining = std::numeric_limits<domain::life_point_type>::max() - field.life_point; std::cmp_greater(delta, remaining))
			{
				// 超出最大值
				field.life_point = std::numeric_limits<domain::life_point_type>::max();
			}
			else
			{
				field.life_point += delta;
			}
		}
		else
		{
			// 减少生命值
			field.life_point += delta;
			// 不会减少到负数
			field.life_point = std::ranges::max(field.life_point, static_cast<domain::life_point_type>(0));
		}
	}

	auto Playground::start_draw(const domain::Player player) noexcept -> std::span<CardReference>
	{
		return draw(player, start_hand(player));
	}

	auto Playground::draw(const domain::Player player, const size_type count) noexcept -> std::span<CardReference>
	{
		auto& field = field_of(player);

		if (field.deck.size() < count)
		{
			// 不够牌抽了 -- 虽然不够抽,但是我们也应该知道最后抽到的是什么卡?
			return {};
		}

		// 当前手牌数量
		const auto hand_size = field.hand.size();

		for (size_type i = 0; i < count; ++i)
		{
			// 从卡组拿取
			auto card = field.deck.back();
			field.deck.pop_back();
			// 加入手牌
			add(player, card, domain::AutoZone::HAND);

			// 设置卡牌移动原因
			auto card_state = card.get().state();
			card_state.set_reason({domain::Reason::DRAW});
		}

		// 返回抽到的卡
		const auto begin = field.hand.begin() + static_cast<std::ptrdiff_t>(hand_size);
		return {begin, begin + count};
	}

	auto Playground::add(const domain::Player player, Card& card, const domain::AutoZone zone) noexcept -> void
	{
		auto& field = field_of(player);
		const auto do_add = [player, &card]<domain::AutoZone Z>(Sequence& sequence) noexcept -> void
		{
			using type = domain::zone_of_t<Z>;

			auto card_state = card.state();
			const auto index = sequence.size();

			card_state.set_controller(player);
			card_state.set_zone(type{.index = static_cast<domain::Zone::size_type>(index)});

			sequence.push_back(card);
		};

		switch (zone)
		{
			case domain::AutoZone::DECK:
			{
				do_add.operator()<domain::AutoZone::DECK>(field.deck);
				break;
			}
			case domain::AutoZone::EXTRA_DECK:
			{
				do_add.operator()<domain::AutoZone::EXTRA_DECK>(field.extra_deck);
				break;
			}
			case domain::AutoZone::HAND:
			{
				do_add.operator()<domain::AutoZone::HAND>(field.hand);
				break;
			}
			case domain::AutoZone::GRAVEYARD:
			{
				do_add.operator()<domain::AutoZone::GRAVEYARD>(field.graveyard);
				break;
			}
			case domain::AutoZone::REMOVED:
			{
				do_add.operator()<domain::AutoZone::REMOVED>(field.removed);
				break;
			}
			default: // NOLINT(clang-diagnostic-covered-switch-default)
			{
				std::unreachable();
			}
		}
	}

	auto Playground::shuffle(const domain::Player player, const domain::AutoZone zone, utility::Random& random) noexcept -> void
	{
		auto& field = field_of(player);
		const auto do_shuffle = [&random]<domain::AutoZone Z>(Sequence& sequence) noexcept -> void
		{
			// 打乱指定区域卡牌
			std::ranges::shuffle(sequence, random);
			// 更新位置序号
			recalculate_zone<Z>(sequence);
		};

		switch (zone)
		{
			case domain::AutoZone::DECK:
			{
				do_shuffle.operator()<domain::AutoZone::DECK>(field.deck);
				break;
			}
			case domain::AutoZone::EXTRA_DECK:
			{
				do_shuffle.operator()<domain::AutoZone::EXTRA_DECK>(field.extra_deck);
				break;
			}
			case domain::AutoZone::HAND:
			{
				do_shuffle.operator()<domain::AutoZone::HAND>(field.hand);
				break;
			}
			case domain::AutoZone::GRAVEYARD:
			{
				do_shuffle.operator()<domain::AutoZone::GRAVEYARD>(field.graveyard);
				break;
			}
			case domain::AutoZone::REMOVED:
			{
				do_shuffle.operator()<domain::AutoZone::REMOVED>(field.removed);
				break;
			}
			default: // NOLINT(clang-diagnostic-covered-switch-default)
			{
				std::unreachable();
			}
		}
	}

	auto Playground::reverse(const domain::Player player, const domain::AutoZone zone) noexcept -> void
	{
		auto& field = field_of(player);
		const auto do_reverse = []<domain::AutoZone Z>(Sequence& sequence) noexcept -> void
		{
			// 反转指定区域卡牌
			std::ranges::reverse(sequence);
			// 更新位置序号
			recalculate_zone<Z>(sequence);
		};

		switch (zone)
		{
			case domain::AutoZone::DECK:
			{
				do_reverse.operator()<domain::AutoZone::DECK>(field.deck);
				break;
			}
			case domain::AutoZone::EXTRA_DECK:
			{
				do_reverse.operator()<domain::AutoZone::EXTRA_DECK>(field.extra_deck);
				break;
			}
			case domain::AutoZone::HAND:
			{
				do_reverse.operator()<domain::AutoZone::HAND>(field.hand);
				break;
			}
			case domain::AutoZone::GRAVEYARD:
			{
				do_reverse.operator()<domain::AutoZone::GRAVEYARD>(field.graveyard);
				break;
			}
			case domain::AutoZone::REMOVED:
			{
				do_reverse.operator()<domain::AutoZone::REMOVED>(field.removed);
				break;
			}
			default: // NOLINT(clang-diagnostic-covered-switch-default)
			{
				std::unreachable();
			}
		}
	}

	auto Playground::select(const domain::Player player, const domain::Zone zone) const noexcept -> CardOptional
	{
		auto& field = field_of(player);
		auto& shared = shared_extra_monster_field;

		return zone.visit(
			utility::Overloaded{
					[&field](const domain::Zone::Deck deck) noexcept -> CardOptional
					{
						if (deck.index >= field.deck.size())
						{
							return nullptr;
						}

						return CardOptional{field.deck[deck.index]};
					},
					[&field](const domain::Zone::ExtraDeck extra_deck) noexcept -> CardOptional
					{
						if (extra_deck.index >= field.extra_deck.size())
						{
							return nullptr;
						}

						return CardOptional{field.extra_deck[extra_deck.index]};
					},
					[&field](const domain::Zone::Hand hand) noexcept -> CardOptional
					{
						if (hand.index >= field.hand.size())
						{
							return nullptr;
						}

						return CardOptional{field.hand[hand.index]};
					},
					[&field](const domain::Zone::Graveyard graveyard) noexcept -> CardOptional
					{
						if (graveyard.index >= field.graveyard.size())
						{
							return nullptr;
						}

						return CardOptional{field.graveyard[graveyard.index]};
					},
					[&field](const domain::Zone::Removed removed) noexcept -> CardOptional
					{
						if (removed.index >= field.removed.size())
						{
							return nullptr;
						}

						return CardOptional{field.removed[removed.index]};
					},
					[]([[maybe_unused]] const domain::Zone::Overlay overlay) noexcept -> CardOptional
					{
						return nullptr;
					},
					[&field](const domain::Zone::Monster monster) noexcept -> CardOptional
					{
						if (std::to_underlying(monster.position) >= domain::Zone::monster_main_count)
						{
							return nullptr;
						}

						return field.monster[std::to_underlying(monster.position)];
					},
					[&shared](const domain::Zone::ExtraMonster extra_monster) noexcept -> CardOptional
					{
						if (std::to_underlying(extra_monster.position) >= domain::Zone::monster_extra_count)
						{
							return nullptr;
						}

						return shared[std::to_underlying(extra_monster.position)];
					},
					[&field](const domain::Zone::SpellTrap spell_trap) noexcept -> CardOptional
					{
						if (std::to_underlying(spell_trap.position) >= domain::Zone::spell_trap_count)
						{
							return nullptr;
						}

						return field.spell_trap[std::to_underlying(spell_trap.position)];
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> CardOptional
					{
						return nullptr;
					},
			}
		);
	}

	auto Playground::occupied(const domain::Player player, const domain::Zone zone) const noexcept -> bool
	{
		return select(player, zone) != nullptr;
	}

	auto Playground::count(const domain::Player player, const domain::Zone zone) const noexcept -> size_type
	{
		return zone.visit(
			utility::Overloaded{
					[this, player]([[maybe_unused]] const domain::Zone::Deck deck) noexcept -> size_type
					{
						return count(player, domain::AutoZone::DECK);
					},
					[this, player]([[maybe_unused]] const domain::Zone::ExtraDeck extra_deck) noexcept -> size_type
					{
						return count(player, domain::AutoZone::EXTRA_DECK);
					},
					[this, player]([[maybe_unused]] const domain::Zone::Hand hand) noexcept -> size_type
					{
						return count(player, domain::AutoZone::HAND);
					},
					[this, player]([[maybe_unused]] const domain::Zone::Graveyard graveyard) noexcept -> size_type
					{
						return count(player, domain::AutoZone::GRAVEYARD);
					},
					[this, player]([[maybe_unused]] const domain::Zone::Removed removed) noexcept -> size_type
					{
						return count(player, domain::AutoZone::REMOVED);
					},
					[]([[maybe_unused]] const domain::Zone::Overlay overlay) noexcept -> size_type
					{
						return 0;
					},
					[this, player]([[maybe_unused]] const domain::Zone::Monster monster) noexcept -> size_type
					{
						return count(player, domain::FieldZone::MONSTER);
					},
					[this, player]([[maybe_unused]] const domain::Zone::ExtraMonster extra_monster) noexcept -> size_type
					{
						return count(player, domain::FieldZone::EXTRA_MONSTER);
					},
					[this, player]([[maybe_unused]] const domain::Zone::SpellTrap spell_trap) noexcept -> size_type
					{
						return count(player, domain::FieldZone::SPELL_TRAP);
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> size_type
					{
						return 0;
					},
			}
		);
	}

	auto Playground::count(const domain::Player player, const domain::AutoZone zone) const noexcept -> size_type
	{
		auto& field = field_of(player);

		switch (zone)
		{
			case domain::AutoZone::DECK:
			{
				return static_cast<size_type>(field.deck.size());
			}
			case domain::AutoZone::EXTRA_DECK:
			{
				return static_cast<size_type>(field.extra_deck.size());
			}
			case domain::AutoZone::HAND:
			{
				return static_cast<size_type>(field.hand.size());
			}
			case domain::AutoZone::GRAVEYARD:
			{
				return static_cast<size_type>(field.graveyard.size());
			}
			case domain::AutoZone::REMOVED:
			{
				return static_cast<size_type>(field.removed.size());
			}
			default: // NOLINT(clang-diagnostic-covered-switch-default)
			{
				std::unreachable();
			}
		}
	}

	auto Playground::count(const domain::Player player, const domain::FieldZone zone) const noexcept -> size_type
	{
		auto& field = field_of(player);
		auto& shared = shared_extra_monster_field;

		switch (zone)
		{
			case domain::FieldZone::MONSTER:
			{
				const auto s = std::ranges::count_if(field.monster, &CardOptional::operator bool);
				return static_cast<size_type>(s);
			}
			case domain::FieldZone::EXTRA_MONSTER:
			{
				size_type s = 0;

				if (shared[std::to_underlying(domain::Zone::ExtraMonsterPosition::EXTRA_1)])
				{
					s += 1;
				}
				if (shared[std::to_underlying(domain::Zone::ExtraMonsterPosition::EXTRA_2)])
				{
					s += 1;
				}

				return s;
			}
			case domain::FieldZone::SPELL_TRAP:
			{
				const auto s = std::ranges::count_if(field.spell_trap, &CardOptional::operator bool);
				return static_cast<size_type>(s);
			}
			default: // NOLINT(clang-diagnostic-covered-switch-default)
			{
				std::unreachable();
			}
		}
	}

	auto Playground::free(const domain::Player player, const domain::Zone zone) const noexcept -> std::vector<size_type>
	{
		return zone.visit(
			utility::Overloaded{
					[this, player]([[maybe_unused]] const domain::Zone::Monster monster) noexcept -> std::vector<size_type>
					{
						return free(player, domain::FieldZone::MONSTER);
					},
					[this, player]([[maybe_unused]] const domain::Zone::ExtraMonster extra_monster) noexcept -> std::vector<size_type>
					{
						return free(player, domain::FieldZone::EXTRA_MONSTER);
					},
					[this, player]([[maybe_unused]] const domain::Zone::SpellTrap spell_trap) noexcept -> std::vector<size_type>
					{
						return free(player, domain::FieldZone::SPELL_TRAP);
					},
					[]([[maybe_unused]] const auto unhandled) noexcept -> std::vector<size_type>
					{
						return {};
					}
			}
		);
	}

	auto Playground::free(const domain::Player player, const domain::FieldZone zone) const noexcept -> std::vector<size_type>
	{
		auto& field = field_of(player);
		auto& shared = shared_extra_monster_field;

		std::vector<size_type> result{};

		switch (zone)
		{
			case domain::FieldZone::MONSTER:
			{
				for (auto begin = field.monster.begin(), it = begin; it != field.monster.end(); ++it)
				{
					if (*it == nullptr)
					{
						const auto index = std::ranges::distance(begin, it);
						result.push_back(static_cast<size_type>(index));
					}
				}

				break;
			}
			case domain::FieldZone::EXTRA_MONSTER:
			{
				if (shared[std::to_underlying(domain::Zone::ExtraMonsterPosition::EXTRA_1)] == nullptr)
				{
					result.push_back(std::to_underlying(domain::Zone::ExtraMonsterPosition::EXTRA_1));
				}
				if (shared[std::to_underlying(domain::Zone::ExtraMonsterPosition::EXTRA_2)] == nullptr)
				{
					result.push_back(std::to_underlying(domain::Zone::ExtraMonsterPosition::EXTRA_2));
				}

				break;
			}
			case domain::FieldZone::SPELL_TRAP:
			{
				for (auto begin = field.spell_trap.begin(), it = begin; it != field.spell_trap.end(); ++it)
				{
					if (*it == nullptr)
					{
						const auto index = std::ranges::distance(begin, it);
						result.push_back(static_cast<size_type>(index));
					}
				}

				break;
			}
			default: // NOLINT(clang-diagnostic-covered-switch-default)
			{
				std::unreachable();
			}
		}

		return result;
	}

	auto Playground::remove(const CardReference card) noexcept -> bool
	{
		const auto card_state = card.get().state();
		const auto controller = card_state.controller();
		const auto zone = card_state.zone();

		auto& field = field_of(controller);
		auto& shared = shared_extra_monster_field;

		return zone.visit(
			utility::Overloaded{
					[&field](const domain::Zone::Deck deck) noexcept -> bool
					{
						if (field.deck.erase(deck.index))
						{
							recalculate_zone<domain::Zone::Deck>(field.deck);
							return true;
						}

						return false;
					},
					[&field](const domain::Zone::ExtraDeck extra_deck) noexcept -> bool
					{
						if (field.extra_deck.erase(extra_deck.index))
						{
							recalculate_zone<domain::Zone::ExtraDeck>(field.extra_deck);
							return true;
						}

						return false;
					},
					[&field](const domain::Zone::Hand hand) noexcept -> bool
					{
						if (field.hand.erase(hand.index))
						{
							recalculate_zone<domain::Zone::Hand>(field.hand);
							return true;
						}

						return false;
					},
					[&field](const domain::Zone::Graveyard graveyard) noexcept -> bool
					{
						if (field.graveyard.erase(graveyard.index))
						{
							recalculate_zone<domain::Zone::Graveyard>(field.graveyard);
							return true;
						}

						return false;
					},
					[&field](const domain::Zone::Removed removed) noexcept -> bool
					{
						if (field.removed.erase(removed.index))
						{
							recalculate_zone<domain::Zone::Removed>(field.removed);
							return true;
						}

						return false;
					},
					[]([[maybe_unused]] const domain::Zone::Overlay overlay) noexcept -> bool
					{
						return false;
					},
					[&field](const domain::Zone::Monster monster) noexcept -> bool
					{
						if (std::to_underlying(monster.position) >= domain::Zone::monster_main_count)
						{
							return false;
						}

						field.monster[std::to_underlying(monster.position)] = nullptr;
						return true;
					},
					[&shared](const domain::Zone::ExtraMonster extra_monster) noexcept -> bool
					{
						if (std::to_underlying(extra_monster.position) >= domain::Zone::spell_trap_count)
						{
							return false;
						}

						shared[std::to_underlying(extra_monster.position)] = nullptr;
						return true;
					},
					[&field](const domain::Zone::SpellTrap spell_trap) noexcept -> bool
					{
						if (std::to_underlying(spell_trap.position) >= domain::Zone::spell_trap_count)
						{
							return false;
						}

						field.spell_trap[std::to_underlying(spell_trap.position)] = nullptr;
						return true;
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> bool
					{
						return false;
					},
			}
		);
	}
}

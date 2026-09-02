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
	}

	auto Playground::player_field(const domain::Player player) noexcept -> PlayerField&
	{
		return player_fields_[std::to_underlying(player)];
	}

	auto Playground::player_field(const domain::Player player) const noexcept -> const PlayerField&
	{
		return player_fields_[std::to_underlying(player)];
	}

	Playground::Playground() noexcept
		: player_fields_
		{
				default_player_field(),
				default_player_field(),
		}
	// shared_extra_monster_field_{}
	{
		//
	}

	auto Playground::set_life_point(const domain::Player player, const domain::life_point_type life_point) noexcept -> void
	{
		player_field(player).life_point = life_point;
	}

	auto Playground::set_start_hand(const domain::Player player, const size_type start_hand) noexcept -> void
	{
		player_field(player).start_hand = start_hand;
	}

	auto Playground::set_draw_count(const domain::Player player, const size_type draw_count) noexcept -> void
	{
		player_field(player).draw_count = draw_count;
	}

	auto Playground::life_point(const domain::Player player) const noexcept -> domain::life_point_type
	{
		return player_field(player).life_point;
	}

	auto Playground::start_hand(const domain::Player player) const noexcept -> size_type
	{
		return player_field(player).start_hand;
	}

	auto Playground::draw_count(const domain::Player player) const noexcept -> size_type
	{
		return player_field(player).draw_count;
	}

	auto Playground::update_life_point(const domain::Player player, const domain::life_point_type delta) noexcept -> void
	{
		auto& pf = player_field(player);

		// 增加生命值
		if (delta > 0)
		{
			if (const auto remaining = std::numeric_limits<domain::life_point_type>::max() - pf.life_point;
				std::cmp_greater(delta, remaining))
			{
				// 超出最大值
				pf.life_point = std::numeric_limits<domain::life_point_type>::max();
			}
			else
			{
				pf.life_point += delta;
			}

			return;
		}

		// 减少生命值
		pf.life_point += delta;
		// 不会减少到负数
		pf.life_point = std::ranges::max(pf.life_point, static_cast<domain::life_point_type>(0));

		if (pf.life_point == 0)
		{
			// 生命值不足
			// todo: 决斗结束
		}
	}

	auto Playground::start_draw(const domain::Player player) noexcept -> void
	{
		draw(player, start_hand(player));
	}

	auto Playground::draw(const domain::Player player, const size_type count) noexcept -> void
	{
		auto& pf = player_field(player);

		for (size_type i = 0; i < count; ++i)
		{
			if (pf.deck.empty())
			{
				// 抽空卡组,决斗结束
				// todo: 决斗结束
				return;
			}

			// 新加入卡的位置序号
			const auto zone_index = pf.hand.size();

			// 从卡组拿取
			auto card = pf.deck.back();
			pf.deck.pop_back();
			// 加入手牌
			pf.hand.push_back(card);

			// 设置卡牌状态
			auto card_state = card.get().state();
			card_state.set_controller(player);
			card_state.set_zone(domain::Zone::Hand{.index = static_cast<size_type>(zone_index)});
			// 因抽卡移动
			card_state.set_reason({domain::Reason::DRAW});

			// todo: 记录抽到的卡
		}

		// todo: 广播抽到的卡
		// 例如 霸王龙Z-ARC[13331639] 的灵摆效果可以破坏刚刚抽到的所有卡
	}

	auto Playground::shuffle_deck(const domain::Player player, utility::Random& random) noexcept -> void
	{
		auto& pf = player_field(player);

		// 打乱卡组
		std::ranges::shuffle(pf.deck, random);
		// 更新位置序号
		recalculate_zone<domain::Zone::Deck>(pf.deck);
	}

	auto Playground::reverse_deck(const domain::Player player) noexcept -> void
	{
		auto& pf = player_field(player);

		// 反转卡组
		std::ranges::reverse(pf.deck);
		// 更新位置序号
		recalculate_zone<domain::Zone::Deck>(pf.deck);
	}

	auto Playground::shuffle_extra_deck(const domain::Player player, utility::Random& random) noexcept -> void
	{
		auto& pf = player_field(player);

		// 打乱额外卡组
		std::ranges::shuffle(pf.extra_deck, random);
		// 更新位置序号
		recalculate_zone<domain::Zone::ExtraDeck>(pf.extra_deck);
	}

	auto Playground::shuffle_hand(const domain::Player player, utility::Random& random) noexcept -> void
	{
		auto& pf = player_field(player);

		// 打乱手牌
		std::ranges::shuffle(pf.hand, random);
		// 更新位置序号
		recalculate_zone<domain::Zone::Hand>(pf.hand);
	}

	auto Playground::occupied(const domain::Player player, const domain::Zone zone) const noexcept -> bool
	{
		return select(player, zone) != nullptr;
	}

	auto Playground::select(const domain::Player player, const domain::Zone zone) const noexcept -> CardOptional
	{
		auto& pf = player_field(player);
		auto& shared = shared_extra_monster_field_;

		return zone.visit(
			utility::Overloaded
			{
					[&pf](const domain::Zone::Deck deck) noexcept -> CardOptional
					{
						if (deck.index >= pf.deck.size())
						{
							return nullptr;
						}

						return CardOptional{pf.deck[deck.index]};
					},
					[&pf](const domain::Zone::ExtraDeck extra_deck) noexcept -> CardOptional
					{
						if (extra_deck.index >= pf.extra_deck.size())
						{
							return nullptr;
						}

						return CardOptional{pf.extra_deck[extra_deck.index]};
					},
					[&pf](const domain::Zone::Hand hand) noexcept -> CardOptional
					{
						if (hand.index >= pf.hand.size())
						{
							return nullptr;
						}

						return CardOptional{pf.hand[hand.index]};
					},
					[&pf](const domain::Zone::Graveyard graveyard) noexcept -> CardOptional
					{
						if (graveyard.index >= pf.graveyard.size())
						{
							return nullptr;
						}

						return CardOptional{pf.graveyard[graveyard.index]};
					},
					[&pf](const domain::Zone::Removed removed) noexcept -> CardOptional
					{
						if (removed.index >= pf.removed.size())
						{
							return nullptr;
						}

						return CardOptional{pf.removed[removed.index]};
					},
					[](const domain::Zone::Overlay overlay) noexcept -> CardOptional
					{
						return nullptr;
					},
					[&pf](const domain::Zone::Monster monster) noexcept -> CardOptional
					{
						if (std::to_underlying(monster.position) >= domain::Zone::monster_main_count)
						{
							return nullptr;
						}

						return pf.monster[std::to_underlying(monster.position)];
					},
					[&shared](const domain::Zone::ExtraMonster extra_monster) noexcept -> CardOptional
					{
						if (std::to_underlying(extra_monster.position) >= domain::Zone::monster_extra_count)
						{
							return nullptr;
						}

						return shared[std::to_underlying(extra_monster.position)];
					},
					[&pf](const domain::Zone::SpellTrap spell_trap) noexcept -> CardOptional
					{
						if (std::to_underlying(spell_trap.position) >= domain::Zone::spell_trap_count)
						{
							return nullptr;
						}

						return pf.spell_trap[std::to_underlying(spell_trap.position)];
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> CardOptional
					{
						return nullptr;
					},
			}
		);
	}

	auto Playground::count(const domain::Player player, const domain::Zone zone) const noexcept -> size_type
	{
		auto& pf = player_field(player);
		auto& shared = shared_extra_monster_field_;

		return zone.visit(
			utility::Overloaded{
					[&pf]([[maybe_unused]] const domain::Zone::Deck deck) noexcept -> size_type
					{
						return static_cast<size_type>(pf.deck.size());
					},
					[&pf]([[maybe_unused]] const domain::Zone::ExtraDeck extra_deck) noexcept -> size_type
					{
						return static_cast<size_type>(pf.extra_deck.size());
					},
					[&pf]([[maybe_unused]] const domain::Zone::Hand hand) noexcept -> size_type
					{
						return static_cast<size_type>(pf.hand.size());
					},
					[&pf]([[maybe_unused]] const domain::Zone::Graveyard graveyard) noexcept -> size_type
					{
						return static_cast<size_type>(pf.graveyard.size());
					},
					[&pf]([[maybe_unused]] const domain::Zone::Removed removed) noexcept -> size_type
					{
						return static_cast<size_type>(pf.removed.size());
					},
					[]([[maybe_unused]] const domain::Zone::Overlay overlay) noexcept -> size_type
					{
						return 0;
					},
					[&pf]([[maybe_unused]] const domain::Zone::Monster monster) noexcept -> size_type
					{
						const auto s = std::ranges::count_if(pf.monster, &CardOptional::operator bool);
						return static_cast<size_type>(s);
					},
					[&shared]([[maybe_unused]] const domain::Zone::ExtraMonster extra_monster) noexcept -> size_type
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
					},
					[&pf]([[maybe_unused]] const domain::Zone::SpellTrap spell_trap) noexcept -> size_type
					{
						const auto s = std::ranges::count_if(pf.spell_trap, &CardOptional::operator bool);
						return static_cast<size_type>(s);
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> size_type
					{
						return 0;
					},
			}
		);
	}

	auto Playground::free_area(const domain::Player player, const domain::Zone zone) const noexcept -> std::vector<domain::Zone::size_type>
	{
		using return_type = std::vector<domain::Zone::size_type>;

		auto& pf = player_field(player);
		auto& shared = shared_extra_monster_field_;

		return zone.visit(
			utility::Overloaded
			{
					[&pf]([[maybe_unused]] const domain::Zone::Monster monster) noexcept -> return_type
					{
						return_type result{};

						for (auto begin = pf.monster.begin(), it = begin; it != pf.monster.end(); ++it)
						{
							if (*it == nullptr)
							{
								const auto index = std::ranges::distance(begin, it);
								result.push_back(static_cast<size_type>(index));
							}
						}

						return result;
					},
					[&shared]([[maybe_unused]] const domain::Zone::ExtraMonster extra_monster) noexcept -> return_type
					{
						return_type result{};

						if (shared[std::to_underlying(domain::Zone::ExtraMonsterPosition::EXTRA_1)] == nullptr)
						{
							result.push_back(std::to_underlying(domain::Zone::ExtraMonsterPosition::EXTRA_1));
						}
						if (shared[std::to_underlying(domain::Zone::ExtraMonsterPosition::EXTRA_2)] == nullptr)
						{
							result.push_back(std::to_underlying(domain::Zone::ExtraMonsterPosition::EXTRA_2));
						}


						return result;
					},
					[&pf]([[maybe_unused]] const domain::Zone::SpellTrap spell_trap) noexcept -> return_type
					{
						return_type result{};

						for (auto begin = pf.spell_trap.begin(), it = begin; it != pf.spell_trap.end(); ++it)
						{
							if (*it == nullptr)
							{
								const auto index = std::ranges::distance(begin, it);
								result.push_back(static_cast<size_type>(index));
							}
						}

						return result;
					},
					[]([[maybe_unused]] const auto unhandled) noexcept -> return_type
					{
						return {};
					}
			}
		);
	}

	auto Playground::remove_card(const CardReference card) noexcept -> bool
	{
		auto card_state = card.get().state();
		const auto current_controller = card_state.controller();
		const auto current_zone = card_state.zone();

		auto& pf = player_field(current_controller);
		auto& shared = shared_extra_monster_field_;

		const auto result = current_zone.visit(
			utility::Overloaded{
					[&pf](const domain::Zone::Deck deck) noexcept -> bool
					{
						if (pf.deck.erase(deck.index))
						{
							recalculate_zone<domain::Zone::Deck>(pf.deck);
							return true;
						}

						return false;
					},
					[&pf](const domain::Zone::ExtraDeck extra_deck) noexcept -> bool
					{
						if (pf.extra_deck.erase(extra_deck.index))
						{
							recalculate_zone<domain::Zone::ExtraDeck>(pf.extra_deck);
							return true;
						}

						return false;
					},
					[&pf](const domain::Zone::Hand hand) noexcept -> bool
					{
						if (pf.hand.erase(hand.index))
						{
							recalculate_zone<domain::Zone::Hand>(pf.hand);
							return true;
						}

						return false;
					},
					[&pf](const domain::Zone::Graveyard graveyard) noexcept -> bool
					{
						if (pf.graveyard.erase(graveyard.index))
						{
							recalculate_zone<domain::Zone::Graveyard>(pf.graveyard);
							return true;
						}

						return false;
					},
					[&pf](const domain::Zone::Removed removed) noexcept -> bool
					{
						if (pf.removed.erase(removed.index))
						{
							recalculate_zone<domain::Zone::Removed>(pf.removed);
							return true;
						}

						return false;
					},
					[]([[maybe_unused]] const domain::Zone::Overlay overlay) noexcept -> bool
					{
						return false;
					},
					[&pf](const domain::Zone::Monster monster) noexcept -> bool
					{
						if (std::to_underlying(monster.position) >= domain::Zone::monster_main_count)
						{
							return false;
						}

						pf.monster[std::to_underlying(monster.position)] = nullptr;
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
					[&pf](const domain::Zone::SpellTrap spell_trap) noexcept -> bool
					{
						if (std::to_underlying(spell_trap.position) >= domain::Zone::spell_trap_count)
						{
							return false;
						}

						pf.spell_trap[std::to_underlying(spell_trap.position)] = nullptr;
						return true;
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> bool
					{
						return false;
					},
			}
		);

		// 需要设置吗?
		card_state.set_zone(domain::Zone::not_exist);
		return result;
	}

	auto Playground::move_card(const CardReference card, const domain::Player player, const domain::Zone zone) noexcept -> bool
	{
		// todo
		return true;
	}
}

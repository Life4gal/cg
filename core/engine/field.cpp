#include <core/engine/field.hpp>

#include <limits>

#include <core/engine/card.hpp>

namespace cg::engine
{
	Field::PlaygroundHandler::PlaygroundHandler(Field& field) noexcept
		: field_{field} {}

	auto Field::PlaygroundHandler::playground_data() noexcept -> Playground&
	{
		return field_.get().playground_;
	}

	auto Field::PlaygroundHandler::playground_data() const noexcept -> const Playground&
	{
		return field_.get().playground_;
	}

	auto Field::PlaygroundHandler::player_field(const domain::Player player) noexcept -> PlayerField&
	{
		return playground_data().player_fields[std::to_underlying(player)];
	}

	auto Field::PlaygroundHandler::player_field(const domain::Player player) const noexcept -> const PlayerField&
	{
		return playground_data().player_fields[std::to_underlying(player)];
	}

	auto Field::PlaygroundHandler::set_life_point(const domain::Player player, const domain::life_point_type life_point) noexcept -> void
	{
		player_field(player).life_point = life_point;
	}

	auto Field::PlaygroundHandler::set_start_hand(const domain::Player player, const domain::zone_sequence_type start_hand) noexcept -> void
	{
		player_field(player).start_hand = start_hand;
	}

	auto Field::PlaygroundHandler::set_draw_count(const domain::Player player, const domain::zone_sequence_type draw_count) noexcept -> void
	{
		player_field(player).draw_count = draw_count;
	}

	auto Field::PlaygroundHandler::life_point(const domain::Player player) const noexcept -> domain::life_point_type
	{
		return player_field(player).life_point;
	}

	auto Field::PlaygroundHandler::start_hand(const domain::Player player) const noexcept -> domain::zone_sequence_type
	{
		return player_field(player).start_hand;
	}

	auto Field::PlaygroundHandler::draw_count(const domain::Player player) const noexcept -> domain::zone_sequence_type
	{
		return player_field(player).draw_count;
	}

	auto Field::PlaygroundHandler::update_life_point(const domain::Player player, const domain::life_point_delta_type delta) noexcept -> void
	{
		auto& pf = player_field(player);

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

		// 如果生命值足够
		if (std::cmp_greater(pf.life_point, -delta))
		{
			pf.life_point += delta;
			return;
		}

		// 生命值不足
		// todo: 决斗结束
	}

	auto Field::PlaygroundHandler::draw(const domain::Player player, const domain::zone_sequence_type count) noexcept -> void
	{
		auto& pf = player_field(player);

		for (auto i = domain::zone_sequence_type{0}; i < count; ++i)
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
			card_state.set_zone(domain::Zone::HAND);
			card_state.set_zone_index(static_cast<domain::zone_sequence_type>(zone_index));
			// 因抽卡移动
			card_state.set_reason({domain::Reason::DRAW});

			// todo: 记录抽到的卡
		}

		// todo: 广播抽到的卡
		// 例如 霸王龙Z-ARC[13331639] 的灵摆效果可以破坏刚刚抽到的所有卡
	}

	auto Field::PlaygroundHandler::shuffle_deck(const domain::Player player) noexcept -> void
	{
		auto& pf = player_field(player);

		// todo: 随机数引擎
		// std::ranges::shuffle(pf.deck, );
		std::ignore = pf.deck;

		// todo: 广播洗牌
	}

	auto Field::PlaygroundHandler::reverse_deck(const domain::Player player) noexcept -> void
	{
		auto& pf = player_field(player);

		// 反转卡组
		std::ranges::reverse(pf.deck);
		// 更新位置序号
		for (auto begin = pf.deck.begin(), it = begin; it < pf.deck.end(); ++it)
		{
			const auto zone_index = std::ranges::distance(begin, it);
			auto card_state = it->get().state();

			card_state.set_zone_index(static_cast<domain::zone_sequence_type>(zone_index));
		}

		// todo: 广播反转
	}

	auto Field::PlaygroundHandler::shuffle_extra_deck(const domain::Player player) noexcept -> void
	{
		auto& pf = player_field(player);

		// todo: 随机数引擎
		// std::ranges::shuffle(pf.extra_deck, );
		std::ignore = pf.extra_deck;

		// todo: 广播洗牌
	}

	auto Field::PlaygroundHandler::shuffle_hand(const domain::Player player) noexcept -> void
	{
		auto& pf = player_field(player);

		// todo: 随机数引擎
		// std::ranges::shuffle(pf.hand, );
		std::ignore = pf.hand;

		// todo: 广播洗牌
	}

	auto Field::PlaygroundHandler::occupied(const domain::Player player, const domain::Zone zone, const domain::FieldZoneSequence field_zone) const noexcept -> bool
	{
		return select(player, zone, field_zone) != nullptr;
	}

	auto Field::PlaygroundHandler::select(const domain::Player player, domain::Zone zone, const domain::FieldZoneSequence field_zone) const noexcept -> CardOptional
	{
		const auto area = [this, player, zone] noexcept -> std::span<const CardOptional>
		{
			const auto& pf = player_field(player);

			switch (zone)
			{
				case domain::Zone::DECK:
				case domain::Zone::HAND:
				{
					return {};
				}
				case domain::Zone::MONSTER:
				{
					return {pf.monster.begin(), pf.monster.end()};
				}
				case domain::Zone::EXTRA_MONSTER:
				{
					const auto& shared = playground_data().shared_extra_monster_field;

					return {shared.begin(), shared.end()};
				}
				case domain::Zone::SPELL_TRAP:
				{
					return {pf.spell_trap.begin(), pf.spell_trap.end()};
				}
				case domain::Zone::GRAVEYARD:
				case domain::Zone::REMOVED:
				case domain::Zone::EXTRA_DECK:
				case domain::Zone::OVERLAY:
				{
					return {};
				}
				default: // NOLINT(clang-diagnostic-covered-switch-default)
				{
					std::unreachable();
				}
			}
		}();

		// 不支持的区域或者没有卡
		if (area.empty())
		{
			return CardOptional{};
		}

		return area[std::to_underlying(field_zone)];
	}

	auto Field::PlaygroundHandler::select(const domain::Player player, const domain::Zone zone, const domain::zone_sequence_type zone_index) const noexcept -> CardOptional
	{
		const auto area = [this, player, zone] noexcept -> std::span<const CardReference>
		{
			const auto& pf = player_field(player);

			switch (zone)
			{
				case domain::Zone::DECK:
				{
					return {pf.deck.begin(), pf.deck.end()};
				}
				case domain::Zone::HAND:
				{
					return {pf.hand.begin(), pf.hand.end()};
				}
				case domain::Zone::MONSTER:
				case domain::Zone::EXTRA_MONSTER:
				case domain::Zone::SPELL_TRAP:
				{
					return {};
				}
				case domain::Zone::GRAVEYARD:
				{
					return {pf.graveyard.begin(), pf.graveyard.end()};
				}
				case domain::Zone::REMOVED:
				{
					return {pf.removed.begin(), pf.removed.end()};
				}
				case domain::Zone::EXTRA_DECK:
				{
					return {pf.extra_deck.begin(), pf.extra_deck.end()};
				}
				case domain::Zone::OVERLAY:
				{
					return {};
				}
				default: // NOLINT(clang-diagnostic-covered-switch-default)
				{
					std::unreachable();
				}
			}
		}();

		// 不支持的区域或者没有卡
		if (area.empty())
		{
			return CardOptional{};
		}

		// 越界
		if (area.size() < zone_index)
		{
			return CardOptional{};
		}

		return CardOptional{area[zone_index]};
	}

	auto Field::PlaygroundHandler::count(const domain::Player player, const domain::Zone zone) const noexcept -> domain::zone_sequence_type
	{
		const auto size = [this, player, zone] noexcept -> std::size_t
		{
			const auto& pf = player_field(player);

			switch (zone)
			{
				case domain::Zone::DECK:
				{
					return pf.deck.size();
				}
				case domain::Zone::HAND:
				{
					return pf.hand.size();
				}
				case domain::Zone::MONSTER:
				{
					return std::ranges::count_if(
						pf.monster,
						[](const CardOptional& card) noexcept -> bool
						{
							return card != nullptr;
						}
					);
				}
				case domain::Zone::EXTRA_MONSTER:
				{
					const auto& shared = playground_data().shared_extra_monster_field;

					return
							// 1
							+(shared[std::to_underlying(domain::FieldZoneSequence::MONSTER_EXTRA_1)] != nullptr) +
							// 2
							+(shared[std::to_underlying(domain::FieldZoneSequence::MONSTER_EXTRA_2)] != nullptr)
							//
							;
				}
				case domain::Zone::SPELL_TRAP:
				{
					return std::ranges::count_if(
						pf.spell_trap,
						[](const CardOptional& card) noexcept -> bool
						{
							return card != nullptr;
						}
					);
				}
				case domain::Zone::GRAVEYARD:
				{
					return pf.graveyard.size();
				}
				case domain::Zone::REMOVED:
				{
					return pf.removed.size();
				}
				case domain::Zone::EXTRA_DECK:
				{
					return pf.extra_deck.size();
				}
				case domain::Zone::OVERLAY:
				{
					return 0;
				}
				default: // NOLINT(clang-diagnostic-covered-switch-default)
				{
					std::unreachable();
				}
			}
		}();

		return static_cast<domain::zone_sequence_type>(size);
	}

	auto Field::PlaygroundHandler::select(const domain::Player player, const domain::Zone zone) const noexcept -> View
	{
		const auto& pf = player_field(player);

		switch (zone)
		{
			case domain::Zone::DECK:
			{
				return View{pf.deck};
			}
			case domain::Zone::HAND:
			{
				return View{pf.hand};
			}
			case domain::Zone::MONSTER:
			{
				auto sequence =
						pf.monster |
						std::views::filter(
							[](const CardOptional& card) noexcept -> bool
							{
								return card != nullptr;
							}
						) |
						std::views::transform([](const CardOptional& card) noexcept -> CardReference
							{
								return CardReference{*card.get()};
							}
						) |
						std::ranges::to<View::container_type>();

				return View{std::move(sequence)};
			}
			case domain::Zone::EXTRA_MONSTER:
			{
				const auto& shared = playground_data().shared_extra_monster_field;
				View::container_type sequence{};
				const auto push = [&sequence](const CardOptional& card) noexcept -> void
				{
					if (card != nullptr)
					{
						sequence.emplace_back(*card.get());
					}
				};

				push(shared[std::to_underlying(domain::FieldZoneSequence::MONSTER_EXTRA_1)]);
				push(shared[std::to_underlying(domain::FieldZoneSequence::MONSTER_EXTRA_2)]);

				return View{std::move(sequence)};
			}
			case domain::Zone::SPELL_TRAP:
			{
				auto sequence =
						pf.spell_trap |
						std::views::filter(
							[](const CardOptional& card) noexcept -> bool
							{
								return card != nullptr;
							}
						) |
						std::views::transform(
							[](const CardOptional& card) noexcept -> CardReference
							{
								return CardReference{*card.get()};
							}
						) |
						std::ranges::to<View::container_type>();

				return View{std::move(sequence)};
			}
			case domain::Zone::GRAVEYARD:
			{
				return View{pf.graveyard};
			}
			case domain::Zone::REMOVED:
			{
				return View{pf.removed};
			}
			case domain::Zone::EXTRA_DECK:
			{
				return View{pf.extra_deck};
			}
			case domain::Zone::OVERLAY:
			{
				return View{};
			}
			default: // NOLINT(clang-diagnostic-covered-switch-default)
			{
				std::unreachable();
			}
		}
	}

	auto Field::PlaygroundHandler::free_area(const domain::Player player, const domain::Zone zone) const noexcept -> std::vector<domain::FieldZoneSequence>
	{
		const auto& pf = player_field(player);

		switch (zone)
		{
			case domain::Zone::DECK:
			case domain::Zone::HAND:
			{
				return {};
			}
			case domain::Zone::MONSTER:
			{
				constexpr auto begin = std::to_underlying(domain::FieldZoneSequence::MONSTER_MAIN_1);
				constexpr auto end = begin + domain::field_monster_main_count;

				std::vector<domain::FieldZoneSequence> area{};

				for (auto i = begin; i < end; ++i)
				{
					if (pf.monster[i] == nullptr)
					{
						area.push_back(static_cast<domain::FieldZoneSequence>(i));
					}
				}

				return area;
			}
			case domain::Zone::EXTRA_MONSTER:
			{
				const auto& shared = playground_data().shared_extra_monster_field;

				std::vector<domain::FieldZoneSequence> area{};

				if (shared[std::to_underlying(domain::FieldZoneSequence::MONSTER_EXTRA_1)] == nullptr)
				{
					area.push_back(domain::FieldZoneSequence::MONSTER_EXTRA_1);
				}
				if (shared[std::to_underlying(domain::FieldZoneSequence::MONSTER_EXTRA_2)] == nullptr)
				{
					area.push_back(domain::FieldZoneSequence::MONSTER_EXTRA_2);
				}

				return area;
			}
			case domain::Zone::SPELL_TRAP:
			{
				constexpr auto begin = std::to_underlying(domain::FieldZoneSequence::SPELL_TRAP_1);
				constexpr auto end = begin + domain::field_spell_trap_count;

				std::vector<domain::FieldZoneSequence> area{};

				for (auto i = begin; i < end; ++i)
				{
					if (pf.spell_trap[i] == nullptr)
					{
						area.push_back(static_cast<domain::FieldZoneSequence>(i));
					}
				}

				return area;
			}
			case domain::Zone::GRAVEYARD:
			case domain::Zone::REMOVED:
			case domain::Zone::EXTRA_DECK:
			case domain::Zone::OVERLAY:
			{
				return {};
			}
			default: // NOLINT(clang-diagnostic-covered-switch-default)
			{
				std::unreachable();
			}
		}
	}

	Field::RandomHandler::RandomHandler(Field& field) noexcept
		: field_{field} {}

	auto Field::RandomHandler::random_data() noexcept -> Random&
	{
		return field_.get().random_;
	}

	auto Field::RandomHandler::random_data() const noexcept -> const Random&
	{
		return field_.get().random_;
	}

	auto Field::RandomHandler::toss_dice(const domain::Player player, const std::size_t count) noexcept -> void
	{
		auto& r = random_data();

		r.dice_results.clear();
		r.dice_results.reserve(count);

		// todo: 随机数引擎
		for (std::size_t i = 0; i < count; ++i)
		{
			// todo: 效果替换?
			// const auto value = random(domain::dice_min_value, domain::dice_max_value);
			// r.dice_results.push_back(static_cast<domain::DiceValue>(value));
		}

		// todo: 广播掷骰子
	}

	auto Field::RandomHandler::toss_coin(const domain::Player player, const std::size_t count) noexcept -> void
	{
		auto& r = random_data();

		r.coin_results.clear();
		r.coin_results.reserve(count);

		// todo: 随机数引擎
		for (std::size_t i = 0; i < count; ++i)
		{
			// todo: 效果替换?
			// const auto value = random(domain::coin_min_value, domain::coin_max_value);
			// r.coin_results.push_back(static_cast<domain::CoinSide>(value));
		}

		// todo: 广播掷硬币
	}

	auto Field::RandomHandler::dice_results(const domain::Player player) const noexcept -> std::span<const domain::DiceValue>
	{
		return random_data().dice_results;
	}

	auto Field::RandomHandler::coin_results(const domain::Player player) const noexcept -> std::span<const domain::CoinSide>
	{
		return random_data().coin_results;
	}

	auto Field::playground() noexcept -> PlaygroundHandler
	{
		return PlaygroundHandler{*this};
	}

	auto Field::random() noexcept -> RandomHandler
	{
		return RandomHandler{*this};
	}
}

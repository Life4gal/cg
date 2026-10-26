#include <cg/driver/local_session.hpp>

#include <algorithm>
#include <ranges>
#include <numeric>

#include <cg/utility/functional.hpp>

namespace cg::driver
{
	ScriptedController::ScriptedController(actions_type actions) noexcept
		: actions_{std::move(actions)},
		  next_{0} {}

	auto ScriptedController::set_actions(actions_type actions) noexcept -> void
	{
		actions_ = std::move(actions);
		next_ = 0;
	}

	auto ScriptedController::choose_action([[maybe_unused]] const PlayerView& view) -> std::optional<LocalAction>
	{
		if (next_ >= actions_.size())
		{
			return std::nullopt;
		}

		auto action = actions_[next_];
		next_ += 1;

		return action;
	}

	auto ScriptedController::select_cards(
		[[maybe_unused]] domain::Player player,
		const execution::Group& candidates,
		const std::size_t min,
		[[maybe_unused]] std::size_t max
	) noexcept -> execution::Group
	{
		if (candidates.size() < min)
		{
			return execution::Group{};
		}

		return candidates | std::views::take(min) | std::ranges::to<execution::Group>();
	}

	auto ScriptedController::select_monster_location(
		[[maybe_unused]] domain::Player player,
		[[maybe_unused]] state::CardInstanceId card,
		const std::span<const domain::MonsterLocation> candidates
	) noexcept -> std::optional<domain::MonsterLocation>
	{
		if (candidates.empty())
		{
			return std::nullopt;
		}

		return candidates.front();
	}

	auto ScriptedController::select_spell_trap_location(
		[[maybe_unused]] domain::Player player,
		[[maybe_unused]] state::CardInstanceId card,
		const std::span<const domain::SpellTrapLocation> candidates
	) noexcept -> std::optional<domain::SpellTrapLocation>
	{
		if (candidates.empty())
		{
			return std::nullopt;
		}

		return candidates.front();
	}

	auto ScriptedController::choose_triggers(const execution::TriggerDecision& decision) noexcept -> std::vector<std::size_t>
	{
		if (decision.optional)
		{
			return {};
		}

		// return std::views::iota(std::size_t{0}, decision.candidates.size()) | std::ranges::to<std::vector<std::size_t>>();

		std::vector<std::size_t> selected(decision.candidates.size());
		std::ranges::iota(selected, 0uz);

		return selected;
	}

	LocalSelector::LocalSelector(const controllers_type controllers) noexcept
		: controllers_{controllers} {}

	auto LocalSelector::controller_of(const domain::Player player) const noexcept -> LocalController&
	{
		return controllers_[std::to_underlying(player)];
	}

	auto LocalSelector::select_cards(
		const domain::Player player,
		const execution::Group& candidates,
		const std::size_t min,
		const std::size_t max
	) noexcept -> execution::Group
	{
		auto& controller = controller_of(player);
		return controller.select_cards(player, candidates, min, max);
	}

	auto LocalSelector::select_monster_location(
		const domain::Player player,
		const state::CardInstanceId card,
		const std::span<const domain::MonsterLocation> candidates
	) noexcept -> std::optional<domain::MonsterLocation>
	{
		auto& controller = controller_of(player);
		return controller.select_monster_location(player, card, candidates);
	}

	auto LocalSelector::select_spell_trap_location(
		const domain::Player player,
		const state::CardInstanceId card,
		const std::span<const domain::SpellTrapLocation> candidates
	) noexcept -> std::optional<domain::SpellTrapLocation>
	{
		auto& controller = controller_of(player);
		return controller.select_spell_trap_location(player, card, candidates);
	}

	auto LocalSelector::choose_triggers(const execution::TriggerDecision& decision) noexcept -> std::vector<std::size_t>
	{
		auto& controller = controller_of(decision.player);
		return controller.choose_triggers(decision);
	}

	auto LocalDuelSession::controller_of(const domain::Player player) noexcept -> LocalController&
	{
		return *controllers_[std::to_underlying(player)];
	}

	auto LocalDuelSession::controller_of(const domain::Player player) const noexcept -> const LocalController&
	{
		return *controllers_[std::to_underlying(player)];
	}

	auto LocalDuelSession::apply(const LocalAction& action, const domain::Player actor) noexcept -> bool
	{
		return std::visit(
			utility::Overloaded
			{
					[&]([[maybe_unused]] const PassAction& a) noexcept -> bool
					{
						// 单次过优先权本身是合法动作; 返回值只表示窗口是否关闭
						duel_.pass_priority();
						return true;
					},
					[&]([[maybe_unused]] const AdvancePhaseAction& a) noexcept -> bool
					{
						return duel_.change_phase();
					},
					[&]([[maybe_unused]] const EndTurnAction& a) noexcept -> bool
					{
						return duel_.end_turn();
					},
					[&]([[maybe_unused]] const SurrenderAction& a) noexcept -> bool
					{
						return duel_.concede(actor);
					},
					[&](const NormalSummonAction& a) noexcept -> bool
					{
						return duel_.normal_summon(a.card, a.column);
					},
					[&](const SetMonsterAction& a) noexcept -> bool
					{
						return duel_.set_monster(a.card, a.column);
					},
					[&](const TributeSummonAction& a) noexcept -> bool
					{
						return duel_.tribute_summon(a.card, a.column, a.tributes);
					},
					[&](const SelfSpecialSummonAction& a) noexcept -> bool
					{
						return duel_.self_special_summon(a.card, a.placement);
					},
					[&](const ExtraSummonAction& a) noexcept -> bool
					{
						switch (a.kind)
						{
							case domain::ExtraDeckSummonType::FUSION:
							{
								return duel_.fusion_summon(a.card, a.placement, a.materials);
							}
							case domain::ExtraDeckSummonType::SYNCHRO:
							{
								return duel_.synchro_summon(a.card, a.placement, a.materials);
							}
							case domain::ExtraDeckSummonType::XYZ:
							{
								return duel_.xyz_summon(a.card, a.placement, a.materials);
							}
							case domain::ExtraDeckSummonType::LINK:
							{
								return duel_.link_summon(a.card, a.placement, a.materials);
							}
							default: // NOLINT(clang-diagnostic-covered-switch-default)
							{
								std::unreachable();
							}
						}
					},
					[&](const ActivateCardAction& a) noexcept -> bool
					{
						return duel_.activate_card(a.card);
					},
					[&](const SetSpellTrapAction& a) noexcept -> bool
					{
						return duel_.set_spell_trap(a.card, a.column);
					},
					[&](const DeclareAttackAction& a) noexcept -> bool
					{
						return duel_.declare_attack(a.attacker, a.target);
					},
					[&](const ReplayAttackAction& a) noexcept -> bool
					{
						return duel_.replay_attack(a.target);
					},
					[&](const ActivateEffectAction& a) noexcept -> bool
					{
						return duel_.activate_effect(a.card, a.effect_index);
					},
					[&](const ApplySpecialSummonEffectAction& a) noexcept -> bool
					{
						return duel_.apply_special_summon_effect(a.card, a.effect_index, a.placement);
					},
			},
			action
		);
	}

	LocalDuelSession::LocalDuelSession(std::unique_ptr<LocalController> first, std::unique_ptr<LocalController> second) noexcept
		: controllers_{std::move(first), std::move(second)},
		  selector_{LocalSelector::controllers_type{*controllers_[0], *controllers_[1]}},
		  duel_{selector_} {}

	auto LocalDuelSession::duel() noexcept -> flow::Duel&
	{
		return duel_;
	}

	auto LocalDuelSession::duel() const noexcept -> const flow::Duel&
	{
		return duel_;
	}

	auto LocalDuelSession::view(const domain::Player player) const noexcept -> PlayerView
	{
		const auto& game = duel_.game();
		const auto opponent = domain::opponent_of(player);

		const auto take_field_spell = [&](const domain::Player p) noexcept -> state::CardInstanceId
		{
			const auto s = game.collect_zone_card(p, domain::Zone::FIELD_SPELL);
			if (s.empty())
			{
				return {};
			}

			return s.front();
		};

		PlayerView::SelfSide self_side
		{
				.deck_count = game.count_zone_card(player, domain::Zone::DECK),
				.extra_deck = game.collect_zone_card(player, domain::Zone::EXTRA_DECK),
				.hand = game.collect_zone_card(player, domain::Zone::HAND),
				.graveyard = game.collect_zone_card(player, domain::Zone::GRAVEYARD),
				.banished = game.collect_zone_card(player, domain::Zone::BANISHED),
				.monsters = game.collect_zones_card_ex(player, {domain::Zone::MONSTER, domain::Zone::EXTRA_MONSTER}),
				.spell_traps = game.collect_zone_card(player, domain::Zone::SPELL_TRAP),
				.field_spell = take_field_spell(player),
		};
		PlayerView::OpponentSide opponent_side
		{
				.deck_count = game.count_zone_card(opponent, domain::Zone::DECK),
				.extra_deck_count = game.count_zone_card(opponent, domain::Zone::EXTRA_DECK),
				.hand_count = game.count_zone_card(opponent, domain::Zone::HAND),
				.graveyard = game.collect_zone_card(opponent, domain::Zone::GRAVEYARD),
				.banished = game.collect_zone_card(opponent, domain::Zone::BANISHED),
				.monsters = game.collect_zones_card_ex(opponent, {domain::Zone::MONSTER, domain::Zone::EXTRA_MONSTER}),
				.spell_traps = game.collect_zone_card(opponent, domain::Zone::SPELL_TRAP),
				.field_spell = take_field_spell(opponent),
		};

		const auto is_public_field_card = [](const state::CardInstance& card) noexcept -> bool
		{
			const auto placement = card.placement();
			const auto position = domain::position_of(placement);
			if (!position.has_value())
			{
				return true;
			}

			return *position != domain::Position::FACE_DOWN_DEFENSE;
		};

		std::unordered_map<std::uint32_t, domain::CardCode> card_codes;
		const auto add_code = [&](const state::CardInstanceId id, const bool public_only) noexcept -> void
		{
			if (!id.valid())
			{
				return;
			}

			if (const auto* card = game.find_instance(id);
				card != nullptr)
			{
				if (public_only && !is_public_field_card(*card))
				{
					return;
				}

				card_codes.emplace(id.raw(), card->origin_code());
			}
		};

		std::ranges::for_each(self_side.extra_deck, std::bind_back(add_code, false));
		std::ranges::for_each(self_side.hand, std::bind_back(add_code, false));
		std::ranges::for_each(self_side.graveyard, std::bind_back(add_code, false));
		std::ranges::for_each(self_side.banished, std::bind_back(add_code, false));
		std::ranges::for_each(self_side.monsters, std::bind_back(add_code, false));
		std::ranges::for_each(self_side.spell_traps, std::bind_back(add_code, false));
		add_code(self_side.field_spell, false);

		std::ranges::for_each(opponent_side.graveyard, std::bind_back(add_code, false));
		std::ranges::for_each(opponent_side.banished, std::bind_back(add_code, true));
		std::ranges::for_each(opponent_side.monsters, std::bind_back(add_code, true));
		std::ranges::for_each(opponent_side.spell_traps, std::bind_back(add_code, true));
		add_code(opponent_side.field_spell, true);

		const std::array life_points{game.life_points(player), game.life_points(opponent)};

		return
		{
				.self_side = std::move(self_side),
				.opponent_side = std::move(opponent_side),
				.card_codes = std::move(card_codes),
				.life_points = life_points,
				.player = player,
				.turn_player = duel_.turn().turn_player(),
				.turn_id = duel_.turn().turn_id(),
				.phase = duel_.turn().phase(),
				.first_turn = duel_.turn().is_first_turn(),
				.priority_player = duel_.priority().holder_player(duel_.turn().turn_player()),
				.priority_both_passed = duel_.priority().both_passed(),
				.pending_attack = duel_.pending_attack(),
				.pending_replay = duel_.pending_replay(),
				.pending_summon = duel_.pending_summon(),
				.chain_pending = !duel_.chain().empty(),
		};
	}

	auto LocalDuelSession::load_scripts(const std::filesystem::path& directory) noexcept -> bool
	{
		return duel_.load_scripts(directory);
	}

	auto LocalDuelSession::initialize(const flow::DuelConfig& config) noexcept -> bool
	{
		initialized_ = duel_.initialize(config);
		return initialized_;
	}

	auto LocalDuelSession::run(const std::size_t max_steps) -> Result
	{
		if (!initialized_)
		{
			return
			{
					.status = Status::INVALID_ACTION,
					.winner = std::nullopt,
					.message = "session is not initialized",
			};
		}

		for (std::size_t step = 0; step < max_steps; ++step)
		{
			if (const auto winner = duel_.winner();
				winner.has_value())
			{
				return
				{
						.status = Status::MATCH_OVER,
						.winner = winner,
						.message = {},
				};
			}

			// 抽卡阶段自动抽卡(第一回合由 Duel 规则拒绝)
			if (duel_.turn().phase() == domain::Phase::DRAW)
			{
				duel_.draw_for_turn();

				if (const auto winner = duel_.winner(); winner.has_value())
				{
					return
					{
							.status = Status::MATCH_OVER,
							.winner = winner,
							.message = {},
					};
				}
			}

			const auto player = duel_.priority().holder_player(duel_.turn().turn_player());
			auto& controller = controller_of(player);

			auto action = controller.choose_action(view(player));
			if (!action.has_value())
			{
				return
				{
						.status = Status::NO_ACTION,
						.winner = std::nullopt,
						.message = "controller returned no action",
				};
			}

			if (!apply(*action, player))
			{
				return
				{
						.status = Status::INVALID_ACTION,
						.winner = std::nullopt,
						.message = "action rejected",
				};
			}
		}

		return
		{
				.status = Status::STEP_LIMIT,
				.winner = std::nullopt,
				.message = "step limit reached",
		};
	}
}

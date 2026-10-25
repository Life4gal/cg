#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include <cg/driver/local_session.hpp>
#include <cg/server/match_config.hpp>

namespace
{
	using namespace cg;

	[[nodiscard]] auto read_line() -> std::optional<std::string>
	{
		std::string line{};
		if (!std::getline(std::cin, line))
		{
			return std::nullopt;
		}

		return line;
	}

	[[nodiscard]] auto parse_index(const std::string_view text, std::size_t& index) noexcept -> bool
	{
		std::istringstream stream{std::string{text}};
		stream >> index;

		return !stream.fail();
	}

	[[nodiscard]] auto id_text(const state::CardInstanceId id) -> std::string
	{
		return std::format("{}", id.raw());
	}

	[[nodiscard]] auto player_text(const domain::Player player) -> std::string_view
	{
		return utility::to_string(player);
	}

	class ConsoleController final : public driver::LocalController
	{
		std::string name_;

		[[nodiscard]] auto prompt(const std::string_view text) const noexcept -> std::optional<std::string>
		{
			std::println("[{}] {}", name_, text);
			return read_line();
		}

		[[nodiscard]] auto choose_candidate_index(const std::string_view label, const std::size_t count) const noexcept -> std::optional<std::size_t>
		{
			if (count == 0)
			{
				return std::nullopt;
			}

			const auto line = prompt(std::format("{} [0..{}]: ", label, count - 1));
			if (!line.has_value())
			{
				return std::nullopt;
			}

			std::size_t index{};
			if (!parse_index(*line, index) || index >= count)
			{
				return std::nullopt;
			}

			return index;
		}

		[[nodiscard]] static auto choose_card_indices(const execution::Group& candidates, const std::size_t min, const std::size_t max) noexcept -> std::vector<std::size_t>
		{
			std::print("candidates: ");
			for (const auto [index, candidate]: candidates | std::views::enumerate)
			{
				std::print(" {}: {}", index, id_text(candidate));
			}
			std::print("\n  select {}...{} indices (space separated): ", min, max);
			std::fflush(stdout);

			const auto line = read_line();
			if (!line.has_value())
			{
				return {};
			}

			std::istringstream stream{*line};
			std::vector<std::size_t> selected{};
			std::size_t index{};
			while (stream >> index)
			{
				if (index >= candidates.size() || std::ranges::contains(selected, index))
				{
					return {};
				}
				selected.push_back(index);
			}

			return selected;
		}

		static auto print_view(const driver::PlayerView& view) noexcept -> void
		{
			std::println(
				"\n=== {} | turn {} | phase {} | LP {}/{} ===",
				player_text(view.player),
				std::to_underlying(view.turn_id),
				utility::to_string(view.phase),
				view.life_points[0],
				view.life_points[1]
			);

			std::print("hand:");
			for (std::size_t i = 0; i < view.self_side.hand.size(); ++i)
			{
				std::print(" {}:{}", i, id_text(view.self_side.hand[i]));
			}

			std::print("\nmonsters:");
			for (std::size_t i = 0; i < view.self_side.monsters.size(); ++i)
			{
				std::print(" {}:{}", i, id_text(view.self_side.monsters[i]));
			}

			std::print("\nspell/traps:");
			for (std::size_t i = 0; i < view.self_side.spell_traps.size(); ++i)
			{
				std::print(" {}:{}", i, id_text(view.self_side.spell_traps[i]));
			}

			if (view.self_side.field_spell.valid())
			{
				std::print(" field:{}", id_text(view.self_side.field_spell));
			}

			std::print("\nopponent monsters:");
			for (std::size_t i = 0; i < view.opponent_side.monsters.size(); ++i)
			{
				std::print(" {}:{}", i, id_text(view.opponent_side.monsters[i]));
			}
			std::print("\nopponent spell/traps:");
			for (std::size_t i = 0; i < view.opponent_side.spell_traps.size(); ++i)
			{
				std::print(" {}:{}", i, id_text(view.opponent_side.spell_traps[i]));
			}

			std::println(
				"\npriority: {} | chain={} | summon={} | attack={} | replay={}",
				player_text(view.priority_player),
				view.chain_pending ? "yes" : "no",
				view.pending_summon ? "yes" : "no",
				view.pending_attack ? "yes" : "no",
				view.pending_replay ? "yes" : "no"
			);
		}

	public:
		explicit ConsoleController(std::string name)
			: name_{std::move(name)} {}

		[[nodiscard]] auto choose_action(const driver::PlayerView& view) -> std::optional<driver::LocalAction> override
		{
			print_view(view);
			std::println("commands: pass | phase | end | summon <hand> <col 0-4> | setmon <hand> <col>");
			std::println("          set <hand> <col> | activate <hand> | effect <monster> <effect>");
			std::println("          tribute <hand> <col> <monster...> | attack <monster> [target]");
			std::println("          replay [target] | quit");

			while (true)
			{
				std::print("> ");
				std::fflush(stdout);
				const auto line = read_line();
				if (!line.has_value())
				{
					return std::nullopt;
				}

				std::istringstream stream{*line};
				std::string command{};
				stream >> command;

				if (command.empty() || command == "pass")
				{
					return driver::LocalAction{driver::PassAction{}};
				}
				if (command == "quit")
				{
					return std::nullopt;
				}
				if (command == "phase")
				{
					return driver::LocalAction{driver::AdvancePhaseAction{}};
				}
				if (command == "end")
				{
					return driver::LocalAction{driver::EndTurnAction{}};
				}

				std::size_t first{};
				std::size_t second{};
				std::size_t third{};

				if (command == "summon" || command == "setmon")
				{
					if (!(stream >> first >> second) || first >= view.self_side.hand.size() || second >= 5)
					{
						std::println("invalid arguments");
						continue;
					}

					const auto card = view.self_side.hand[first];
					const auto column = static_cast<domain::FieldColumn>(second);
					if (command == "summon")
					{
						return driver::LocalAction{driver::NormalSummonAction{card, column}};
					}
					return driver::LocalAction{driver::SetMonsterAction{card, column}};
				}

				if (command == "set")
				{
					if (!(stream >> first >> second) || first >= view.self_side.hand.size() || second >= 5)
					{
						std::println("invalid arguments");
						continue;
					}

					return driver::LocalAction{
							driver::SetSpellTrapAction{
									view.self_side.hand[first],
									static_cast<domain::FieldColumn>(second),
							}
					};
				}

				if (command == "activate")
				{
					if (!(stream >> first) || first >= view.self_side.hand.size())
					{
						std::println("invalid arguments");
						continue;
					}

					return driver::LocalAction{driver::ActivateCardAction{view.self_side.hand[first]}};
				}

				if (command == "effect")
				{
					if (!(stream >> first >> second) || first >= view.self_side.monsters.size())
					{
						std::println("invalid arguments");
						continue;
					}

					return driver::LocalAction{
							driver::ActivateEffectAction{
									view.self_side.monsters[first],
									second,
							}
					};
				}

				if (command == "tribute")
				{
					if (!(stream >> first >> second) || first >= view.self_side.hand.size() || second >= 5)
					{
						std::println("invalid arguments");
						continue;
					}

					std::vector<state::CardInstanceId> tributes{};
					while (stream >> third)
					{
						if (third >= view.self_side.monsters.size())
						{
							tributes.clear();
							break;
						}
						tributes.push_back(view.self_side.monsters[third]);
					}

					if (tributes.empty())
					{
						std::println("invalid tribute list");
						continue;
					}

					return driver::LocalAction{
							driver::TributeSummonAction{
									view.self_side.hand[first],
									static_cast<domain::FieldColumn>(second),
									std::move(tributes),
							}
					};
				}

				if (command == "attack")
				{
					if (!(stream >> first) || first >= view.self_side.monsters.size())
					{
						std::println("invalid arguments");
						continue;
					}

					std::optional<state::CardInstanceId> target{};
					if (stream >> second)
					{
						if (second >= view.opponent_side.monsters.size())
						{
							std::println("invalid target");
							continue;
						}
						target = view.opponent_side.monsters[second];
					}

					return driver::LocalAction{
							driver::DeclareAttackAction{
									view.self_side.monsters[first],
									target,
							}
					};
				}

				if (command == "replay")
				{
					std::optional<state::CardInstanceId> target{};
					if (stream >> first)
					{
						if (first >= view.opponent_side.monsters.size())
						{
							std::println("invalid target");
							continue;
						}
						target = view.opponent_side.monsters[first];
					}

					return driver::LocalAction{driver::ReplayAttackAction{target}};
				}

				std::println("unknown command");
			}
		}

		[[nodiscard]] auto select_cards(
			[[maybe_unused]] domain::Player player,
			const execution::Group& candidates,
			const std::size_t min,
			const std::size_t max
		) noexcept -> execution::Group override
		{
			const auto selected = choose_card_indices(candidates, min, max);
			if (selected.size() < min || selected.size() > max)
			{
				return {};
			}

			execution::Group result{};
			for (const auto index: selected)
			{
				result.push_back(*(candidates.begin() + index));
			}
			return result;
		}

		[[nodiscard]] auto select_monster_location(
			[[maybe_unused]] domain::Player player,
			[[maybe_unused]] state::CardInstanceId card,
			const std::span<const domain::MonsterLocation> candidates
		) noexcept -> std::optional<domain::MonsterLocation> override
		{
			std::print("monster locations:");
			for (std::size_t i = 0; i < candidates.size(); ++i)
			{
				std::print(" {}:col{}", i, std::to_underlying(candidates[i].column));
			}
			std::println();

			const auto index = choose_candidate_index("location index", candidates.size());
			if (!index.has_value())
			{
				return std::nullopt;
			}
			return candidates[*index];
		}

		[[nodiscard]] auto select_spell_trap_location(
			[[maybe_unused]] domain::Player player,
			[[maybe_unused]] state::CardInstanceId card,
			const std::span<const domain::SpellTrapLocation> candidates
		) noexcept -> std::optional<domain::SpellTrapLocation> override
		{
			std::print("spell/trap locations:");
			for (std::size_t i = 0; i < candidates.size(); ++i)
			{
				std::print(" {}:col{}", i, std::to_underlying(candidates[i].column));
			}
			std::println();

			const auto index = choose_candidate_index("location index", candidates.size());
			if (!index.has_value())
			{
				return std::nullopt;
			}
			return candidates[*index];
		}

		[[nodiscard]] auto choose_triggers(const execution::TriggerDecision& decision) noexcept -> std::vector<std::size_t> override
		{
			std::print("triggers:");
			for (std::size_t i = 0; i < decision.candidates.size(); ++i)
			{
				const auto& candidate = decision.candidates[i];
				std::print(" {}:{}", i, id_text(candidate.source));
			}
			std::println();

			const auto line = prompt(
				decision.optional ? "optional trigger indices (empty=none): " : "mandatory trigger order: "
			);
			if (!line.has_value() || line->empty())
			{
				if (decision.optional)
				{
					return {};
				}

				std::vector<std::size_t> all(decision.candidates.size());
				std::ranges::iota(all, std::size_t{0});
				return all;
			}

			std::istringstream stream{*line};
			std::vector<std::size_t> selected{};
			std::size_t index{};
			while (stream >> index)
			{
				if (index >= decision.candidates.size() || std::ranges::contains(selected, index))
				{
					return {};
				}
				selected.push_back(index);
			}
			return selected;
		}
	};
}

auto main(const int argc, char** argv) noexcept -> int
{
	const auto default_config_path = [argv]() -> std::filesystem::path
	{
		if (argv[0] != nullptr)
		{
			const auto beside_executable = std::filesystem::absolute(argv[0]).parent_path() / "configs" / "local_duel.json";
			if (std::filesystem::exists(beside_executable))
			{
				return beside_executable;
			}
		}

		return "configs/local_duel.json";
	};

	const std::filesystem::path config_path = argc > 1 ? argv[1] : default_config_path();

	const auto config = server::MatchConfig::load(config_path);
	if (!config.has_value())
	{
		std::println(stderr, "config load failed: {}", config.error());
		return 1;
	}

	auto first = std::make_unique<ConsoleController>("FIRST");
	auto second = std::make_unique<ConsoleController>("SECOND");
	driver::LocalDuelSession session{std::move(first), std::move(second)};

	if (!session.load_scripts(config->cards_path))
	{
		std::println(stderr, "load scripts failed");
		return 1;
	}
	if (!session.initialize(config->duel))
	{
		std::println(stderr, "initialize duel failed");
		return 1;
	}

	while (true)
	{
		const auto result = session.run();
		if (result.status == driver::LocalDuelSession::Status::MATCH_OVER)
		{
			if (result.winner.has_value())
			{
				std::println("winner: {}", player_text(*result.winner));
			}
			else
			{
				std::println("match over");
			}
			return 0;
		}
		if (result.status == driver::LocalDuelSession::Status::NO_ACTION)
		{
			std::println("no action: {}", result.message);
			return 0;
		}
		if (result.status == driver::LocalDuelSession::Status::INVALID_ACTION)
		{
			std::println("action rejected: {}", result.message);
			continue;
		}

		std::println("step limit reached");
		return 1;
	}
}

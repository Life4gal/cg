#include <cg/server/match_config.hpp>

#include <format>
#include <fstream>

#include <cg/utility/enum.hpp>
#include <cg/server/deck.hpp>

#include <nlohmann/json.hpp>

namespace cg::server
{
	namespace
	{
		[[nodiscard]] auto resolve_path(const std::filesystem::path& base, const std::filesystem::path& path) noexcept -> std::filesystem::path
		{
			if (path.is_absolute())
			{
				return path;
			}

			return (base / path).lexically_normal();
		}

		[[nodiscard]] auto read_json(const std::filesystem::path& file) -> std::expected<nlohmann::json, std::string>
		{
			std::ifstream stream{file, std::ios::binary};
			if (!stream.good())
			{
				return std::unexpected(std::format("cannot open config file: {}", file.string()));
			}

			try
			{
				return nlohmann::json::parse(stream);
			}
			catch (const nlohmann::json::parse_error& error)
			{
				return std::unexpected(std::format("config JSON parse error: {}", error.what()));
			}
		}
	}

	auto MatchConfig::load(const std::filesystem::path& file) noexcept -> std::expected<MatchConfig, std::string>
	{
		const auto document = read_json(file);
		if (!document.has_value())
		{
			return std::unexpected(document.error());
		}
		if (!document->is_object())
		{
			return std::unexpected("config root must be a JSON object");
		}

		const auto& json = *document;
		const auto base = file.parent_path();

		MatchConfig config{};

		try
		{
			if (const auto it = json.find("cards");
				it == json.end() || !it->is_string())
			{
				return std::unexpected("config field \"cards\" must be a string");
			}
			else
			{
				config.cards_path = resolve_path(base, it->get<std::string>());
			}

			const auto players_it = json.find("players");
			if (players_it == json.end() || !players_it->is_array() || players_it->size() != 2)
			{
				return std::unexpected("config field \"players\" must be an array of two objects");
			}

			for (std::size_t i = 0; i < 2; ++i)
			{
				const auto& player = (*players_it)[i];
				if (!player.is_object())
				{
					return std::unexpected("each entry of \"players\" must be an object");
				}

				const auto deck_it = player.find("deck");
				if (deck_it == player.end() || !deck_it->is_string())
				{
					return std::unexpected("each player must have a string field \"deck\"");
				}

				config.deck_paths[i] = resolve_path(base, deck_it->get<std::string>());

				const auto deck_document = read_json(config.deck_paths[i]);
				if (!deck_document.has_value())
				{
					return std::unexpected(deck_document.error());
				}

				const auto deck = Deck::load(*deck_document);
				if (!deck.has_value())
				{
					return std::unexpected(std::format("cannot load deck {}: {}", config.deck_paths[i].string(), deck.error()));
				}

				auto& target = config.duel.decks[i];
				target.main_deck = deck->main_deck;
				target.extra_deck = deck->extra_deck;
				target.side_deck = deck->side_deck;
			}

			if (const auto seed_it = json.find("seed");
				seed_it != json.end())
			{
				if (!seed_it->is_number_unsigned() && !seed_it->is_number_integer())
				{
					return std::unexpected("config field \"seed\" must be an integer");
				}
				config.duel.seed = seed_it->get<std::uint64_t>();
			}

			if (const auto first_it = json.find("first_player");
				first_it != json.end())
			{
				if (!first_it->is_string())
				{
					return std::unexpected("config field \"first_player\" must be a string");
				}

				const auto first = utility::from_string<domain::Player>(first_it->get<std::string>());
				if (!first.has_value())
				{
					return std::unexpected("config field \"first_player\" must be FIRST or SECOND");
				}
				config.duel.first_player = *first;
			}

			if (const auto rules_it = json.find("rules");
				rules_it != json.end())
			{
				if (!rules_it->is_object())
				{
					return std::unexpected("config field \"rules\" must be an object");
				}

				if (const auto life_it = rules_it->find("life_points");
					life_it != rules_it->end())
				{
					if (!life_it->is_number_integer())
					{
						return std::unexpected("rules.life_points must be an integer");
					}
					config.duel.life_points = life_it->get<std::int32_t>();
				}

				if (const auto hand_it = rules_it->find("opening_hand");
					hand_it != rules_it->end())
				{
					if (!hand_it->is_number_unsigned())
					{
						return std::unexpected("rules.opening_hand must be an unsigned integer");
					}
					config.duel.opening_hand = hand_it->get<std::size_t>();
				}

				if (const auto draw_it = rules_it->find("first_turn_draw");
					draw_it != rules_it->end())
				{
					if (!draw_it->is_boolean())
					{
						return std::unexpected("rules.first_turn_draw must be a boolean");
					}
					config.duel.first_turn_draw = draw_it->get<bool>();
				}

				if (const auto deck_rules_it = rules_it->find("deck");
					deck_rules_it != rules_it->end())
				{
					if (!deck_rules_it->is_object())
					{
						return std::unexpected("rules.deck must be an object");
					}

					auto& rules = config.duel.deck_rules;
					if (const auto it = deck_rules_it->find("main_min");
						it != deck_rules_it->end())
					{
						rules.main_min = it->get<std::size_t>();
					}
					if (const auto it = deck_rules_it->find("main_max");
						it != deck_rules_it->end())
					{
						rules.main_max = it->get<std::size_t>();
					}
					if (const auto it = deck_rules_it->find("extra_max");
						it != deck_rules_it->end())
					{
						rules.extra_max = it->get<std::size_t>();
					}
					if (const auto it = deck_rules_it->find("side_max");
						it != deck_rules_it->end())
					{
						rules.side_max = it->get<std::size_t>();
					}
					if (const auto it = deck_rules_it->find("max_copies");
						it != deck_rules_it->end())
					{
						rules.max_copies = it->get<std::size_t>();
					}
				}
			}

			return config;
		}
		catch (const nlohmann::json::exception& error)
		{
			return std::unexpected(std::format("config type error: {}", error.what()));
		}
		catch (const std::exception& error)
		{
			return std::unexpected(std::format("config load error: {}", error.what()));
		}
	}
}

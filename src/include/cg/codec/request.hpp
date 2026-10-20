#pragma once

#include <cg/driver/request.hpp>

#include <cg/codec/action.hpp>

namespace cg::codec
{
	template<>
	class ValueCodec<driver::request::QueryIdleCommand>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryIdleCommand>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryIdleCommand{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryIdleCommand& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SubmitAction>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// action
			// if (!json.contains("action") || !ValueCodec<driver::action::Action>::valid(json))
			if (!json.contains("action"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SubmitAction>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto action_it = json.find("action");
			auto action = ValueCodec<driver::action::Action>::decode(action_it.value());
			if (!action.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SubmitAction{.player = *player, .action = *std::move(action)};
		}

		[[nodiscard]] static auto encode(const driver::request::SubmitAction& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["action"] = ValueCodec<driver::action::Action>::encode(data.action);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			// min
			if (!json.contains("min") || !json.at("min").is_number_unsigned())
			{
				return false;
			}

			// max
			if (!json.contains("max") || !json.at("max").is_number_unsigned())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}
			const auto min_it = json.find("min");
			const auto min = min_it.value().get<domain::DeckLocation::size_type>();
			const auto max_it = json.find("max");
			const auto max = max_it.value().get<domain::DeckLocation::size_type>();


			return driver::request::SelectCards{.player = *player, .candidates = *std::move(candidates), .min = min, .max = max};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);
			object["min"] = data.max;
			object["max"] = data.max;

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectYesNo>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectYesNo>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectYesNo{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectYesNo& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectNumbers>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// candidates
			if (!json.contains("candidates") || !json.at("candidates").is_array())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectNumbers>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto candidates_it = json.find("candidates");
			std::vector<std::int64_t> candidates;
			try
			{
				candidates = candidates_it.value().get<std::vector<std::int64_t>>();
			}
			catch ([[maybe_unused]] const nlohmann::json::parse_error& error)
			{
				return std::nullopt;
			}

			return driver::request::SelectNumbers{.player = *player, .candidates = std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectNumbers& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["candidates"] = data.candidates;

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::OrderCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// candidates
			if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::OrderCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::request::OrderCards{.player = *player, .candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::request::OrderCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectMonsterZoneLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// location
			// if (!json.contains("location") || !ValueCodec<domain::MonsterLocation>::valid(json.at("location")))
			if (!json.contains("location"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectMonsterZoneLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto location_it = json.find("location");
			const auto location = ValueCodec<domain::MonsterLocation>::decode(location_it.value());
			if (!location.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectMonsterZoneLocation{.player = *player, .card = *card, .location = *location};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectMonsterZoneLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["location"] = ValueCodec<domain::MonsterLocation>::encode(data.location);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectAnyMonsterZoneLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// location
			// if (!json.contains("location") || !ValueCodec<domain::AnyMonsterLocation>::valid(json.at("location")))
			if (!json.contains("location"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectAnyMonsterZoneLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto location_it = json.find("location");
			const auto location = ValueCodec<domain::AnyMonsterLocation>::decode(location_it.value());
			if (!location.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectAnyMonsterZoneLocation{.player = *player, .card = *card, .location = *location};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectAnyMonsterZoneLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["location"] = ValueCodec<domain::AnyMonsterLocation>::encode(data.location);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectMonsterPosition>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// position
			// if (!json.contains("position") || !ValueCodec<domain::Position>::valid(json.at("position")))
			if (!json.contains("position"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectMonsterPosition>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto position_it = json.find("position");
			const auto position = ValueCodec<domain::Position>::decode(position_it.value());
			if (!position.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectMonsterPosition{.player = *player, .card = *card, .position = *position};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectMonsterPosition& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["position"] = ValueCodec<domain::Position>::encode(data.position);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectSpellTrapLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// location
			// if (!json.contains("location") || !ValueCodec<domain::SpellTrapLocation>::valid(json.at("location")))
			if (!json.contains("location"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectSpellTrapLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto location_it = json.find("location");
			const auto location = ValueCodec<domain::SpellTrapLocation>::decode(location_it.value());
			if (!location.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectSpellTrapLocation{.player = *player, .card = *card, .location = *location};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectSpellTrapLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["location"] = ValueCodec<domain::SpellTrapLocation>::encode(data.location);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryPositionChangeableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryPositionChangeableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryPositionChangeableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryPositionChangeableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};


	template<>
	class ValueCodec<driver::request::QueryPositionChangeablePositions>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryPositionChangeablePositions>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryPositionChangeablePositions{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryPositionChangeablePositions& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryNormalSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryNormalSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryNormalSummonableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryNormalSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryNormalSummonType>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryNormalSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryNormalSummonType{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryNormalSummonType& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::CheckNormalSummonType>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// type
			// if (!json.contains("type") || !ValueCodec<domain::NormalSummonType>::valid(json.at("type")))
			if (!json.contains("type"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::CheckNormalSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto type_it = json.find("type");
			const auto type = ValueCodec<domain::NormalSummonType>::decode(type_it.value());
			if (!type.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::CheckNormalSummonType{.player = *player, .type = *type, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::CheckNormalSummonType& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["type"] = ValueCodec<domain::NormalSummonType>::encode(data.type);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryNormalSummonLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryNormalSummonLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryNormalSummonLocation{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryNormalSummonLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryNormalSummonPlacement>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryNormalSummonPlacement>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryNormalSummonPlacement{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryNormalSummonPlacement& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryTributeSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryTributeSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			return driver::request::QueryTributeSummonableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryTributeSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryTributeSummonTributeCandidates>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryTributeSummonTributeCandidates>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryTributeSummonTributeCandidates{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryTributeSummonTributeCandidates& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryTributeSummonType>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryTributeSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryTributeSummonType{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryTributeSummonType& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::CheckTributeSummonType>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// type
			// if (!json.contains("type") || !ValueCodec<domain::NormalSummonType>::valid(json.at("type")))
			if (!json.contains("type"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::CheckTributeSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto type_it = json.find("type");
			const auto type = ValueCodec<domain::NormalSummonType>::decode(type_it.value());
			if (!type.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::CheckTributeSummonType{.player = *player, .type = *type, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::CheckTributeSummonType& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["type"] = ValueCodec<domain::NormalSummonType>::encode(data.type);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectTributeSummonTributes>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}


			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// tributes
			// if (!json.contains("tributes") || !ValueCodec<std::vector<state::CardInstanceId>>::valid("tributes"))
			if (!json.contains("tributes") || !json.at("tributes").is_array())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectTributeSummonTributes>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto tributes_it = json.find("tributes");
			auto tributes = ValueCodec<std::vector<state::CardInstanceId>>::decode(tributes_it.value());
			if (!tributes.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectTributeSummonTributes{.player = *player, .card = *card, .tributes = *std::move(tributes)};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectTributeSummonTributes& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["tributes"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.tributes);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryTributeSummonLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryTributeSummonLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryTributeSummonLocation{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryTributeSummonLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryFlipSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryFlipSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryFlipSummonableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryFlipSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryGeminiSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryGeminiSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryGeminiSummonableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryGeminiSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryExtraDeckSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// method
			// if (!json.contains("method") || !ValueCodec<domain::SummonMethod>::valid(json.at("method")))
			if (!json.contains("method"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryExtraDeckSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto method_it = json.find("method");
			const auto method = ValueCodec<domain::SummonMethod>::decode(method_it.value());
			if (!method.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryExtraDeckSummonableCards{.player = *player, .method = *method};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryExtraDeckSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["method"] = ValueCodec<domain::SummonMethod>::encode(data.method);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryExtraDeckSummonMaterialSources>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// method
			// if (!json.contains("method") || !ValueCodec<domain::SummonMethod>::valid(json.at("method")))
			if (!json.contains("method"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryExtraDeckSummonMaterialSources>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto method_it = json.find("method");
			const auto method = ValueCodec<domain::SummonMethod>::decode(method_it.value());
			if (!method.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryExtraDeckSummonMaterialSources{.player = *player, .card = *card, .method = *method};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryExtraDeckSummonMaterialSources& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["method"] = ValueCodec<domain::SummonMethod>::encode(data.method);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryExtraDeckSummonMaterials>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// method
			// if (!json.contains("method") || !ValueCodec<domain::SummonMethod>::valid(json.at("method")))
			if (!json.contains("method"))
			{
				return false;
			}

			// material_source
			// if (!json.contains("material_source") || !ValueCodec<utility::EnumMask<domain::SummonMaterialSource>>::valid(json.at("material_source")))
			if (!json.contains("material_source"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryExtraDeckSummonMaterials>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto method_it = json.find("method");
			const auto method = ValueCodec<domain::SummonMethod>::decode(method_it.value());
			if (!method.has_value())
			{
				return std::nullopt;
			}
			const auto material_source_it = json.find("material_source");
			const auto material_source = ValueCodec<utility::EnumMask<domain::SummonMaterialSource>>::decode(material_source_it.value());
			if (!material_source.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryExtraDeckSummonMaterials{.player = *player, .card = *card, .method = *method, .material_source = *material_source};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryExtraDeckSummonMaterials& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["method"] = ValueCodec<domain::SummonMethod>::encode(data.method);
			object["material_source"] = ValueCodec<utility::EnumMask<domain::SummonMaterialSource>>::encode(data.material_source);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryExtraDeckSummonLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// method
			// if (!json.contains("method") || !ValueCodec<domain::SummonMethod>::valid(json.at("method")))
			if (!json.contains("method"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryExtraDeckSummonLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto method_it = json.find("method");
			const auto method = ValueCodec<domain::SummonMethod>::decode(method_it.value());
			if (!method.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryExtraDeckSummonLocation{.player = *player, .card = *card, .method = *method};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryExtraDeckSummonLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["method"] = ValueCodec<domain::SummonMethod>::encode(data.method);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryPendulumSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryPendulumSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryPendulumSummonableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryPendulumSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QuerySpecialSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QuerySpecialSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QuerySpecialSummonableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QuerySpecialSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QuerySpecialSummonLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QuerySpecialSummonLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QuerySpecialSummonLocation{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QuerySpecialSummonLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QuerySpecialSummonPlacements>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QuerySpecialSummonPlacements>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QuerySpecialSummonPlacements{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QuerySpecialSummonPlacements& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QuerySetSpellTrapCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QuerySetSpellTrapCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QuerySetSpellTrapCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QuerySetSpellTrapCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QuerySpellTrapSetLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QuerySpellTrapSetLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QuerySpellTrapSetLocation{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QuerySpellTrapSetLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryActivatePendulumCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryActivatePendulumCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryActivatePendulumCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryActivatePendulumCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryActivatePendulumSide>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryActivatePendulumSide>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryActivatePendulumSide{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryActivatePendulumSide& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryActivatableEffects>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (json.contains("card") && !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryActivatableEffects>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			if (card_it == json.end())
			{
				return driver::request::QueryActivatableEffects{.player = *player, .card = std::nullopt};
			}
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryActivatableEffects{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryActivatableEffects& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			if (data.card.has_value())
			{
				object["card"] = ValueCodec<state::CardInstanceId>::encode(*data.card);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryEffectTargets>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// effect
			// if (!json.contains("effect") || !ValueCodec<state::EffectId>::valid(json.at("effect")))
			if (!json.contains("effect"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryEffectTargets>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto effect_it = json.find("effect");
			const auto effect = ValueCodec<dsl::EffectId>::decode(effect_it.value());
			if (!effect.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryEffectTargets{.player = *player, .card = *card, .effect = *effect};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryEffectTargets& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["effect"] = ValueCodec<dsl::EffectId>::encode(data.effect);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectEffectActivation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// effect
			// if (!json.contains("effect") || !ValueCodec<state::EffectId>::valid(json.at("effect")))
			if (!json.contains("effect"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectEffectActivation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto effect_it = json.find("effect");
			const auto effect = ValueCodec<dsl::EffectId>::decode(effect_it.value());
			if (!effect.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectEffectActivation{.player = *player, .card = *card, .effect = *effect};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectEffectActivation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["effect"] = ValueCodec<dsl::EffectId>::encode(data.effect);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectEffectTargets>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// effect
			// if (!json.contains("effect") || !ValueCodec<state::EffectId>::valid(json.at("effect")))
			if (!json.contains("effect"))
			{
				return false;
			}

			// targets
			// if (!json.contains("targets") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("targets")))
			if (!json.contains("targets"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectEffectTargets>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto effect_it = json.find("effect");
			const auto effect = ValueCodec<dsl::EffectId>::decode(effect_it.value());
			if (!effect.has_value())
			{
				return std::nullopt;
			}
			const auto targets_it = json.find("targets");
			auto targets = ValueCodec<std::vector<state::CardInstanceId>>::decode(targets_it.value());
			if (!targets.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectEffectTargets{.player = *player, .card = *card, .effect = *effect, .targets = *std::move(targets)};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectEffectTargets& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["effect"] = ValueCodec<dsl::EffectId>::encode(data.effect);
			object["targets"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.targets);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryChainableEffects>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryChainableEffects>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryChainableEffects{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryChainableEffects& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectChain>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// effect
			// if (!json.contains("effect") || !ValueCodec<state::EffectId>::valid(json.at("effect")))
			if (!json.contains("effect"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectChain>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto effect_it = json.find("effect");
			const auto effect = ValueCodec<dsl::EffectId>::decode(effect_it.value());
			if (!effect.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectChain{.player = *player, .card = *card, .effect = *effect};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectChain& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["effect"] = ValueCodec<dsl::EffectId>::encode(data.effect);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryAttackableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryAttackableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryAttackableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryAttackableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::CheckDirectAttackable>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::CheckDirectAttackable>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::CheckDirectAttackable{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::CheckDirectAttackable& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryAttackTargets>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryAttackTargets>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryAttackTargets{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryAttackTargets& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectAttack>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// target
			// if (json.contains("target") && !ValueCodec<state::CardInstanceId>::valid(json.at("target")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectAttack>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto target_it = json.find("target");
			if (target_it == json.end())
			{
				return driver::request::SelectAttack{.player = *player, .card = *card, .target = std::nullopt};
			}
			const auto target = ValueCodec<state::CardInstanceId>::decode(target_it.value());
			if (!target.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectAttack{.player = *player, .card = *card, .target = *target};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectAttack& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			if (data.target.has_value())
			{
				object["target"] = ValueCodec<state::CardInstanceId>::encode(*data.target);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryPhaseTransitions>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryPhaseTransitions>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryPhaseTransitions{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryPhaseTransitions& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectPhaseTransition>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// to
			// if (!json.contains("to") || !ValueCodec<domain::Phase>::valid(json.at("to")))
			if (!json.contains("to"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectPhaseTransition>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto to_it = json.find("to");
			const auto to = ValueCodec<domain::Phase>::decode(to_it.value());
			if (!to.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectPhaseTransition{.player = *player, .to = *to};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectPhaseTransition& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["to"] = ValueCodec<domain::Phase>::encode(data.to);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::CancelSelection>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// context
			// if (!json.contains("context") || !ValueCodec<domain::SelectionContext>::valid(json.at("context")))
			if (!json.contains("context"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::CancelSelection>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto context_it = json.find("context");
			const auto context = ValueCodec<domain::SelectionContext>::decode(context_it.value());
			if (!context.has_value())
			{
				return std::nullopt;
			}

			return driver::request::CancelSelection{.player = *player, .context = *context};
		}

		[[nodiscard]] static auto encode(const driver::request::CancelSelection& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["context"] = ValueCodec<domain::SelectionContext>::encode(data.context);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::ConfirmSelection>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// context
			// if (!json.contains("context") || !ValueCodec<domain::SelectionContext>::valid(json.at("context")))
			if (!json.contains("context"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::ConfirmSelection>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto context_it = json.find("context");
			const auto context = ValueCodec<domain::SelectionContext>::decode(context_it.value());
			if (!context.has_value())
			{
				return std::nullopt;
			}

			return driver::request::ConfirmSelection{.player = *player, .context = *context};
		}

		[[nodiscard]] static auto encode(const driver::request::ConfirmSelection& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["context"] = ValueCodec<domain::SelectionContext>::encode(data.context);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::PassResponse>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::PassResponse>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::PassResponse{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::PassResponse& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::Request>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// type相比于variant_index更方便校验
			if (json.contains("type") && !json.at("type").is_string())
			{
				return false;
			}

			if (!ValueCodec<VariantWrap<driver::request::Request>>::valid(json))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::Request>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			// 如果有type则按照type匹配
			if (const auto type_it = json.find("type");
				type_it != json.end())
			{
				const auto do_decode = [&]<typename L> noexcept -> std::optional<driver::request::Request>
				{
					if (const auto r = ValueCodec<L>::decode(json);
						r.has_value())
					{
						return driver::request::Request{*std::move(r)};
					}

					return std::nullopt;
				};

				const auto& type = type_it.value().get_ref<const nlohmann::json::string_t&>();

				if (type == "query_idle_command")
				{
					return do_decode.operator()<driver::request::QueryIdleCommand>();
				}
				if (type == "submit_action")
				{
					return do_decode.operator()<driver::request::SubmitAction>();
				}
				if (type == "select_cards")
				{
					return do_decode.operator()<driver::request::SelectCards>();
				}
				if (type == "select_yes_no")
				{
					return do_decode.operator()<driver::request::SelectYesNo>();
				}
				if (type == "select_numbers")
				{
					return do_decode.operator()<driver::request::SelectNumbers>();
				}
				if (type == "order_cards")
				{
					return do_decode.operator()<driver::request::OrderCards>();
				}
				if (type == "select_monster_zone_location")
				{
					return do_decode.operator()<driver::request::SelectMonsterZoneLocation>();
				}
				if (type == "select_any_monster_zone_location")
				{
					return do_decode.operator()<driver::request::SelectAnyMonsterZoneLocation>();
				}
				if (type == "select_monster_position")
				{
					return do_decode.operator()<driver::request::SelectMonsterPosition>();
				}
				if (type == "select_spell_trap_location")
				{
					return do_decode.operator()<driver::request::SelectSpellTrapLocation>();
				}
				if (type == "query_position_changeable_cards")
				{
					return do_decode.operator()<driver::request::QueryPositionChangeableCards>();
				}
				if (type == "query_position_changeable_positions")
				{
					return do_decode.operator()<driver::request::QueryPositionChangeablePositions>();
				}
				if (type == "query_normal_summonable_cards")
				{
					return do_decode.operator()<driver::request::QueryNormalSummonableCards>();
				}
				if (type == "query_normal_summon_type")
				{
					return do_decode.operator()<driver::request::QueryNormalSummonType>();
				}
				if (type == "check_normal_summon_type")
				{
					return do_decode.operator()<driver::request::CheckNormalSummonType>();
				}
				if (type == "query_normal_summon_location")
				{
					return do_decode.operator()<driver::request::QueryNormalSummonLocation>();
				}
				if (type == "query_normal_summon_placement")
				{
					return do_decode.operator()<driver::request::QueryNormalSummonPlacement>();
				}
				if (type == "query_tribute_summonable_cards")
				{
					return do_decode.operator()<driver::request::QueryTributeSummonableCards>();
				}
				if (type == "query_tribute_summon_tribute_candidates")
				{
					return do_decode.operator()<driver::request::QueryTributeSummonTributeCandidates>();
				}
				if (type == "query_tribute_summon_type")
				{
					return do_decode.operator()<driver::request::QueryTributeSummonType>();
				}
				if (type == "check_tribute_summon_type")
				{
					return do_decode.operator()<driver::request::CheckTributeSummonType>();
				}
				if (type == "select_tribute_summon_tributes")
				{
					return do_decode.operator()<driver::request::SelectTributeSummonTributes>();
				}
				if (type == "query_tribute_summon_location")
				{
					return do_decode.operator()<driver::request::QueryTributeSummonLocation>();
				}
				if (type == "query_flip_summonable_cards")
				{
					return do_decode.operator()<driver::request::QueryFlipSummonableCards>();
				}
				if (type == "query_gemini_summonable_cards")
				{
					return do_decode.operator()<driver::request::QueryGeminiSummonableCards>();
				}
				if (type == "query_extra_deck_summonable_cards")
				{
					return do_decode.operator()<driver::request::QueryExtraDeckSummonableCards>();
				}
				if (type == "query_extra_deck_summon_material_sources")
				{
					return do_decode.operator()<driver::request::QueryExtraDeckSummonMaterialSources>();
				}
				if (type == "query_extra_deck_summon_materials")
				{
					return do_decode.operator()<driver::request::QueryExtraDeckSummonMaterials>();
				}
				if (type == "query_extra_deck_summon_location")
				{
					return do_decode.operator()<driver::request::QueryExtraDeckSummonLocation>();
				}
				if (type == "query_pendulum_summonable_cards")
				{
					return do_decode.operator()<driver::request::QueryPendulumSummonableCards>();
				}
				if (type == "query_special_summonable_cards")
				{
					return do_decode.operator()<driver::request::QuerySpecialSummonableCards>();
				}
				if (type == "query_special_summon_location")
				{
					return do_decode.operator()<driver::request::QuerySpecialSummonLocation>();
				}
				if (type == "query_special_summon_placements")
				{
					return do_decode.operator()<driver::request::QuerySpecialSummonPlacements>();
				}
				if (type == "query_set_spell_trap_cards")
				{
					return do_decode.operator()<driver::request::QuerySetSpellTrapCards>();
				}
				if (type == "query_spell_trap_set_location")
				{
					return do_decode.operator()<driver::request::QuerySpellTrapSetLocation>();
				}
				if (type == "query_activate_pendulum_cards")
				{
					return do_decode.operator()<driver::request::QueryActivatePendulumCards>();
				}
				if (type == "query_activate_pendulum_side")
				{
					return do_decode.operator()<driver::request::QueryActivatePendulumSide>();
				}
				if (type == "query_activatable_effects")
				{
					return do_decode.operator()<driver::request::QueryActivatableEffects>();
				}
				if (type == "query_effect_targets")
				{
					return do_decode.operator()<driver::request::QueryEffectTargets>();
				}
				if (type == "select_effect_activation")
				{
					return do_decode.operator()<driver::request::SelectEffectActivation>();
				}
				if (type == "select_effect_targets")
				{
					return do_decode.operator()<driver::request::SelectEffectTargets>();
				}
				if (type == "query_chainable_effects")
				{
					return do_decode.operator()<driver::request::QueryChainableEffects>();
				}
				if (type == "select_chain")
				{
					return do_decode.operator()<driver::request::SelectChain>();
				}
				if (type == "query_attackable_cards")
				{
					return do_decode.operator()<driver::request::QueryAttackableCards>();
				}
				if (type == "check_direct_attackable")
				{
					return do_decode.operator()<driver::request::CheckDirectAttackable>();
				}
				if (type == "query_attack_targets")
				{
					return do_decode.operator()<driver::request::QueryAttackTargets>();
				}
				if (type == "select_attack")
				{
					return do_decode.operator()<driver::request::SelectAttack>();
				}
				if (type == "query_phase_transitions")
				{
					return do_decode.operator()<driver::request::QueryPhaseTransitions>();
				}
				if (type == "select_phase_transition")
				{
					return do_decode.operator()<driver::request::SelectPhaseTransition>();
				}
				if (type == "cancel_selection")
				{
					return do_decode.operator()<driver::request::CancelSelection>();
				}
				if (type == "confirm_selection")
				{
					return do_decode.operator()<driver::request::ConfirmSelection>();
				}
				if (type == "pass_response")
				{
					return do_decode.operator()<driver::request::PassResponse>();
				}

				// 未知type直接失败
				return std::nullopt;
			}

			// 没有type按照index匹配
			return ValueCodec<VariantWrap<driver::request::Request>>::decode(json);
		}

		[[nodiscard]] static auto encode(const driver::request::Request& data) noexcept -> nlohmann::json
		{
			auto object = ValueCodec<VariantWrap<driver::request::Request>>::encode(data);

			switch (data.index())
			{
				case utility::variant_index_v<driver::request::QueryIdleCommand, driver::request::Request>:
				{
					object["type"] = "query_idle_command";
					break;
				}
				case utility::variant_index_v<driver::request::SubmitAction, driver::request::Request>:
				{
					object["type"] = "submit_action";
					break;
				}
				case utility::variant_index_v<driver::request::SelectCards, driver::request::Request>:
				{
					object["type"] = "select_cards";
					break;
				}
				case utility::variant_index_v<driver::request::SelectYesNo, driver::request::Request>:
				{
					object["type"] = "select_yes_no";
					break;
				}
				case utility::variant_index_v<driver::request::SelectNumbers, driver::request::Request>:
				{
					object["type"] = "select_numbers";
					break;
				}
				case utility::variant_index_v<driver::request::OrderCards, driver::request::Request>:
				{
					object["type"] = "order_cards";
					break;
				}
				case utility::variant_index_v<driver::request::SelectMonsterZoneLocation, driver::request::Request>:
				{
					object["type"] = "select_monster_zone_location";
					break;
				}
				case utility::variant_index_v<driver::request::SelectAnyMonsterZoneLocation, driver::request::Request>:
				{
					object["type"] = "select_any_monster_zone_location";
					break;
				}
				case utility::variant_index_v<driver::request::SelectMonsterPosition, driver::request::Request>:
				{
					object["type"] = "select_monster_position";
					break;
				}
				case utility::variant_index_v<driver::request::SelectSpellTrapLocation, driver::request::Request>:
				{
					object["type"] = "select_spell_trap_location";
					break;
				}
				case utility::variant_index_v<driver::request::QueryPositionChangeableCards, driver::request::Request>:
				{
					object["type"] = "query_position_changeable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryPositionChangeablePositions, driver::request::Request>:
				{
					object["type"] = "query_position_changeable_positions";
					break;
				}
				case utility::variant_index_v<driver::request::QueryNormalSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_normal_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryNormalSummonType, driver::request::Request>:
				{
					object["type"] = "query_normal_summon_type";
					break;
				}
				case utility::variant_index_v<driver::request::CheckNormalSummonType, driver::request::Request>:
				{
					object["type"] = "check_normal_summon_type";
					break;
				}
				case utility::variant_index_v<driver::request::QueryNormalSummonLocation, driver::request::Request>:
				{
					object["type"] = "query_normal_summon_location";
					break;
				}
				case utility::variant_index_v<driver::request::QueryNormalSummonPlacement, driver::request::Request>:
				{
					object["type"] = "query_normal_summon_placement";
					break;
				}
				case utility::variant_index_v<driver::request::QueryTributeSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_tribute_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryTributeSummonTributeCandidates, driver::request::Request>:
				{
					object["type"] = "query_tribute_summon_tribute_candidates";
					break;
				}
				case utility::variant_index_v<driver::request::QueryTributeSummonType, driver::request::Request>:
				{
					object["type"] = "query_tribute_summon_type";
					break;
				}
				case utility::variant_index_v<driver::request::CheckTributeSummonType, driver::request::Request>:
				{
					object["type"] = "check_tribute_summon_type";
					break;
				}
				case utility::variant_index_v<driver::request::SelectTributeSummonTributes, driver::request::Request>:
				{
					object["type"] = "select_tribute_summon_tributes";
					break;
				}
				case utility::variant_index_v<driver::request::QueryTributeSummonLocation, driver::request::Request>:
				{
					object["type"] = "query_tribute_summon_location";
					break;
				}
				case utility::variant_index_v<driver::request::QueryFlipSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_flip_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryGeminiSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_gemini_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryExtraDeckSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_extra_deck_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryExtraDeckSummonMaterialSources, driver::request::Request>:
				{
					object["type"] = "query_extra_deck_summon_material_sources";
					break;
				}
				case utility::variant_index_v<driver::request::QueryExtraDeckSummonMaterials, driver::request::Request>:
				{
					object["type"] = "query_extra_deck_summon_materials";
					break;
				}
				case utility::variant_index_v<driver::request::QueryExtraDeckSummonLocation, driver::request::Request>:
				{
					object["type"] = "query_extra_deck_summon_location";
					break;
				}
				case utility::variant_index_v<driver::request::QueryPendulumSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_pendulum_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QuerySpecialSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_special_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QuerySpecialSummonLocation, driver::request::Request>:
				{
					object["type"] = "query_special_summon_location";
					break;
				}
				case utility::variant_index_v<driver::request::QuerySpecialSummonPlacements, driver::request::Request>:
				{
					object["type"] = "query_special_summon_placements";
					break;
				}
				case utility::variant_index_v<driver::request::QuerySetSpellTrapCards, driver::request::Request>:
				{
					object["type"] = "query_set_spell_trap_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QuerySpellTrapSetLocation, driver::request::Request>:
				{
					object["type"] = "query_spell_trap_set_location";
					break;
				}
				case utility::variant_index_v<driver::request::QueryActivatePendulumCards, driver::request::Request>:
				{
					object["type"] = "query_activate_pendulum_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryActivatePendulumSide, driver::request::Request>:
				{
					object["type"] = "query_activate_pendulum_side";
					break;
				}
				case utility::variant_index_v<driver::request::QueryActivatableEffects, driver::request::Request>:
				{
					object["type"] = "query_activatable_effects";
					break;
				}
				case utility::variant_index_v<driver::request::QueryEffectTargets, driver::request::Request>:
				{
					object["type"] = "query_effect_targets";
					break;
				}
				case utility::variant_index_v<driver::request::SelectEffectActivation, driver::request::Request>:
				{
					object["type"] = "select_effect_activation";
					break;
				}
				case utility::variant_index_v<driver::request::SelectEffectTargets, driver::request::Request>:
				{
					object["type"] = "select_effect_targets";
					break;
				}
				case utility::variant_index_v<driver::request::QueryChainableEffects, driver::request::Request>:
				{
					object["type"] = "query_chainable_effects";
					break;
				}
				case utility::variant_index_v<driver::request::SelectChain, driver::request::Request>:
				{
					object["type"] = "select_chain";
					break;
				}
				case utility::variant_index_v<driver::request::QueryAttackableCards, driver::request::Request>:
				{
					object["type"] = "query_attackable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::CheckDirectAttackable, driver::request::Request>:
				{
					object["type"] = "check_direct_attackable";
					break;
				}
				case utility::variant_index_v<driver::request::QueryAttackTargets, driver::request::Request>:
				{
					object["type"] = "query_attack_targets";
					break;
				}
				case utility::variant_index_v<driver::request::SelectAttack, driver::request::Request>:
				{
					object["type"] = "select_attack";
					break;
				}
				case utility::variant_index_v<driver::request::QueryPhaseTransitions, driver::request::Request>:
				{
					object["type"] = "query_phase_transitions";
					break;
				}
				case utility::variant_index_v<driver::request::SelectPhaseTransition, driver::request::Request>:
				{
					object["type"] = "select_phase_transition";
					break;
				}
				case utility::variant_index_v<driver::request::CancelSelection, driver::request::Request>:
				{
					object["type"] = "cancel_selection";
					break;
				}
				case utility::variant_index_v<driver::request::ConfirmSelection, driver::request::Request>:
				{
					object["type"] = "confirm_selection";
					break;
				}
				case utility::variant_index_v<driver::request::PassResponse, driver::request::Request>:
				{
					object["type"] = "pass_response";
					break;
				}
				default: {}
			}

			return object;
		}
	};
}

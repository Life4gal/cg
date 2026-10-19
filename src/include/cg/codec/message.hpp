#pragma once

#include <cg/server/message.hpp>

#include <cg/codec/request.hpp>
#include <cg/codec/response.hpp>

namespace cg::codec
{
	// 卡组
	template<>
	class ValueCodec<server::Deck>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// filename
			if (!json.contains("filename") || !json.at("filename").is_string())
			{
				return false;
			}

			// name
			if (!json.contains("name") || !json.at("name").is_string())
			{
				return false;
			}

			// main
			// if (!json.contains("main") || !ValueCodec<std::vector<domain::CardCode>>::valid(json.at("main")))
			if (!json.contains("main") || !json.at("main").is_array())
			{
				return false;
			}

			// extra
			// if (json.contains("extra") && !ValueCodec<std::vector<domain::CardCode>>::valid(json.at("extra")))
			if (json.contains("extra") && !json.at("extra").is_array())
			{
				return false;
			}

			// side
			// if (json.contains("side") && !ValueCodec<std::vector<domain::CardCode>>::valid(json.at("side")))
			if (json.contains("side") && !json.at("side").is_array())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::Deck>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto filename_it = json.find("filename");
			auto filename = filename_it.value().get<nlohmann::json::string_t>();
			const auto name_it = json.find("name");
			auto name = name_it.value().get<nlohmann::json::string_t>();
			const auto main_it = json.find("main");
			auto main = ValueCodec<std::vector<domain::CardCode>>::decode(main_it.value());
			if (!main.has_value())
			{
				return std::nullopt;
			}
			std::vector<domain::CardCode> extra;
			if (const auto extra_it = json.find("extra");
				extra_it != json.end())
			{
				auto extra_result = ValueCodec<std::vector<domain::CardCode>>::decode(extra_it.value());
				if (!extra_result.has_value())
				{
					return std::nullopt;
				}

				extra = *std::move(extra_result);
			}
			std::vector<domain::CardCode> side;
			if (const auto side_it = json.find("side");
				side_it != json.end())
			{
				auto side_result = ValueCodec<std::vector<domain::CardCode>>::decode(side_it.value());
				if (!side_result.has_value())
				{
					return std::nullopt;
				}

				side = *std::move(side_result);
			}

			return server::Deck
			{
					.filename = std::move(filename),
					.name = std::move(name),
					.main_deck = *std::move(main),
					.extra_deck = std::move(extra),
					.side_deck = std::move(side),
			};
		}

		[[nodiscard]] static auto encode(const server::Deck& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["filename"] = data.filename;
			object["name"] = data.name;
			object["main"] = ValueCodec<std::vector<domain::CardCode>>::encode(data.main_deck);
			object["extra"] = ValueCodec<std::vector<domain::CardCode>>::encode(data.extra_deck);
			object["side"] = ValueCodec<std::vector<domain::CardCode>>::encode(data.side_deck);

			return object;
		}
	};

	// 加入房间
	template<>
	class ValueCodec<server::message::Join>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// seat -- 可缺省/可为null,表示自动分配
			if (json.contains("seat") && !ValueCodec<std::optional<domain::Player>>::valid(json.at("seat")))
			{
				return false;
			}

			// session
			if (json.contains("session") && !json.at("session").is_string())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::Join>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			std::optional<domain::Player> seat;
			if (const auto seat_it = json.find("seat"); seat_it != json.end())
			{
				auto seat_result = ValueCodec<std::optional<domain::Player>>::decode(seat_it.value());
				if (!seat_result.has_value())
				{
					return std::nullopt;
				}
				seat = *seat_result;
			}

			std::string session;
			if (const auto session_it = json.find("session"); session_it != json.end())
			{
				session = session_it.value().get<nlohmann::json::string_t>();
			}

			return server::message::Join{.seat = seat, .session = std::move(session)};
		}

		[[nodiscard]] static auto encode(const server::message::Join& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["seat"] = ValueCodec<std::optional<domain::Player>>::encode(data.seat);
			object["session"] = data.session;

			return object;
		}
	};

	// 选择卡组
	template<>
	class ValueCodec<server::message::ChooseDeck>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// deck
			if (!json.contains("deck") || !json.at("deck").is_string())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::ChooseDeck>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto deck_it = json.find("deck");
			auto deck = deck_it.value().get<nlohmann::json::string_t>();

			return server::message::ChooseDeck{.deck = std::move(deck)};
		}

		[[nodiscard]] static auto encode(const server::message::ChooseDeck& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["deck"] = data.deck;

			return object;
		}
	};

	// 投降
	template<>
	class ValueCodec<server::message::Surrender>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::Surrender>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			return server::message::Surrender{};
		}

		[[nodiscard]] static auto encode(const server::message::Surrender& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			return object;
		}
	};

	// 房间同步请求
	template<>
	class ValueCodec<server::message::Sync>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_object();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::Sync>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			return server::message::Sync{};
		}

		[[nodiscard]] static auto encode(const server::message::Sync& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			return object;
		}
	};

	// 决斗回应
	template<>
	class ValueCodec<server::message::Response>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			if (!json.contains("request_id") || !json.at("request_id").is_number_unsigned())
			{
				return false;
			}

			if (!json.contains("payload") || !ValueCodec<driver::response::Response>::valid(json.at("payload")))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::Response>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto request_id = json.at("request_id").get<nlohmann::json::number_unsigned_t>();
			auto payload = ValueCodec<driver::response::Response>::decode(json.at("payload"));
			if (!payload.has_value())
			{
				return std::nullopt;
			}

			return server::message::Response{.request_id = request_id, .payload = *std::move(payload)};
		}

		[[nodiscard]] static auto encode(const server::message::Response& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["request_id"] = data.request_id;
			object["payload"] = ValueCodec<driver::response::Response>::encode(data.payload);

			return object;
		}
	};

	// 座位分配成功
	template<>
	class ValueCodec<server::message::Joined>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			if (!json.contains("seat") || !ValueCodec<domain::Player>::valid(json.at("seat")))
			{
				return false;
			}

			if (json.contains("session") && !json.at("session").is_string())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::Joined>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto seat = ValueCodec<domain::Player>::decode(json.at("seat"));
			if (!seat.has_value())
			{
				return std::nullopt;
			}

			std::string session;
			if (const auto session_it = json.find("session"); session_it != json.end())
			{
				session = session_it.value().get<nlohmann::json::string_t>();
			}

			return server::message::Joined{.seat = *seat, .session = std::move(session)};
		}

		[[nodiscard]] static auto encode(const server::message::Joined& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["seat"] = ValueCodec<domain::Player>::encode(data.seat);
			object["session"] = data.session;

			return object;
		}
	};

	template<>
	class ValueCodec<server::message::LobbyState::Seat>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// empty => 未选择卡组
			if (!json.contains("deck_name") || !json.at("deck_name").is_string())
			{
				return false;
			}

			// ready = 已选择卡组 + 已准备
			if (!json.contains("ready") || !json.at("ready").is_boolean())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::LobbyState::Seat>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto deck_name_it = json.find("deck_name");
			auto deck_name = deck_name_it.value().get<nlohmann::json::string_t>();
			const auto ready_it = json.find("ready");
			const auto ready = ready_it.value().get<nlohmann::json::boolean_t>();

			return server::message::LobbyState::Seat{.deck_name = std::move(deck_name), .ready = ready};
		}

		[[nodiscard]] static auto encode(const server::message::LobbyState::Seat& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["deck_name"] = data.deck_name;
			object["ready"] = data.ready;

			return object;
		}
	};

	// 大厅状态
	template<>
	class ValueCodec<server::message::LobbyState>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// decks
			// if (!json.contains("decks") || !ValueCodec<std::vector<server::Deck>>::valid(json.at("decks")))
			if (!json.contains("decks") || !json.at("decks").is_array())
			{
				return false;
			}

			// seats
			// if (!json.contains("seats") || !ValueCodec<server::message::LobbyState::seats_type>::valid(json.at("seats")))
			if (!json.contains("seats") || !json.at("seats").is_array())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::LobbyState>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto decks_it = json.find("decks");
			auto decks = ValueCodec<std::vector<server::Deck>>::decode(decks_it.value());
			if (!decks.has_value())
			{
				return std::nullopt;
			}
			const auto seats_it = json.find("seats");
			auto seats = ValueCodec<server::message::LobbyState::seats_type>::decode(seats_it.value());
			if (!seats.has_value())
			{
				return std::nullopt;
			}

			return server::message::LobbyState{.decks = *std::move(decks), .seats = *std::move(seats)};
		}

		[[nodiscard]] static auto encode(const server::message::LobbyState& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["decks"] = ValueCodec<std::vector<server::Deck>>::encode(data.decks);
			object["seats"] = ValueCodec<std::array<server::message::LobbyState::Seat, server::message::LobbyState::seats_size>>::encode(data.seats);

			return object;
		}
	};

	template<>
	class ValueCodec<server::message::RoomSync::Seat>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// empty => 未选择卡组
			if (!json.contains("deck_name") || !json.at("deck_name").is_string())
			{
				return false;
			}

			// ready = 已选择卡组 + 已准备
			if (!json.contains("ready") || !json.at("ready").is_boolean())
			{
				return false;
			}

			// joined = 已有玩家加入
			if (!json.contains("joined") || !json.at("joined").is_boolean())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::RoomSync::Seat>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto deck_name_it = json.find("deck_name");
			auto deck_name = deck_name_it.value().get<nlohmann::json::string_t>();
			const auto ready_it = json.find("ready");
			const auto ready = ready_it.value().get<nlohmann::json::boolean_t>();
			const auto joined_it = json.find("joined");
			const auto joined = joined_it.value().get<nlohmann::json::boolean_t>();

			return server::message::RoomSync::Seat{.deck_name = std::move(deck_name), .ready = ready, .joined = joined};
		}

		[[nodiscard]] static auto encode(const server::message::RoomSync::Seat& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["deck_name"] = data.deck_name;
			object["ready"] = data.ready;
			object["joined"] = data.joined;

			return object;
		}
	};

	// 房间同步
	template<>
	class ValueCodec<server::message::RoomSync>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			if (json.contains("started") && !json.at("started").is_boolean())
			{
				return false;
			}
			if (json.contains("finished") && !json.at("finished").is_boolean())
			{
				return false;
			}

			// seats
			if (!json.contains("seats") || !json.at("seats").is_array())
			{
				return false;
			}

			if (json.contains("lobby") && !ValueCodec<server::message::LobbyState>::valid(json.at("lobby")))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::RoomSync>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			bool started = false;
			if (const auto started_it = json.find("started"); started_it != json.end())
			{
				started = started_it.value().get<nlohmann::json::boolean_t>();
			}
			bool finished = false;
			if (const auto finished_it = json.find("finished"); finished_it != json.end())
			{
				finished = finished_it.value().get<nlohmann::json::boolean_t>();
			}

			const auto seats_it = json.find("seats");
			auto seats = ValueCodec<server::message::RoomSync::seats_type>::decode(seats_it.value());
			if (!seats.has_value())
			{
				return std::nullopt;
			}

			std::optional<server::message::LobbyState> lobby;
			if (const auto lobby_it = json.find("lobby"); lobby_it != json.end())
			{
				auto lobby_result = ValueCodec<server::message::LobbyState>::decode(lobby_it.value());
				if (!lobby_result.has_value())
				{
					return std::nullopt;
				}
				lobby = *std::move(lobby_result);
			}

			return server::message::RoomSync{.started = started, .finished = finished, .seats = *std::move(seats), .lobby = std::move(lobby)};
		}

		[[nodiscard]] static auto encode(const server::message::RoomSync& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["started"] = data.started;
			object["finished"] = data.finished;
			object["seats"] = ValueCodec<server::message::RoomSync::seats_type>::encode(data.seats);
			if (data.lobby.has_value())
			{
				object["lobby"] = ValueCodec<server::message::LobbyState>::encode(*data.lobby);
			}

			return object;
		}
	};

	// 决斗请求
	template<>
	class ValueCodec<server::message::Request>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			if (!json.contains("request_id") || !json.at("request_id").is_number_unsigned())
			{
				return false;
			}

			if (!json.contains("payload") || !ValueCodec<driver::request::Request>::valid(json.at("payload")))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::Request>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto request_id = json.at("request_id").get<nlohmann::json::number_unsigned_t>();
			auto payload = ValueCodec<driver::request::Request>::decode(json.at("payload"));
			if (!payload.has_value())
			{
				return std::nullopt;
			}

			return server::message::Request{.request_id = request_id, .payload = *std::move(payload)};
		}

		[[nodiscard]] static auto encode(const server::message::Request& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["request_id"] = data.request_id;
			object["payload"] = ValueCodec<driver::request::Request>::encode(data.payload);

			return object;
		}
	};


	// 日志
	template<>
	class ValueCodec<server::message::Log>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			if (!json.contains("sequence") || !json.at("sequence").is_number_unsigned())
			{
				return false;
			}

			// entry
			if (!json.contains("entry") || !json.at("entry").is_object())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::Log>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto sequence = json.at("sequence").get<nlohmann::json::number_unsigned_t>();
			auto entry = json.at("entry");

			return server::message::Log{.sequence = sequence, .entry = std::move(entry)};
		}

		[[nodiscard]] static auto encode(const server::message::Log& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["sequence"] = data.sequence;
			object["entry"] = data.entry;

			return object;
		}
	};

	// 错误
	template<>
	class ValueCodec<server::message::Error>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			if (!json.contains("message") || !json.at("message").is_string())
			{
				return false;
			}

			if (json.contains("source") && !json.at("source").is_string())
			{
				return false;
			}

			if (json.contains("request_id") && !json.at("request_id").is_number_unsigned())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::Error>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			server::message::Error error;
			error.message = json.at("message").get<nlohmann::json::string_t>();
			if (const auto source_it = json.find("source"); source_it != json.end())
			{
				error.source = source_it.value().get<nlohmann::json::string_t>();
			}
			if (const auto request_id_it = json.find("request_id"); request_id_it != json.end())
			{
				error.request_id = request_id_it.value().get<nlohmann::json::number_unsigned_t>();
			}

			return error;
		}

		[[nodiscard]] static auto encode(const server::message::Error& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["message"] = data.message;
			if (data.source.has_value())
			{
				object["source"] = *data.source;
			}
			if (data.request_id.has_value())
			{
				object["request_id"] = *data.request_id;
			}

			return object;
		}
	};


	template<>
	class ValueCodec<server::message::Message>
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

			if (!ValueCodec<VariantWrap<server::message::Message>>::valid(json))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<server::message::Message>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			// 如果有type则按照type匹配
			if (const auto type_it = json.find("type");
				type_it != json.end())
			{
				const auto do_decode = [&]<typename M>() noexcept -> std::optional<server::message::Message>
				{
					if (const auto r = ValueCodec<M>::decode(json);
						r.has_value())
					{
						return server::message::Message{*std::move(r)};
					}

					return std::nullopt;
				};

				const auto& type = type_it.value().get_ref<const nlohmann::json::string_t&>();

				if (type == "join")
				{
					return do_decode.operator()<server::message::Join>();
				}
				if (type == "choose_deck")
				{
					return do_decode.operator()<server::message::ChooseDeck>();
				}
				if (type == "surrender")
				{
					return do_decode.operator()<server::message::Surrender>();
				}
				if (type == "sync")
				{
					return do_decode.operator()<server::message::Sync>();
				}
				if (type == "response")
				{
					return do_decode.operator()<server::message::Response>();
				}
				if (type == "joined")
				{
					return do_decode.operator()<server::message::Joined>();
				}
				if (type == "lobby")
				{
					return do_decode.operator()<server::message::LobbyState>();
				}
				if (type == "room_sync")
				{
					return do_decode.operator()<server::message::RoomSync>();
				}
				if (type == "request")
				{
					return do_decode.operator()<server::message::Request>();
				}
				if (type == "log")
				{
					return do_decode.operator()<server::message::Log>();
				}
				if (type == "error")
				{
					return do_decode.operator()<server::message::Error>();
				}

				// 未知type直接失败
				return std::nullopt;
			}

			// 没有type按照index匹配
			return ValueCodec<VariantWrap<server::message::Message>>::decode(json);
		}

		[[nodiscard]] static auto encode(const server::message::Message& data) noexcept -> nlohmann::json
		{
			auto object = ValueCodec<VariantWrap<server::message::Message>>::encode(data);

			switch (data.index())
			{
				case utility::variant_index_v<server::message::Join, server::message::Message>:
				{
					object["type"] = "join";
					break;
				}
				case utility::variant_index_v<server::message::ChooseDeck, server::message::Message>:
				{
					object["type"] = "choose_deck";
					break;
				}
				case utility::variant_index_v<server::message::Surrender, server::message::Message>:
				{
					object["type"] = "surrender";
					break;
				}
				case utility::variant_index_v<server::message::Sync, server::message::Message>:
				{
					object["type"] = "sync";
					break;
				}
				case utility::variant_index_v<server::message::Response, server::message::Message>:
				{
					object["type"] = "response";
					break;
				}
				case utility::variant_index_v<server::message::Joined, server::message::Message>:
				{
					object["type"] = "joined";
					break;
				}
				case utility::variant_index_v<server::message::LobbyState, server::message::Message>:
				{
					object["type"] = "lobby";
					break;
				}
				case utility::variant_index_v<server::message::RoomSync, server::message::Message>:
				{
					object["type"] = "room_sync";
					break;
				}
				case utility::variant_index_v<server::message::Request, server::message::Message>:
				{
					object["type"] = "request";
					break;
				}
				case utility::variant_index_v<server::message::Log, server::message::Message>:
				{
					object["type"] = "log";
					break;
				}
				case utility::variant_index_v<server::message::Error, server::message::Message>:
				{
					object["type"] = "error";
					break;
				}
				default: {}
			}

			return object;
		}
	};
}

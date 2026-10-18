#include <cg/server/room.hpp>

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <tuple>
#include <utility>

#include <cg/utility/enum.hpp>

#include <spdlog/spdlog.h>

#define ROOM_LOG_PREFIX "[Room]: "

namespace cg::server
{
	auto Room::load_db() noexcept -> bool
	{
		const std::filesystem::path path{options_.cards_path};

		// TODO: 载入卡牌脚本
		std::ignore = path;
		std::ignore = this;

		return true;
	}

	auto Room::load_catalog() noexcept -> bool
	{
		catalog_decks_.clear();

		const std::filesystem::path path{options_.decks_path};

		std::error_code error_code{};
		if (!exists(path, error_code))
		{
			SPDLOG_ERROR(ROOM_LOG_PREFIX "卡组目录{}不存在!({})", options_.decks_path, error_code.message());
			return false;
		}
		if (!is_directory(path, error_code))
		{
			SPDLOG_ERROR(ROOM_LOG_PREFIX "卡组目录{}不是一个文件夹!({})", options_.decks_path, error_code.message());
			return false;
		}

		std::vector<std::filesystem::path> files{};
		for (const auto& entry: std::filesystem::directory_iterator{path, error_code})
		{
			if (!entry.is_regular_file(error_code))
			{
				continue;
			}

			files.push_back(entry.path());
		}

		std::ranges::sort(files);

		for (const auto& file: files)
		{
			if (file.extension() != ".json")
			{
				SPDLOG_WARN(ROOM_LOG_PREFIX "暂不支持加载{}类型卡组文件,跳过文件{}!", file.extension().string(), file.string());
				continue;
			}

			nlohmann::json json;
			try
			{
				std::ifstream in{file};
				json = nlohmann::json::parse(in);
			}
			catch (const nlohmann::json::parse_error& error)
			{
				SPDLOG_ERROR(ROOM_LOG_PREFIX "卡组文件{}解析失败!({})", file.string(), error.what());
				continue;
			}

			if (auto result = Deck::load(json);
				!result.has_value())
			{
				SPDLOG_ERROR(ROOM_LOG_PREFIX "载入卡组文件{}失败!({})", file.string(), result.error());
				continue;
			}
			else
			{
				auto deck = *std::move(result);
				deck.filename = file.stem().string();
				if (deck.name.empty())
				{
					deck.name = deck.filename;
				}

				catalog_decks_.push_back(std::move(deck));
			}

			// TODO: 检查卡组使用的卡牌是否在DB中
			// TODO: 检查卡组使用的卡牌是否满足规则要求

			SPDLOG_INFO(ROOM_LOG_PREFIX "卡组文件{}加载成功!", file.string());
		}

		SPDLOG_INFO(ROOM_LOG_PREFIX "成功加载{}副卡组", catalog_decks_.size());
		return true;
	}

	auto Room::lobby_state_locked() const noexcept -> message::LobbyState
	{
		message::LobbyState lobby_state
		{
				.decks = catalog_decks_,
				.seats = {},
		};

		for (auto [state, seat]: std::views::zip(lobby_state.seats, seats_))
		{
			state.deck_name = seat.deck_name;
			state.ready = seat.ready;
		}

		return lobby_state;
	}

	auto Room::room_sync_locked() const noexcept -> message::RoomSync
	{
		message::RoomSync sync
		{
				.started = duel_started_,
				.finished = duel_finished_,
				.seats = {},
				.lobby = {},
		};

		for (auto [state, seat]: std::views::zip(sync.seats, seats_))
		{
			state.deck_name = seat.deck_name;
			state.ready = seat.ready;
			state.joined = seat.joined;
		}

		if (!duel_started_)
		{
			sync.lobby = lobby_state_locked();
		}

		return sync;
	}

	auto Room::check_ready_locked() noexcept -> bool
	{
		if (duel_started_)
		{
			return false;
		}

		const auto ready = std::ranges::all_of(
			seats_,
			[](const Seat& seat) noexcept -> bool
			{
				return seat.joined && seat.ready;
			}
		);

		if (ready)
		{
			duel_started_ = true;
		}

		return ready;
	}

	auto Room::deliver(const domain::Player player, const message::Message& message) const noexcept -> bool
	{
		Client* client;
		{
			std::scoped_lock lock{mutex_};
			client = seats_[std::to_underlying(player)].client;
		}

		if (client == nullptr)
		{
			return false;
		}

		client->deliver(message);
		return true;
	}

	auto Room::broadcast(const message::Message& message) const noexcept -> void
	{
		std::array<Client*, max_peer> clients{};
		{
			std::scoped_lock lock{mutex_};
			for (std::size_t index = 0; index < max_peer; ++index)
			{
				clients[index] = seats_[index].client;
			}
		}

		for (auto* client: clients)
		{
			if (client != nullptr)
			{
				client->deliver(message);
			}
		}
	}

	auto Room::send_lobby(const domain::Player player) const noexcept -> void
	{
		message::LobbyState state;
		{
			std::scoped_lock lock{mutex_};
			state = lobby_state_locked();
		}

		deliver(player, message::Message{std::move(state)});
	}

	auto Room::broadcast_lobby() const noexcept -> void
	{
		message::LobbyState state;
		{
			std::scoped_lock lock{mutex_};
			state = lobby_state_locked();
		}

		broadcast(message::Message{std::move(state)});
	}

	auto Room::send_room_sync(const domain::Player player) const noexcept -> void
	{
		message::RoomSync sync;
		{
			std::scoped_lock lock{mutex_};
			sync = room_sync_locked();
		}

		deliver(player, message::Message{std::move(sync)});
	}

	auto Room::broadcast_room_sync() const noexcept -> void
	{
		message::RoomSync sync;
		{
			std::scoped_lock lock{mutex_};
			sync = room_sync_locked();
		}

		broadcast(message::Message{std::move(sync)});
	}

	auto Room::send_log(const nlohmann::json& entry) noexcept -> void
	{
		std::array<Client*, max_peer> clients{};
		std::uint64_t sequence;
		{
			std::scoped_lock lock{mutex_};
			sequence_ += 1;
			sequence = sequence_;

			for (std::size_t index = 0; index < max_peer; ++index)
			{
				clients[index] = seats_[index].client;
			}
		}

		const message::Message log{message::Log{.sequence = sequence, .entry = entry}};
		for (auto* client: clients)
		{
			if (client != nullptr)
			{
				client->deliver(log);
			}
		}
	}

	auto Room::send_error(const domain::Player player, const message::Error& error) const noexcept -> void
	{
		deliver(player, message::Message{error});
	}

	auto Room::handle_choose_deck(const domain::Player player, std::string deck) noexcept -> void
	{
		std::string error;
		bool ready = false;
		{
			std::scoped_lock lock{mutex_};

			if (auto& seat = seats_[std::to_underlying(player)];
				!seat.joined)
			{
				error = "not joined";
			}
			else if (duel_started_)
			{
				error = "duel already started; deck selection is closed";
			}
			else
			{
				if (const auto deck_it = std::ranges::find(catalog_decks_, deck, &Deck::filename);
					deck_it == catalog_decks_.end())
				{
					error = std::format("unknown deck \"{}\"", deck);
				}
				else
				{
					seat.ready = true;
					seat.deck_name = std::move(deck);
					seat.main_deck = deck_it->main_deck;
					seat.extra_deck = deck_it->extra_deck;
					seat.side_deck = deck_it->side_deck;

					ready = check_ready_locked();
				}
			}
		}

		if (!error.empty())
		{
			send_error(player, message::Error{.message = std::move(error), .source = std::nullopt, .request_id = std::nullopt});
			return;
		}

		broadcast_lobby();

		if (ready)
		{
			start_duel_async();
		}
	}

	auto Room::handle_surrender(const domain::Player player) noexcept -> void
	{
		bool joined;
		{
			std::scoped_lock lock{mutex_};

			joined = seats_[std::to_underlying(player)].joined;
			if (joined && duel_started_ && !duel_finished_ && !pending_concede_.has_value())
			{
				pending_concede_ = player;
			}
		}

		if (!joined)
		{
			send_error(player, message::Error{.message = "not joined", .source = std::nullopt, .request_id = std::nullopt});
			return;
		}

		condition_variable_.notify_all();

		auto entry = nlohmann::json::object();
		entry["event"] = "surrender";
		entry["player"] = std::string{utility::to_string(player)};

		send_log(entry);
	}

	auto Room::handle_sync(const domain::Player player) const noexcept -> void
	{
		message::RoomSync sync;
		std::optional<message::Request> request;
		bool joined;
		{
			std::scoped_lock lock{mutex_};
			const auto& seat = seats_[std::to_underlying(player)];
			joined = seat.joined;

			if (joined)
			{
				sync = room_sync_locked();
				if (waiting_seat_.has_value() && *waiting_seat_ == player && seat.last_request.has_value())
				{
					request = seat.last_request;
				}
			}
		}

		if (!joined)
		{
			send_error(player, message::Error{.message = "not joined", .source = std::nullopt, .request_id = std::nullopt});
			return;
		}

		deliver(player, message::Message{std::move(sync)});

		if (request.has_value())
		{
			deliver(player, message::Message{*std::move(request)});
		}
	}

	auto Room::handle_response(const domain::Player player, const std::uint64_t request_id, driver::response::Response response) noexcept -> bool
	{
		std::string error;
		bool duplicate = false;
		{
			std::scoped_lock lock{mutex_};

			if (const auto& seat = seats_[std::to_underlying(player)];
				!seat.joined)
			{
				error = "not joined";
			}
			else if (!waiting_seat_.has_value() || *waiting_seat_ != player)
			{
				error = "no request is pending for this seat";
			}
			else if (request_id != waiting_request_id_)
			{
				error = "stale or unknown request_id";
			}
			else if (pending_response_.has_value())
			{
				duplicate = true;
			}
			else
			{
				// TODO: 校验response的kind与当前waiting request的kind是否匹配
				pending_response_ = std::move(response);
			}
		}

		if (duplicate)
		{
			return false;
		}

		if (!error.empty())
		{
			send_error(player, message::Error{.message = std::move(error), .source = "response", .request_id = request_id});
			return false;
		}

		condition_variable_.notify_all();
		return true;
	}

	auto Room::start_duel_async() noexcept -> void
	{
		std::scoped_lock thread_lock{duel_thread_mutex_};
		if (duel_thread_.joinable())
		{
			duel_thread_.join();
		}

		duel_thread_ = std::thread
		{
				[this] noexcept -> void
				{
					start_duel();
				}
		};
	}

	auto Room::start_duel() noexcept -> void
	{
		try
		{
			// TODO: 创建对局并运行
			// auto game = state::Game{...};
			// auto driver = RoomDriver{*this};
			// ... flow::run(game, driver) ...

			{
				std::scoped_lock lock{mutex_};
				duel_finished_ = true;
			}
			condition_variable_.notify_all();

			auto entry = nlohmann::json::object();
			entry["event"] = "duel_finished";

			send_log(entry);
		}
		catch (const DuelConceded& conceded)
		{
			{
				std::scoped_lock lock{mutex_};
				duel_finished_ = true;
			}
			condition_variable_.notify_all();

			auto entry = nlohmann::json::object();
			entry["event"] = "duel_conceded";
			entry["player"] = std::string{utility::to_string(conceded.player)};

			send_log(entry);
		}
		catch (const DuelAborted&)
		{
			{
				std::scoped_lock lock{mutex_};
				duel_finished_ = true;
			}
			condition_variable_.notify_all();

			auto entry = nlohmann::json::object();
			entry["event"] = "duel_aborted";

			send_log(entry);
		}
		catch (const std::exception& exception)
		{
			{
				std::scoped_lock lock{mutex_};
				duel_finished_ = true;
			}
			condition_variable_.notify_all();

			broadcast(message::Message{message::Error{.message = std::format("duel error: {}", exception.what()), .source = std::nullopt, .request_id = std::nullopt}});
		}
		catch (...)
		{
			{
				std::scoped_lock lock{mutex_};
				duel_finished_ = true;
			}
			condition_variable_.notify_all();

			broadcast(message::Message{message::Error{.message = "unexpected duel error", .source = std::nullopt, .request_id = std::nullopt}});
		}
	}

	auto Room::wait_for_response() -> driver::response::Response
	{
		std::unique_lock lock{mutex_};
		condition_variable_.wait(
			lock,
			[this] noexcept -> bool
			{
				return pending_response_.has_value() || pending_concede_.has_value() || duel_abort_ || stopping_;
			}
		);

		if (duel_abort_ || stopping_)
		{
			throw DuelAborted{};
		}

		if (pending_concede_.has_value())
		{
			throw DuelConceded{*pending_concede_};
		}

		auto response = *std::move(pending_response_);
		pending_response_.reset();
		waiting_seat_.reset();

		return response;
	}

	Room::Room(RoomOptions options) noexcept
		: options_{std::move(options)}
	{
		//
	}

	Room::~Room() noexcept
	{
		stop();
	}

	auto Room::initialize() noexcept -> bool
	{
		if (!load_db())
		{
			return false;
		}
		if (!load_catalog())
		{
			return false;
		}

		return true;
	}

	auto Room::decks() const noexcept -> std::vector<Deck>
	{
		std::scoped_lock lock{mutex_};
		return catalog_decks_;
	}

	auto Room::join(Client& client, const std::optional<domain::Player> preferred, std::string session) noexcept -> std::optional<domain::Player>
	{
		std::optional<domain::Player> assigned;
		{
			std::scoped_lock lock{mutex_};

			// 已是房间成员
			for (std::size_t index = 0; index < max_peer; ++index)
			{
				if (seats_[index].client == &client)
				{
					seats_[index].session = session;
					assigned = static_cast<domain::Player>(index);
					break;
				}
			}

			if (!assigned.has_value() && !duel_started_)
			{
				if (preferred.has_value())
				{
					if (const auto index = std::to_underlying(*preferred);
						index < max_peer && !seats_[index].joined)
					{
						assigned = *preferred;
					}
				}
				else
				{
					for (std::size_t index = 0; index < max_peer; ++index)
					{
						if (!seats_[index].joined)
						{
							assigned = static_cast<domain::Player>(index);
							break;
						}
					}
				}
			}

			if (!assigned.has_value())
			{
				return std::nullopt;
			}

			auto& seat = seats_[std::to_underlying(*assigned)];
			seat.client = &client;
			seat.joined = true;
			seat.session = session;
		}

		deliver(*assigned, message::Message{message::Joined{.seat = *assigned, .session = std::move(session)}});
		send_lobby(*assigned);
		broadcast_room_sync();

		return assigned;
	}

	auto Room::leave(const domain::Player player) noexcept -> void
	{
		bool abort = false;
		{
			std::scoped_lock lock{mutex_};

			auto& [client, joined, ready, session, deck_name, main_deck, extra_deck, side_deck, last_request] = seats_[std::to_underlying(player)];
			client = nullptr;
			joined = false;
			ready = false;
			session.clear();
			deck_name.clear();
			main_deck.clear();
			extra_deck.clear();
			side_deck.clear();
			last_request.reset();

			if (duel_started_ && !duel_finished_)
			{
				duel_abort_ = true;
				abort = true;
			}
		}

		if (abort)
		{
			condition_variable_.notify_all();
		}

		broadcast_room_sync();
	}

	auto Room::seat_of(const Client& client) const noexcept -> domain::Player
	{
		std::scoped_lock lock{mutex_};

		for (std::size_t index = 0; index < max_peer; ++index)
		{
			if (seats_[index].client == &client)
			{
				return static_cast<domain::Player>(index);
			}
		}

		return invalid_seat;
	}

	auto Room::is_joined(const domain::Player player) const noexcept -> bool
	{
		if (player == invalid_seat)
		{
			return false;
		}

		std::scoped_lock lock{mutex_};
		return seats_[std::to_underlying(player)].joined;
	}

	auto Room::lobby_state() const noexcept -> message::LobbyState
	{
		std::scoped_lock lock{mutex_};
		return lobby_state_locked();
	}

	auto Room::dispatch(const domain::Player player, const message::ChooseDeck& message) noexcept -> void
	{
		handle_choose_deck(player, message.deck);
	}

	auto Room::dispatch(const domain::Player player, const message::Surrender&) noexcept -> void
	{
		handle_surrender(player);
	}

	auto Room::dispatch(const domain::Player player, const message::Sync&) const noexcept -> void
	{
		handle_sync(player);
	}

	auto Room::dispatch(const domain::Player player, const message::Response& message) noexcept -> void
	{
		std::ignore = handle_response(player, message.request_id, message.payload);
	}

	auto Room::decide(const driver::request::Request& request) -> driver::response::Response
	{
		const auto player = driver::request::player_of(request);

		std::uint64_t request_id;
		{
			std::scoped_lock lock{mutex_};

			if (!seats_[std::to_underlying(player)].joined)
			{
				throw DuelAborted{};
			}

			next_request_id_ += 1;
			request_id = next_request_id_;
			waiting_request_id_ = request_id;
			waiting_seat_ = player;
			pending_response_.reset();

			seats_[std::to_underlying(player)].last_request = message::Request{.request_id = request_id, .payload = request};
		}

		{
			auto entry = nlohmann::json::object();
			entry["event"] = "request";
			entry["request_id"] = request_id;
			entry["player"] = std::string{utility::to_string(player)};

			send_log(entry);
		}

		if (!deliver(player, message::Message{message::Request{.request_id = request_id, .payload = request}}))
		{
			throw DuelAborted{};
		}

		auto response = wait_for_response();

		{
			auto entry = nlohmann::json::object();
			entry["event"] = "response";
			entry["request_id"] = request_id;
			entry["player"] = std::string{utility::to_string(player)};

			send_log(entry);
		}

		return response;
	}

	auto Room::wait() noexcept -> void
	{
		{
			std::unique_lock lock{mutex_};
			condition_variable_.wait(
				lock,
				[this] noexcept -> bool
				{
					return stopping_ || duel_finished_;
				}
			);
		}

		std::scoped_lock thread_lock{duel_thread_mutex_};
		if (duel_thread_.joinable())
		{
			duel_thread_.join();
		}
	}

	auto Room::stop() noexcept -> void
	{
		{
			std::scoped_lock lock{mutex_};
			stopping_ = true;
			duel_abort_ = true;
		}

		condition_variable_.notify_all();

		std::scoped_lock thread_lock{duel_thread_mutex_};
		if (duel_thread_.joinable())
		{
			duel_thread_.join();
		}
	}
}

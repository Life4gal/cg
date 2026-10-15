#include <cg/server/net_server.hpp>

#include <algorithm>
#include <fstream>
#include <filesystem>
#include <ranges>
#include <print>

#include <cg/utility/enum.hpp>
#include <cg/driver/driver.hpp>
#include <cg/server/deck_loader.hpp>
#include <cg/server/codec.hpp>

#include <spdlog/spdlog.h>
#include <libassert/assert.hpp>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXHttpServer.h>

#define NS_LOG_PREFIX "[NetServer]: "

namespace cg::server
{
	namespace
	{
		[[nodiscard]] auto mime_type(const std::filesystem::path& extension) noexcept -> const char*
		{
			if (extension == ".html")
			{
				return "text/html; charset=utf-8";
			}
			if (extension == ".js")
			{
				return "text/javascript; charset=utf-8";
			}
			if (extension == ".css")
			{
				return "text/css; charset=utf-8";
			}
			if (extension == ".json")
			{
				return "application/json; charset=utf-8";
			}
			if (extension == ".svg")
			{
				return "image/svg+xml";
			}
			if (extension == ".png")
			{
				return "image/png";
			}
			if (extension == ".jpg" || extension == ".jpeg")
			{
				return "image/jpeg";
			}
			if (extension == ".ico")
			{
				return "image/x-icon";
			}
			return "application/octet-stream";
		}

		class DuelAborted final : public std::exception
		{
		public:
			using exception::exception;
		};

		class DuelConceded final : public std::exception
		{
		public:
			domain::Player player;

			explicit DuelConceded(const domain::Player player) noexcept
				: player{player} {}
		};
	}

	class NetServer::NetDriver final : public driver::Driver
	{
	public:
		std::reference_wrapper<NetServer> server;

		explicit NetDriver(NetServer& server) noexcept
			: server{server} {}

		auto decide(const driver::Request& request) noexcept -> driver::Response override
		{
			auto& s = server.get();

			return s.next_response(request);
		}
	};

	auto NetServer::load_db() noexcept -> bool
	{
		const std::filesystem::path path{options_.cards_path};

		// TODO: 载入卡牌脚本
		std::ignore = path;
		std::ignore = this;

		return true;
	}

	auto NetServer::load_catalog() noexcept -> bool
	{
		catalog_decks_.clear();

		const std::filesystem::path path{options_.decks_path};

		std::error_code error_code{};
		if (!exists(path, error_code))
		{
			SPDLOG_ERROR(NS_LOG_PREFIX "卡组目录{}不存在!({})", options_.decks_path, error_code.message());
			return false;
		}
		if (!is_directory(path, error_code))
		{
			SPDLOG_ERROR(NS_LOG_PREFIX "卡组目录{}不是一个文件夹!({})", options_.decks_path, error_code.message());
			return false;
		}

		const auto directory_iterator = std::filesystem::directory_iterator{path, error_code};
		if (error_code != std::error_code{})
		{
			SPDLOG_ERROR(NS_LOG_PREFIX "无法遍历卡组目录{}!({})", options_.decks_path, error_code.message());
			return false;
		}

		const auto file_count = std::ranges::count_if(
			directory_iterator,
			// &std::filesystem::directory_entry::is_regular_file
			[](const std::filesystem::directory_entry& entry) noexcept -> bool
			{
				std::error_code ec{};
				if (!entry.is_regular_file(ec))
				{
					return false;
				}
				if (ec != std::error_code{})
				{
					SPDLOG_ERROR(NS_LOG_PREFIX "文件{}检查失败!({})", entry.path().string(), ec.message());
					return false;
				}
				return true;
			}
		);

		std::vector<std::filesystem::path> files{};
		files.reserve(file_count);

		for (const auto& entry: directory_iterator)
		{
			if (!entry.is_regular_file(error_code))
			{
				continue;
			}

			files.push_back(entry.path());
		}

		std::ranges::sort(files);

		catalog_decks_.reserve(file_count);
		for (const auto& file: files)
		{
			if (file.extension() == ".json")
			{
				nlohmann::json json;
				try
				{
					std::ifstream in{file};
					json = nlohmann::json::parse(in);
				}
				catch (const nlohmann::json::parse_error& error)
				{
					SPDLOG_ERROR(NS_LOG_PREFIX "卡组文件{}解析失败!({})", file.string(), error.what());
					continue;
				}

				if (auto result = load_json_deck(json);
					!result.has_value())
				{
					SPDLOG_ERROR(NS_LOG_PREFIX "载入卡组文件{}失败!({})", file.string(), result.error());
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
			}
			else
			{
				SPDLOG_WARN(NS_LOG_PREFIX "暂不支持加载{}类型卡组文件,跳过文件{}!", file.extension().string(), file.string());
				continue;
			}

			// TODO: 检查卡组使用的卡牌是否在DB中
			// TODO: 检查卡组使用的卡牌是否满足规则要求

			SPDLOG_INFO(NS_LOG_PREFIX "卡组文件{}加载成功!", file.string());
		}

		SPDLOG_INFO(NS_LOG_PREFIX "成功加载{}副卡组", catalog_decks_.size());
		return true;
	}

	auto NetServer::load_web() noexcept -> bool
	{
		// ===================================================
		// WEB
		// ===================================================

		{
			std::filesystem::path path{options_.web_path};

			std::error_code error_code{};
			if (!exists(path, error_code))
			{
				SPDLOG_ERROR(NS_LOG_PREFIX "WEB目录{}不存在!({})", options_.web_path, error_code.message());
				return false;
			}
			if (!is_directory(path, error_code))
			{
				SPDLOG_ERROR(NS_LOG_PREFIX "WEB目录{}不是一个文件夹!({})", options_.web_path, error_code.message());
				return false;
			}

			const auto index_path = path / index_filename;
			if (!exists(index_path, error_code))
			{
				SPDLOG_ERROR(NS_LOG_PREFIX "WEB目录{}不存在{}文件!({})", options_.web_path, index_filename, error_code.message());
				return false;
			}
			if (!is_regular_file(index_path, error_code))
			{
				SPDLOG_ERROR(NS_LOG_PREFIX "{}不是一个文件!({})", index_path.string(), error_code.message());
				return false;
			}

			web_root_ = std::move(path);
		}

		// ===================================================
		// PICS
		// ===================================================

		{
			std::filesystem::path path{options_.pics_path};

			std::error_code error_code{};
			if (!exists(path, error_code))
			{
				SPDLOG_ERROR(NS_LOG_PREFIX "PICS目录{}不存在!({})", options_.pics_path, error_code.message());
				return false;
			}
			if (!is_directory(path, error_code))
			{
				SPDLOG_ERROR(NS_LOG_PREFIX "PICS目录{}不是一个文件夹!({})", options_.pics_path, error_code.message());
				return false;
			}

			pics_root_ = std::move(path);
		}

		// ===================================================
		// ASSETS
		// ===================================================

		{
			std::filesystem::path path{options_.assets_path};

			std::error_code error_code{};
			if (!exists(path, error_code))
			{
				SPDLOG_ERROR(NS_LOG_PREFIX "ASSETS目录{}不存在!({})", options_.assets_path, error_code.message());
				return false;
			}
			if (!is_directory(path, error_code))
			{
				SPDLOG_ERROR(NS_LOG_PREFIX "ASSETS目录{}不是一个文件夹!({})", options_.assets_path, error_code.message());
				return false;
			}

			assets_root_ = std::move(path);
		}

		return true;
	}

	auto NetServer::setup_http_server() noexcept -> bool
	{
		http_server_ = std::make_unique<ix::HttpServer>(options_.port, options_.host);
		http_server_->setOnConnectionCallback(
			[this](const ix::HttpRequestPtr& request, [[maybe_unused]] std::shared_ptr<ix::ConnectionState> state) noexcept -> ix::HttpResponsePtr
			{
				auto response = std::make_shared<ix::HttpResponse>();
				if (request->method != "GET")
				{
					response->statusCode = 405;
					response->description = "Method Not Allowed";
					response->headers["Content-Type"] = "text/plain";
					response->body = "method not allowed";
					return response;
				}

				const char* content_type = "text/plain";
				int status = 200;
				auto body = serve_static(request->uri, content_type, status);

				response->headers["Content-Type"] = content_type;
				response->statusCode = status;
				response->body = std::move(body);

				return response;
			}
		);
		http_server_->setOnClientMessageCallback(
			[this](const std::shared_ptr<ix::ConnectionState>& state, ix::WebSocket& socket, const ix::WebSocketMessagePtr& message) noexcept -> void
			{
				on_message(state, socket, *message);
			}
		);

		return true;
	}

	auto NetServer::serve_static(const std::string_view uri, const char*& content_type, int& status) const noexcept -> std::string
	{
		auto relative = std::string{uri};
		const auto query = relative.find("?");
		if (query != std::string::npos)
		{
			relative = relative.substr(0, query);
		}
		if (relative.empty() || relative == "/")
		{
			relative = "/";
			relative += index_filename;
		}
		if (relative.find("..") != std::string::npos)
		{
			status = 403;
			content_type = "text/plain";
			return "forbidden";
		}

		auto path = relative.substr(1);
		auto base = web_root_;
		if (path.rfind("pics/", 0) == 0)
		{
			base = pics_root_;
			path = path.substr(5);
		}
		else if (path.rfind("assets/", 0) == 0)
		{
			base = assets_root_;
			path = path.substr(7);
		}

		const auto resolved = base / path;
		std::error_code error_code{};
		if (!exists(resolved, error_code))
		{
			SPDLOG_WARN(NS_LOG_PREFIX "请求的资源{}不存在!({})", resolved.string(), error_code.message());

			status = 404;
			content_type = "text/plain";
			return "not found";
		}

		std::ifstream file{resolved, std::ios::binary};
		std::ostringstream buffer{};
		buffer << file.rdbuf();

		status = 200;
		content_type = mime_type(resolved.extension());
		return std::move(buffer).str();
	}

	auto NetServer::get_lobby_payload_locked() const noexcept -> nlohmann::json
	{
		auto decks = nlohmann::json::array();
		for (const auto& [filename, name, main_deck, extra_deck, side_deck]: catalog_decks_)
		{
			auto deck_content = nlohmann::json::object();

			deck_content["filename"] = filename;
			deck_content["name"] = name;
			deck_content["main"] = main_deck.size();
			deck_content["extra"] = extra_deck.size();
			deck_content["side"] = side_deck.size();

			decks.push_back(std::move(deck_content));
		}

		return
		{
				// 可选卡组
				{"deck", std::move(decks)},
				//
		};
	}

	auto NetServer::get_lobby_content_locked() const noexcept -> nlohmann::json
	{
		auto content = get_lobby_payload_locked();
		content["type"] = "lobby";
		return content;
	}

	auto NetServer::check_lobby_both_ready_locked() noexcept -> bool
	{
		if (duel_started_)
		{
			return false;
		}

		const auto ready = std::ranges::all_of(
			peers_,
			[](const Peer& peer) noexcept -> bool
			{
				return peer.joined && peer.chose_deck;
			}
		);

		if (ready)
		{
			duel_started_ = true;
		}

		return ready;
	}

	auto NetServer::get_room_sync_content_locked() const noexcept -> nlohmann::json
	{
		auto content = nlohmann::json::object();

		content["type"] = "sync";
		content["duel_started"] = duel_started_;
		content["duel_finished"] = duel_finished_;
		content["seats"][utility::to_string(domain::Player::FIRST)] = peers_[std::to_underlying(domain::Player::FIRST)].joined;
		content["seats"][utility::to_string(domain::Player::SECOND)] = peers_[std::to_underlying(domain::Player::SECOND)].joined;

		if (!duel_started_)
		{
			content["lobby"] = get_lobby_payload_locked();
		}

		return content;
	}

	auto NetServer::send_lobby(ix::WebSocket& socket) noexcept -> void
	{
		nlohmann::json content;
		{
			std::scoped_lock lock{mutex_};
			content = get_lobby_content_locked();
		}

		socket.send(content.dump());
	}

	auto NetServer::broadcast_lobby() noexcept -> void
	{
		nlohmann::json content;
		{
			std::scoped_lock lock{mutex_};
			content = get_lobby_content_locked();
		}

		broadcast_message(content);
	}

	auto NetServer::broadcast_room_sync() noexcept -> void
	{
		nlohmann::json content;
		{
			std::scoped_lock lock{mutex_};
			content = get_room_sync_content_locked();
		}

		broadcast_message(content);
	}

	auto NetServer::send_log(const nlohmann::json& entry) noexcept -> void
	{
		std::array<std::shared_ptr<ix::WebSocket>, max_peer> sockets;
		std::uint64_t sequence;
		{
			std::scoped_lock lock{mutex_};

			sockets[std::to_underlying(domain::Player::FIRST)] = peers_[std::to_underlying(domain::Player::FIRST)].socket;
			sockets[std::to_underlying(domain::Player::SECOND)] = peers_[std::to_underlying(domain::Player::SECOND)].socket;

			sequence_ += 1;
			sequence = sequence_;
		}

		auto content = nlohmann::json::object();

		content["type"] = "log";
		content["sequence"] = sequence;
		content["entry"] = entry;

		for (const auto data = content.dump();
		     const auto& socket: sockets)
		{
			if (socket)
			{
				socket->send(data);
			}
		}
	}

	auto NetServer::send_error(const domain::Player player, const nlohmann::json& error) noexcept -> void
	{
		auto content = nlohmann::json::object();

		content["type"] = "error";
		content["message"] = error;

		send_message(player, content);
	}

	auto NetServer::socket_for(const ix::WebSocket& socket) noexcept -> std::shared_ptr<ix::WebSocket>
	{
		LIBASSERT_DEBUG_ASSERT(http_server_ != nullptr);
		if (http_server_ == nullptr)
		{
			return nullptr;
		}

		for (const auto& clients = http_server_->getClients();
		     const auto& client: clients)
		{
			if (client.get() == &socket)
			{
				return client;
			}
		}

		return nullptr;
	}

	auto NetServer::send_message(const domain::Player player, const nlohmann::json& message) noexcept -> bool
	{
		std::shared_ptr<ix::WebSocket> socket;
		{
			std::scoped_lock lock{mutex_};
			socket = peers_[std::to_underlying(player)].socket;
		}

		if (socket == nullptr)
		{
			return false;
		}

		socket->send(message.dump());
		return true;
	}

	auto NetServer::broadcast_message(const nlohmann::json& message) noexcept -> void
	{
		std::array<std::shared_ptr<ix::WebSocket>, max_peer> sockets;
		{
			std::scoped_lock lock{mutex_};

			sockets[std::to_underlying(domain::Player::FIRST)] = peers_[std::to_underlying(domain::Player::FIRST)].socket;
			sockets[std::to_underlying(domain::Player::SECOND)] = peers_[std::to_underlying(domain::Player::SECOND)].socket;
		}

		for (const auto data = message.dump();
		     const auto& socket: sockets)
		{
			if (socket)
			{
				socket->send(data);
			}
		}
	}

	auto NetServer::on_message([[maybe_unused]] std::shared_ptr<ix::ConnectionState> state, ix::WebSocket& socket, const ix::WebSocketMessage& message) noexcept -> void
	{
		if (message.type == ix::WebSocketMessageType::Close || message.type == ix::WebSocketMessageType::Error)
		{
			on_disconnect(socket);
			return;
		}

		if (message.type == ix::WebSocketMessageType::Open)
		{
			send_lobby(socket);
			return;
		}

		if (message.type != ix::WebSocketMessageType::Message)
		{
			return;
		}

		nlohmann::json content;
		try
		{
			content = nlohmann::json::parse(message.str);
		}
		catch (const nlohmann::json::parse_error& error)
		{
			SPDLOG_WARN(NS_LOG_PREFIX "无法解析的消息内容: {}({})", message.str, error.what());

			auto response = nlohmann::json::object();

			response["type"] = "error";
			response["message"] = "invalid JSON content";

			socket.send(response.dump());
			return;
		}

		if (const auto type = content.value("type", std::string{});
			type == "join")
		{
			handle_join(socket, content);
		}
		else if (type == "choose_deck")
		{
			handle_choose_deck(socket, content);
		}
		else if (type == "surrender")
		{
			handle_surrender(socket);
		}
		else if (type == "response")
		{
			handle_response(socket, content);
		}
		else if (type == "sync")
		{
			handle_sync(socket);
		}
		else
		{
			SPDLOG_WARN(NS_LOG_PREFIX "无法处理的消息类型: {}", message.str);

			auto response = nlohmann::json::object();

			response["type"] = "error";
			response["message"] = "unknown message type";

			socket.send(response.dump());
		}
	}

	auto NetServer::on_disconnect(const ix::WebSocket& socket) noexcept -> void
	{
		std::scoped_lock lock{mutex_};

		for (auto& peer: peers_)
		{
			if (peer.socket && peer.socket.get() == &socket)
			{
				peer.socket.reset();
				peer.joined = false;

				if (duel_started_ && !duel_finished_)
				{
					duel_abort_ = true;
					condition_variable_.notify_all();
				}
			}
		}
	}

	auto NetServer::seat_of_socket(const ix::WebSocket& socket) noexcept -> domain::Player
	{
		std::scoped_lock lock{mutex_};

		for (const auto player: {domain::Player::FIRST, domain::Player::SECOND})
		{
			if (const auto& peer = peers_[std::to_underlying(player)];
				peer.socket && peer.socket.get() == &socket)
			{
				return player;
			}
		}

		return invalid_seat;
	}

	auto NetServer::handle_join(ix::WebSocket& socket, const nlohmann::json& message) noexcept -> void
	{
		const auto client = socket_for(socket);
		if (client == nullptr)
		{
			return;
		}

		domain::Player seat = invalid_seat;
		{
			std::scoped_lock lock{mutex_};

			if (duel_started_)
			{
				auto response = nlohmann::json::object();

				response["type"] = "error";
				response["message"] = "duel already started; send \"sync\" to resync";

				client->send(response.dump());
				return;
			}

			if (const auto seat_str = message.value("seat", "?");
				seat_str == utility::to_string(domain::Player::FIRST) && !peers_[std::to_underlying(domain::Player::FIRST)].joined)
			{
				seat = domain::Player::FIRST;
			}
			else if (seat_str == utility::to_string(domain::Player::SECOND) && !peers_[std::to_underlying(domain::Player::SECOND)].joined)
			{
				seat = domain::Player::SECOND;
			}
			else if (seat_str == "?")
			{
				if (!peers_[std::to_underlying(domain::Player::FIRST)].joined)
				{
					seat = domain::Player::FIRST;
				}
				else if (!peers_[std::to_underlying(domain::Player::SECOND)].joined)
				{
					seat = domain::Player::SECOND;
				}
			}

			if (seat == invalid_seat)
			{
				auto response = nlohmann::json::object();

				response["type"] = "error";
				response["message"] = "seat unavailable (room full)";

				client->send(response.dump());
				return;
			}

			peers_[std::to_underlying(seat)].socket = client;
			peers_[std::to_underlying(seat)].joined = true;
		}

		{
			auto response = nlohmann::json::object();

			response["type"] = "join";
			response["seat"] = utility::to_string(seat);
			response["session"] = message.value("session", "");

			client->send(response.dump());
		}

		send_lobby(socket);
		broadcast_room_sync();
	}

	auto NetServer::handle_choose_deck(ix::WebSocket& socket, const nlohmann::json& message) noexcept -> void
	{
		const auto seat = seat_of_socket(socket);
		if (seat == invalid_seat)
		{
			auto response = nlohmann::json::object();

			response["type"] = "error";
			response["message"] = "not joined";

			socket.send(response.dump());
			return;
		}
		const auto seat_name = utility::to_string(seat);

		bool ready;
		{
			std::scoped_lock lock{mutex_};
			if (duel_started_)
			{
				auto response = nlohmann::json::object();

				response["type"] = "error";
				response["message"] = "duel already started; deck selection is closed";

				socket.send(response.dump());
				return;
			}

			if (const auto claimed = message.value("seat", "");
				!claimed.empty() && claimed != seat_name)
			{
				auto response = nlohmann::json::object();

				response["type"] = "error";
				response["message"] = "seat does not match this connection";

				socket.send(response.dump());
				return;
			}

			const auto deck_name = message.value("deck", "");
			const auto deck_it = std::ranges::find(catalog_decks_, deck_name, &Deck::filename);
			if (deck_it == catalog_decks_.end())
			{
				auto response = nlohmann::json::object();

				response["type"] = "error";
				response["message"] = std::format("unknown deck \"{}\"", deck_name);

				socket.send(response.dump());
				return;
			}

			auto& peer = peers_[std::to_underlying(seat)];
			peer.main_deck = deck_it->main_deck;
			peer.extra_deck = deck_it->extra_deck;
			peer.side_deck = deck_it->side_deck;
			peer.chose_deck = true;

			ready = check_lobby_both_ready_locked();
		}

		broadcast_lobby();

		if (ready)
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
	}

	auto NetServer::handle_surrender(ix::WebSocket& socket) noexcept -> void
	{
		const auto seat = seat_of_socket(socket);
		if (seat == invalid_seat)
		{
			auto response = nlohmann::json::object();

			response["type"] = "error";
			response["message"] = "not joined";

			socket.send(response.dump());
			return;
		}

		{
			std::scoped_lock lock{mutex_};
			if (!pending_concede_.has_value())
			{
				pending_concede_ = seat;
			}
			condition_variable_.notify_all();
		}

		auto log = nlohmann::json::object();

		log["event"] = "surrender";
		log["player"] = utility::to_string(seat);

		send_log(log);
	}

	auto NetServer::handle_response(ix::WebSocket& socket, const nlohmann::json& message) noexcept -> void
	{
		const auto seat = seat_of_socket(socket);
		if (seat == invalid_seat)
		{
			auto response = nlohmann::json::object();

			response["type"] = "error";
			response["message"] = "not joined";

			socket.send(response.dump());
			return;
		}

		{
			std::scoped_lock lock{mutex_};

			if (!waiting_seat_.has_value() || *waiting_seat_ != seat)
			{
				auto response = nlohmann::json::object();
				response["type"] = "error";
				response["message"] = "no request is pending for this seat";

				socket.send(response.dump());
				return;
			}

			if (const auto request_id = message.value("request_id", static_cast<std::uint64_t>(0));
				request_id != waiting_request_id_)
			{
				auto response = nlohmann::json::object();

				response["type"] = "error";
				response["message"] = "stale or unknown request_id";

				socket.send(response.dump());
				return;
			}

			if (pending_response_.has_value())
			{
				// 重复应答
				return;
			}

			if (!message.contains("response"))
			{
				auto response = nlohmann::json::object();

				response["type"] = "error";
				response["message"] = "response message requires \"response\"";

				socket.send(response.dump());
				return;
			}

			pending_response_ = message;
			condition_variable_.notify_all();
		}
	}

	auto NetServer::handle_sync(ix::WebSocket& socket) noexcept -> void
	{
		const auto seat = seat_of_socket(socket);
		if (seat == invalid_seat)
		{
			auto response = nlohmann::json::object();

			response["type"] = "error";
			response["message"] = "not joined";

			socket.send(response.dump());
			return;
		}

		nlohmann::json state;
		nlohmann::json request;
		{
			std::scoped_lock lock{mutex_};

			const auto& peer = peers_[std::to_underlying(seat)];

			state = peer.last_state;
			if (waiting_seat_.has_value() && *waiting_seat_ == seat && !peer.last_request.is_null())
			{
				request = peer.last_request;
			}
		}

		if (state.is_null())
		{
			nlohmann::json sync;
			{
				std::scoped_lock lock{mutex_};
				sync = get_room_sync_content_locked();
			}
			socket.send(sync.dump());
		}
		else
		{
			socket.send(state.dump());
		}

		if (!request.is_null())
		{
			socket.send(request.dump());
		}
	}

	auto NetServer::wait_for_response() -> nlohmann::json
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
		return response;
	}

	auto NetServer::start_duel() noexcept -> void
	{
		try
		{
			// TODO
			// 创建对局
		}
		catch (const DuelAborted&)
		{
			{
				std::scoped_lock lock{mutex_};
				duel_finished_ = true;
			}

			{
				auto content = nlohmann::json::object();

				content["type"] = "error";
				content["message"] = "duel aborted";

				broadcast_message(content);
			}
			{
				auto content = nlohmann::json::object();

				content["event"] = "duel_aborted";

				send_log(content);
			}
		}
		catch (const std::exception& exception)
		{
			{
				std::scoped_lock lock{mutex_};
				duel_finished_ = true;
			}

			{
				auto content = nlohmann::json::object();

				content["type"] = "error";
				content["message"] = std::format("Error: {}", exception.what());

				broadcast_message(content);
			}
		}
		catch (...)
		{
			{
				std::scoped_lock lock{mutex_};
				duel_finished_ = true;
			}

			{
				auto content = nlohmann::json::object();

				content["type"] = "error";
				content["message"] = "unexpected error";

				broadcast_message(content);
			}
		}
	}

	auto NetServer::next_response(const driver::Request& request) -> driver::Response
	{
		const auto player = driver::player_of(request);

		std::uint64_t request_id;
		{
			std::scoped_lock lock{mutex_};

			next_request_id_ += 1;
			request_id = next_request_id_;
			waiting_request_id_ = next_request_id_;
			waiting_seat_ = player;
			pending_response_.reset();

			decisions_[std::to_underlying(player)] += 1;
		}

		// TODO: 广播决斗状态
		// broadcast_duel(duel);

		auto request_data = encode_request(request);
		request_data["request_id"] = request_id;

		{
			std::scoped_lock lock{mutex_};
			peers_[std::to_underlying(player)].last_request = request_data;
		}

		{
			auto content = nlohmann::json::object();

			content["event"] = "request";
			content["request_id"] = request_id;
			content["player"] = utility::to_string(player);
			content["kind"] = request.index();

			send_log(content);
		}

		if (!send_message(player, request_data))
		{
			throw DuelAborted{};
		}

		auto response_data = wait_for_response();
		for (;;)
		{
			auto response = decode_response(request, response_data);
			if (response.has_value())
			{
				auto content = nlohmann::json::object();

				content["event"] = "response";
				content["request_id"] = request_id;
				content["player"] = utility::to_string(player);
				content["kind"] = response->index();

				send_log(content);

				return *std::move(response);
			}

			send_error(player, response.error());

			if (!send_message(player, request_data))
			{
				throw DuelAborted{};
			}

			response_data = wait_for_response();
		}
	}

	NetServer::NetServer(NetOptions options) noexcept
		: options_{std::move(options)},
		  http_server_{nullptr},
		  // peers_{},
		  duel_started_{false},
		  duel_finished_{false},
		  duel_abort_{false},
		  stopping_{false},
		  sequence_{0},
		  next_request_id_{0},
		  waiting_request_id_{0},
		  decisions_{0, 0}
	{
		//
	}

	NetServer::~NetServer() noexcept
	{
		{
			std::scoped_lock lock{mutex_};
			stopping_ = true;
			duel_abort_ = true;
			condition_variable_.notify_all();
		}
		{
			std::scoped_lock lock{duel_thread_mutex_};
			if (duel_thread_.joinable())
			{
				duel_thread_.join();
			}
		}
	}

	auto NetServer::initialize() noexcept -> bool
	{
#ifdef _WIN32
		if (!ix::initNetSystem())
		{
			return false;
		}
#endif

		if (!load_db())
		{
			return false;
		}
		if (!load_catalog())
		{
			return false;
		}
		if (!load_web())
		{
			return false;
		}
		if (!setup_http_server())
		{
			return false;
		}

		return true;
	}

	auto NetServer::run() noexcept -> int
	{
		if (!http_server_->listenAndStart())
		{
			SPDLOG_ERROR(NS_LOG_PREFIX "HTTP-SERVER启动失败!");
			return 1;
		}

		std::println(
			"Server: \n"
			"\t http://{}:{}\n"
			"\t ws://{}:{}\n"
			"\t static={}\n"
			"\t pics=/pics/** <- {}\n"
			"\t assets=/assets/** <- {}\n"
			"\t cards={}\n"
			"\t decks={}\n",
			// HTTP
			options_.host,
			options_.port,
			// WS
			options_.host,
			options_.port,
			// STATIC
			options_.web_path,
			// PICS
			options_.pics_path,
			// ASSETS
			options_.assets_path,
			// CARDS
			options_.cards_path,
			// DECKS
			options_.decks_path
		);

		http_server_->wait();
		return 0;
	}
}

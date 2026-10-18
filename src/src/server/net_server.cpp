#include <cg/server/net_server.hpp>

#include <algorithm>
#include <fstream>
#include <print>

#include <cg/utility/functional.hpp>

#include <cg/server/codec.hpp>

#include <spdlog/spdlog.h>
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
	}

	// ==============================================================================================
	// NetPeer
	// ==============================================================================================

	NetServer::NetPeer::~NetPeer() noexcept = default;

	auto NetServer::NetPeer::attach(std::shared_ptr<ix::WebSocket> socket) noexcept -> void
	{
		std::scoped_lock lock{mutex_};
		socket_ = std::move(socket);
	}

	auto NetServer::NetPeer::detach() noexcept -> void
	{
		std::scoped_lock lock{mutex_};
		socket_.reset();
	}

	auto NetServer::NetPeer::attached() const noexcept -> bool
	{
		std::scoped_lock lock{mutex_};
		return socket_ != nullptr;
	}

	auto NetServer::NetPeer::matches(const ix::WebSocket& socket) const noexcept -> bool
	{
		std::scoped_lock lock{mutex_};
		return socket_.get() == &socket;
	}

	auto NetServer::NetPeer::seat() const noexcept -> std::optional<domain::Player>
	{
		std::scoped_lock lock{mutex_};
		return seat_;
	}

	auto NetServer::NetPeer::set_seat(const std::optional<domain::Player> seat) noexcept -> void
	{
		std::scoped_lock lock{mutex_};
		seat_ = seat;
	}

	void NetServer::NetPeer::deliver(const message::Message& message) noexcept
	{
		std::shared_ptr<ix::WebSocket> socket;
		{
			std::scoped_lock lock{mutex_};
			socket = socket_;
		}

		if (socket == nullptr)
		{
			return;
		}

		try
		{
			socket->send(ValueCodec<message::Message>::encode(message).dump());
		}
		catch (const std::exception& exception)
		{
			SPDLOG_WARN(NS_LOG_PREFIX "发送消息失败: {}", exception.what());
		}
	}

	// ==============================================================================================
	// 初始化
	// ==============================================================================================

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
			[this](const ix::HttpRequestPtr& request, [[maybe_unused]] const std::shared_ptr<ix::ConnectionState>& state) noexcept -> ix::HttpResponsePtr
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
				[[maybe_unused]] const auto& connection_state = state;

				switch (message->type)
				{
					case ix::WebSocketMessageType::Open:
					{
						on_open(socket);
						break;
					}
					case ix::WebSocketMessageType::Close:
					case ix::WebSocketMessageType::Error:
					{
						on_close(socket);
						break;
					}
					case ix::WebSocketMessageType::Message:
					{
						nlohmann::json content;
						try
						{
							content = nlohmann::json::parse(message->str);
						}
						catch (const nlohmann::json::parse_error& error)
						{
							SPDLOG_WARN(NS_LOG_PREFIX "无法解析的消息内容: {}({})", message->str, error.what());

							if (auto* peer = find_peer(socket);
								peer != nullptr)
							{
								peer->deliver(message::Message{message::Error{.message = "invalid JSON content", .source = std::nullopt, .request_id = std::nullopt}});
							}
							break;
						}

						on_message(socket, content);
						break;
					}
					case ix::WebSocketMessageType::Ping:
					case ix::WebSocketMessageType::Pong:
					case ix::WebSocketMessageType::Fragment:
					default: // NOLINT(clang-diagnostic-covered-switch-default)
					{
						break;
					}
				}
			}
		);

		return true;
	}

	auto NetServer::serve_static(const std::string_view uri, const char*& content_type, int& status) const noexcept -> std::string
	{
		auto relative = std::string{uri};
		if (const auto query = relative.find('?');
			query != std::string::npos)
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
		if (path.starts_with("pics/"))
		{
			base = pics_root_;
			path = path.substr(5);
		}
		else if (path.starts_with("assets/"))
		{
			base = assets_root_;
			path = path.substr(7);
		}

		const auto resolved = base / path;
		if (std::error_code error_code{};
			!exists(resolved, error_code))
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

	// ==============================================================================================
	// 连接
	// ==============================================================================================

	auto NetServer::socket_for(const ix::WebSocket& socket) const noexcept -> std::shared_ptr<ix::WebSocket>
	{
		if (http_server_ == nullptr)
		{
			return nullptr;
		}

		for (const auto& client: http_server_->getClients())
		{
			if (client.get() == &socket)
			{
				return client;
			}
		}

		return nullptr;
	}

	auto NetServer::find_peer(const ix::WebSocket& socket) const noexcept -> NetPeer*
	{
		std::scoped_lock lock{peers_mutex_};

		for (auto& peer: peers_)
		{
			if (peer != nullptr && peer->matches(socket))
			{
				return peer.get();
			}
		}

		return nullptr;
	}

	auto NetServer::allocate_peer() noexcept -> NetPeer*
	{
		std::scoped_lock lock{peers_mutex_};

		for (auto& peer: peers_)
		{
			if (peer == nullptr)
			{
				peer = std::make_unique<NetPeer>();
				return peer.get();
			}
			if (!peer->attached())
			{
				return peer.get();
			}
		}

		return nullptr;
	}

	auto NetServer::on_open(const ix::WebSocket& socket) noexcept -> void
	{
		auto* peer = allocate_peer();
		if (peer == nullptr)
		{
			SPDLOG_WARN(NS_LOG_PREFIX "连接数已满,拒绝新连接");
			return;
		}

		peer->attach(socket_for(socket));
		peer->deliver(message::Message{room_.lobby_state()});
	}

	auto NetServer::on_close(const ix::WebSocket& socket) noexcept -> void
	{
		auto* peer = find_peer(socket);
		if (peer == nullptr)
		{
			return;
		}

		if (const auto seat = peer->seat(); seat.has_value())
		{
			room_.leave(*seat);
		}

		peer->detach();
		peer->set_seat(std::nullopt);
	}

	auto NetServer::on_message(const ix::WebSocket& socket, const nlohmann::json& content) noexcept -> void
	{
		auto* peer = find_peer(socket);
		if (peer == nullptr)
		{
			return;
		}

		const auto message = ValueCodec<message::Message>::decode(content);
		if (!message.has_value())
		{
			message::Error error;
			error.message = "invalid message";
			if (const auto type_it = content.find("type");
				type_it != content.end() && type_it.value().is_string())
			{
				error.source = type_it.value().get<nlohmann::json::string_t>();
			}

			peer->deliver(message::Message{std::move(error)});
			return;
		}

		handle_client_message(*peer, *message);
	}

	auto NetServer::handle_join(NetPeer& peer, const message::Join& message) noexcept -> void
	{
		if (const auto seat = peer.seat(); seat.has_value())
		{
			// 已加入 -- 重发同步
			room_.dispatch(*seat, message::Sync{});
			return;
		}

		const auto seat = room_.join(peer, message.seat, message.session);
		if (!seat.has_value())
		{
			peer.deliver(message::Message{message::Error{.message = "seat unavailable (room full or duel already started)", .source = std::nullopt, .request_id = std::nullopt}});
			return;
		}

		peer.set_seat(seat);
	}

	auto NetServer::handle_client_message(NetPeer& peer, const message::Message& message) noexcept -> void
	{
		const auto deliver_not_joined = [&peer]() noexcept -> void
		{
			peer.deliver(message::Message{message::Error{.message = "not joined", .source = std::nullopt, .request_id = std::nullopt}});
		};

		std::visit(
			utility::Overloaded
			{
					[&](const message::Join& join) noexcept -> void
					{
						handle_join(peer, join);
					},
					[&](const message::ChooseDeck& choose_deck) noexcept -> void
					{
						if (const auto seat = peer.seat(); seat.has_value())
						{
							room_.dispatch(*seat, choose_deck);
						}
						else
						{
							deliver_not_joined();
						}
					},
					[&](const message::Surrender& surrender) noexcept -> void
					{
						if (const auto seat = peer.seat(); seat.has_value())
						{
							room_.dispatch(*seat, surrender);
						}
						else
						{
							deliver_not_joined();
						}
					},
					[&](const message::Sync& sync) noexcept -> void
					{
						if (const auto seat = peer.seat(); seat.has_value())
						{
							room_.dispatch(*seat, sync);
						}
						else
						{
							deliver_not_joined();
						}
					},
					[&](const message::Response& response) noexcept -> void
					{
						if (const auto seat = peer.seat(); seat.has_value())
						{
							room_.dispatch(*seat, response);
						}
						else
						{
							deliver_not_joined();
						}
					},
					[&]<typename T>([[maybe_unused]] const T& unhandled) noexcept -> void
					{
						peer.deliver(message::Message{message::Error{.message = "unexpected message", .source = std::nullopt, .request_id = std::nullopt}});
					}
			},
			message
		);
	}

	// ==============================================================================================
	// NetServer
	// ==============================================================================================

	NetServer::NetServer(NetOptions options) noexcept
		: options_{std::move(options)},
		  room_{options_.room_options},
		  http_server_{nullptr}
	{
		//
	}

	NetServer::~NetServer() noexcept
	{
		// 先停下决斗线程,再停止网络层
		room_.stop();

		if (http_server_ != nullptr)
		{
			http_server_->stop();
		}

#ifdef _WIN32
		if (net_system_initialized_)
		{
			ix::uninitNetSystem();
		}
#endif
	}

	auto NetServer::initialize() noexcept -> bool
	{
#ifdef _WIN32
		if (!ix::initNetSystem())
		{
			return false;
		}
		net_system_initialized_ = true;
#endif

		if (!room_.initialize())
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
		if (http_server_ == nullptr || !http_server_->listenAndStart())
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
			options_.room_options.cards_path,
			// DECKS
			options_.room_options.decks_path
		);

		http_server_->wait();
		return 0;
	}
}

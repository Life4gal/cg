#pragma once

#include <array>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

#include <cg/domain/player.hpp>

#include <cg/server/message.hpp>
#include <cg/server/room.hpp>
#include <cg/server/server.hpp>

#include <nlohmann/json_fwd.hpp>

namespace ix
{
	class HttpServer;
	class WebSocket;
	class ConnectionState;
	class WebSocketMessage;
}

namespace cg::server
{
	class NetOptions
	{
	public:
		RoomOptions room_options;

		// web资源路径
		std::string web_path;
		// 卡图目录
		std::string pics_path;
		// 资源路径
		std::string assets_path;

		// HOST
		std::string host;
		// 端口
		int port;
	};

	class NetServer final : public Server
	{
	public:
		// 一个WebSocket连接 -- 同时是Room的出站接收者
		class NetPeer final : public Room::Client
		{
		public:
			explicit NetPeer() noexcept = default;

			NetPeer(const NetPeer&) noexcept = delete;
			auto operator=(const NetPeer&) noexcept -> NetPeer& = delete;
			NetPeer(NetPeer&&) noexcept = delete;
			auto operator=(NetPeer&&) noexcept -> NetPeer& = delete;

			~NetPeer() noexcept override;

			auto attach(std::shared_ptr<ix::WebSocket> socket) noexcept -> void;
			auto detach() noexcept -> void;

			[[nodiscard]] auto attached() const noexcept -> bool;
			[[nodiscard]] auto matches(const ix::WebSocket& socket) const noexcept -> bool;
			[[nodiscard]] auto seat() const noexcept -> std::optional<domain::Player>;
			auto set_seat(std::optional<domain::Player> seat) noexcept -> void;

			void deliver(const message::Message& message) noexcept override;

		private:
			mutable std::mutex mutex_;
			std::shared_ptr<ix::WebSocket> socket_;
			std::optional<domain::Player> seat_;
		};

		constexpr static std::string_view index_filename = "index.html";

	private:
		NetOptions options_;
		Room room_;
		std::unique_ptr<ix::HttpServer> http_server_;

		mutable std::mutex peers_mutex_;
		std::array<std::unique_ptr<NetPeer>, Room::max_peer> peers_;

		std::filesystem::path web_root_;
		std::filesystem::path pics_root_;
		std::filesystem::path assets_root_;

		bool net_system_initialized_ = false;

		// ====================================
		// 初始化
		// ====================================

		[[nodiscard]] auto load_web() noexcept -> bool;
		[[nodiscard]] auto setup_http_server() noexcept -> bool;
		[[nodiscard]] auto serve_static(std::string_view uri, const char*& content_type, int& status) const noexcept -> std::string;

		// ====================================
		// 连接
		// ====================================

		[[nodiscard]] auto socket_for(const ix::WebSocket& socket) const noexcept -> std::shared_ptr<ix::WebSocket>;
		[[nodiscard]] auto find_peer(const ix::WebSocket& socket) const noexcept -> NetPeer*;
		[[nodiscard]] auto allocate_peer() noexcept -> NetPeer*;

		auto on_open(const ix::WebSocket& socket) noexcept -> void;
		auto on_close(const ix::WebSocket& socket) noexcept -> void;
		auto on_message(const ix::WebSocket& socket, const nlohmann::json& content) noexcept -> void;

		auto handle_join(NetPeer& peer, const message::Join& message) noexcept -> void;
		auto handle_client_message(NetPeer& peer, const message::Message& message) noexcept -> void;

	public:
		explicit NetServer(NetOptions options) noexcept;

		NetServer(const NetServer&) noexcept = delete;
		auto operator=(const NetServer&) noexcept -> NetServer& = delete;

		NetServer(NetServer&&) noexcept = delete;
		auto operator=(NetServer&&) noexcept -> NetServer& = delete;

		~NetServer() noexcept override;

		auto initialize() noexcept -> bool override;

		auto run() noexcept -> int override;
	};
}

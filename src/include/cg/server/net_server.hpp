#pragma once

#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>

#include <cg/domain/player.hpp>

#include <cg/driver/request.hpp>
#include <cg/driver/response.hpp>

#include <cg/server/deck.hpp>
#include <cg/server/server.hpp>

#include <nlohmann/json.hpp>

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
		// 卡牌脚本目录
		std::string cards_path;
		// 可用卡组目录
		std::string decks_path;
		// web资源路径
		std::string web_path = "web/dist";
		// 卡图目录
		std::string pics_path = "pics";
		// 资源路径
		std::string assets_path = "web/public/assets";

		// HOST
		std::string host = "127.0.0.1";
		// 端口
		int port = 5678;
	};

	class NetServer final : public Server
	{
	public:
		class NetDriver;

		class Peer
		{
		public:
			std::shared_ptr<ix::WebSocket> socket = nullptr;
			bool joined = false;
			bool chose_deck = false;

			nlohmann::json last_state = {};
			nlohmann::json last_request = {};

			std::vector<domain::CardCode> main_deck = {};
			std::vector<domain::CardCode> extra_deck = {};
			std::vector<domain::CardCode> side_deck = {};
		};

		// FIRST + SECOND
		constexpr static std::size_t max_peer = 2;
		constexpr static auto invalid_seat = static_cast<domain::Player>(std::numeric_limits<std::underlying_type_t<domain::Player>>::max());

		constexpr static std::string_view index_filename = "index.html";

	private:
		NetOptions options_;
		std::unique_ptr<ix::HttpServer> http_server_;

		std::array<Peer, max_peer> peers_;
		std::vector<Deck> catalog_decks_;
		std::filesystem::path web_root_;
		std::filesystem::path pics_root_;
		std::filesystem::path assets_root_;

		std::mutex mutex_;
		std::condition_variable condition_variable_;
		std::thread duel_thread_;
		std::mutex duel_thread_mutex_;

		// 决斗已开始
		bool duel_started_;
		// 决斗已结束
		bool duel_finished_;
		// 决斗被中断
		bool duel_abort_;

		// 服务器正在停止
		bool stopping_;

		std::uint64_t sequence_;
		std::uint64_t next_request_id_;
		std::uint64_t waiting_request_id_;
		std::optional<domain::Player> waiting_seat_;
		std::optional<nlohmann::json> pending_response_;
		std::optional<domain::Player> pending_concede_;
		std::array<std::size_t, max_peer> decisions_;

		// ====================================
		// 初始化
		// ====================================

		[[nodiscard]] auto load_db() noexcept -> bool;
		[[nodiscard]] auto load_catalog() noexcept -> bool;
		[[nodiscard]] auto load_web() noexcept -> bool;
		[[nodiscard]] auto setup_http_server() noexcept -> bool;

		[[nodiscard]] auto serve_static(std::string_view uri, const char*& content_type, int& status) const noexcept -> std::string;

		// ====================================
		// 大厅/房间
		// ====================================

		[[nodiscard]] auto get_lobby_payload_locked() const noexcept -> nlohmann::json;
		[[nodiscard]] auto get_lobby_content_locked() const noexcept -> nlohmann::json;
		[[nodiscard]] auto check_lobby_both_ready_locked() noexcept -> bool;

		[[nodiscard]] auto get_room_sync_content_locked() const noexcept -> nlohmann::json;

		auto send_lobby(ix::WebSocket& socket) noexcept -> void;
		auto broadcast_lobby() noexcept -> void;
		auto broadcast_room_sync() noexcept -> void;

		auto send_log(const nlohmann::json& entry) noexcept -> void;
		auto send_error(domain::Player player, const nlohmann::json& error) noexcept -> void;

		// ====================================
		// 发送信息
		// ====================================

		[[nodiscard]] auto socket_for(const ix::WebSocket& socket) noexcept -> std::shared_ptr<ix::WebSocket>;

		auto send_message(domain::Player player, const nlohmann::json& message) noexcept -> bool;
		auto broadcast_message(const nlohmann::json& message) noexcept -> void;

		// ====================================
		// 网络回调
		// ====================================

		auto on_message(std::shared_ptr<ix::ConnectionState> state, ix::WebSocket& socket, const ix::WebSocketMessage& message) noexcept -> void;
		auto on_disconnect(const ix::WebSocket& socket) noexcept -> void;

		[[nodiscard]] auto seat_of_socket(const ix::WebSocket& socket) noexcept -> domain::Player;

		auto handle_join(ix::WebSocket& socket, const nlohmann::json& message) noexcept -> void;
		auto handle_choose_deck(ix::WebSocket& socket, const nlohmann::json& message) noexcept -> void;
		auto handle_surrender(ix::WebSocket& socket) noexcept -> void;
		auto handle_response(ix::WebSocket& socket, const nlohmann::json& message) noexcept -> void;
		auto handle_sync(ix::WebSocket& socket) noexcept -> void;

		// ====================================
		// 决斗
		// ====================================

		[[nodiscard]] auto wait_for_response() -> nlohmann::json;

		auto start_duel() noexcept -> void;
		// 供NetDriver调用
		[[nodiscard]] auto next_response(const driver::Request& request) -> driver::Response;

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

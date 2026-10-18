#pragma once

#include <array>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <limits>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <cg/domain/player.hpp>

#include <cg/driver/request.hpp>
#include <cg/driver/response.hpp>

#include <cg/server/deck.hpp>
#include <cg/server/message.hpp>

#include <nlohmann/json.hpp>

namespace cg::server
{
	// 决斗被中断(连接断开/服务器停止)
	class DuelAborted final : public std::exception
	{
	public:
		using exception::exception;
	};

	// 决斗被投降终止
	class DuelConceded final : public std::exception
	{
	public:
		domain::Player player;

		explicit DuelConceded(const domain::Player player) noexcept
			: player{player} {}
	};

	class RoomOptions
	{
	public:
		// 卡牌脚本目录
		std::string cards_path;
		// 可用卡组目录
		std::string decks_path;
	};

	// 传输无关的房间/对局会话
	// 只处理协议语义与决斗线程,不涉及任何具体传输(网络/GUI)
	class Room final
	{
	public:
		// 出站消息接收者
		class Client
		{
		public:
			constexpr Client() noexcept = default;
			constexpr Client(const Client&) noexcept = delete;
			constexpr auto operator=(const Client&) noexcept -> Client& = delete;
			constexpr Client(Client&&) noexcept = default;
			constexpr auto operator=(Client&&) noexcept -> Client& = default;

			constexpr virtual ~Client() noexcept = default;

			virtual void deliver(const message::Message& message) noexcept = 0;
		};

		// FIRST + SECOND
		constexpr static std::size_t max_peer = 2;
		constexpr static auto invalid_seat = static_cast<domain::Player>(std::numeric_limits<std::underlying_type_t<domain::Player>>::max());

	private:
		class Seat
		{
		public:
			Client* client = nullptr;
			bool joined = false;
			bool ready = false;
			std::string session;
			std::string deck_name;
			std::vector<domain::CardCode> main_deck;
			std::vector<domain::CardCode> extra_deck;
			std::vector<domain::CardCode> side_deck;
			std::optional<message::Request> last_request;
		};

		RoomOptions options_;
		mutable std::mutex mutex_;
		std::condition_variable condition_variable_;
		std::array<Seat, max_peer> seats_;
		std::vector<Deck> catalog_decks_;

		std::thread duel_thread_;
		std::mutex duel_thread_mutex_;

		// 决斗已开始
		bool duel_started_ = false;
		// 决斗已结束
		bool duel_finished_ = false;
		// 决斗被中断
		bool duel_abort_ = false;
		// 服务器正在停止
		bool stopping_ = false;

		std::uint64_t sequence_ = 0;
		std::uint64_t next_request_id_ = 0;
		std::uint64_t waiting_request_id_ = 0;
		std::optional<domain::Player> waiting_seat_;
		std::optional<driver::response::Response> pending_response_;
		std::optional<domain::Player> pending_concede_;

		// ====================================
		// 初始化
		// ====================================

		[[nodiscard]] auto load_db() noexcept -> bool;
		[[nodiscard]] auto load_catalog() noexcept -> bool;

		// ====================================
		// 状态构造(调用时需持有mutex_)
		// ====================================

		[[nodiscard]] auto lobby_state_locked() const noexcept -> message::LobbyState;
		[[nodiscard]] auto room_sync_locked() const noexcept -> message::RoomSync;
		[[nodiscard]] auto check_ready_locked() noexcept -> bool;

		// ====================================
		// 发送(内部自行加锁)
		// ====================================

		auto deliver(domain::Player player, const message::Message& message) const noexcept -> bool;
		auto broadcast(const message::Message& message) const noexcept -> void;

		auto send_lobby(domain::Player player) const noexcept -> void;
		auto broadcast_lobby() const noexcept -> void;
		auto send_room_sync(domain::Player player) const noexcept -> void;
		auto broadcast_room_sync() const noexcept -> void;

		auto send_log(const nlohmann::json& entry) noexcept -> void;
		auto send_error(domain::Player player, const message::Error& error) const noexcept -> void;

		// ====================================
		// 处理
		// ====================================

		auto handle_choose_deck(domain::Player player, std::string deck) noexcept -> void;
		auto handle_surrender(domain::Player player) noexcept -> void;
		auto handle_sync(domain::Player player) const noexcept -> void;
		auto handle_response(domain::Player player, std::uint64_t request_id, driver::response::Response response) noexcept -> bool;

		// ====================================
		// 决斗
		// ====================================

		auto start_duel_async() noexcept -> void;
		auto start_duel() noexcept -> void;
		[[nodiscard]] auto wait_for_response() -> driver::response::Response;

	public:
		explicit Room(RoomOptions options) noexcept;

		Room(const Room&) noexcept = delete;
		auto operator=(const Room&) noexcept -> Room& = delete;
		Room(Room&&) noexcept = delete;
		auto operator=(Room&&) noexcept -> Room& = delete;

		~Room() noexcept;

		// 加载卡牌/卡组
		[[nodiscard]] auto initialize() noexcept -> bool;

		// 可用卡组(拷贝)
		[[nodiscard]] auto decks() const noexcept -> std::vector<Deck>;

		// ====================================
		// 座位 / 大厅
		// ====================================

		// 加入房间,返回分配到的座位;失败返回null-opt
		[[nodiscard]] auto join(Client& client, std::optional<domain::Player> preferred, std::string session) noexcept -> std::optional<domain::Player>;
		// 离开房间
		auto leave(domain::Player player) noexcept -> void;

		// client所属座位
		[[nodiscard]] auto seat_of(const Client& client) const noexcept -> domain::Player;
		[[nodiscard]] auto is_joined(domain::Player player) const noexcept -> bool;

		// 当前大厅状态
		[[nodiscard]] auto lobby_state() const noexcept -> message::LobbyState;

		// ====================================
		// 客户端消息
		// ====================================

		auto dispatch(domain::Player player, const message::ChooseDeck& message) noexcept -> void;
		auto dispatch(domain::Player player, const message::Surrender& message) noexcept -> void;
		auto dispatch(domain::Player player, const message::Sync& message) const noexcept -> void;
		auto dispatch(domain::Player player, const message::Response& message) noexcept -> void;

		// ====================================
		// 决斗握手 -- 供Driver适配器调用
		// ====================================

		// 阻塞地向player请求一个决策
		// 中断/投降以异常形式抛出
		[[nodiscard]] auto decide(const driver::request::Request& request) -> driver::response::Response;

		// 等待决斗结束或被停止
		auto wait() noexcept -> void;

		// 停止决斗线程并唤醒等待者(可重复调用)
		auto stop() noexcept -> void;
	};
}

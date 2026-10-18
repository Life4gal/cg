#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

#include <cg/domain/player.hpp>

#include <cg/driver/request.hpp>
#include <cg/driver/response.hpp>

#include <cg/server/deck.hpp>

namespace cg::server::message
{
	// =================================================================
	// 客户端 -> 服务端
	// =================================================================

	// 加入房间
	// JSON:
	// {
	//     "seat": "FIRST" | null,
	//     "session": "..."
	// }
	class Join
	{
	public:
		// null-opt => 自动分配空闲座位
		std::optional<domain::Player> seat;
		std::string session;
	};

	// 选择卡组
	// JSON:
	// {
	//     "deck": "blue-eyes"
	// }
	class ChooseDeck
	{
	public:
		std::string deck;
	};

	// 投降
	class Surrender {};

	// 房间同步请求 -- 服务器以RoomSync回应
	class Sync {};

	// 决斗回应
	// JSON:
	// {
	//     "request_id": 42,
	//     "payload": { ... driver::response::Response ... }
	// }
	class Response
	{
	public:
		std::uint64_t request_id;
		driver::response::Response payload;
	};

	// =================================================================
	// 服务端 -> 客户端
	// =================================================================

	// 座位分配成功
	class Joined
	{
	public:
		domain::Player seat;
		// 会话标识,用于后续重连
		std::string session;
	};

	// 大厅状态
	// JSON:
	// {
	//     "decks": [ { "filename": "...", "name": "...", "main": [...], "extra": [...], "side": [...] } ],
	//     "seats": [ { "deck_name": "...", "ready": true } ]
	// }
	class LobbyState
	{
	public:
		class Seat
		{
		public:
			// empty => 未选择卡组
			std::string deck_name;
			// ready = 已选择卡组 + 已准备
			bool ready;
		};

		constexpr static std::size_t seats_size = 2;

		using seats_type = std::array<Seat, seats_size>;

		// 所有可选卡组
		std::vector<Deck> decks;
		// 当前座位状态 -- 选择卡组+准备
		seats_type seats;
	};

	// 房间同步
	// JSON:
	// {
	//     "started": false,
	//     "finished": false,
	//     "seats": [ { "deck_name": "...", "ready": true, "joined": true } ],
	//     "lobby": { ... }
	// }
	class RoomSync
	{
	public:
		class Seat
		{
		public:
			// 使用的卡组
			std::string deck_name;
			// 已准备
			bool ready;
			// 已加入/已连接
			bool joined;
		};

		using seats_type = std::array<Seat, LobbyState::seats_size>;

		// 决斗已开始
		bool started;
		// 决斗已结束
		bool finished;
		// 座位信息
		seats_type seats;
		// 决斗开始前附带大厅信息
		std::optional<LobbyState> lobby;
	};

	// 决斗请求
	// JSON:
	// {
	//     "request_id": 42,
	//     "payload": { ... driver::request::Request ... }
	// }
	class Request
	{
	public:
		std::uint64_t request_id;
		driver::request::Request payload;
	};

	// 日志
	// JSON:
	// {
	//     "sequence": 12,
	//     "entry": { ... }
	// }
	class Log
	{
	public:
		std::uint64_t sequence;
		// 引擎定义的日志内容
		nlohmann::json entry;
	};

	// 错误
	// JSON:
	// {
	//     "message": "...",
	//     "source": "response",
	//     "request_id": 7
	// }
	class Error
	{
	public:
		std::string message;
		// 出错消息的type(若能提取)
		std::optional<std::string> source;
		// 出错请求的ID(仅适用于回应链路)
		std::optional<std::uint64_t> request_id;
	};

	// =================================================================
	// Message
	// =================================================================

	class Message : public std::variant<
				// 客户端 -> 服务器
				Join,
				ChooseDeck,
				Surrender,
				Sync,
				Response,
				// 服务器 -> 客户端
				Joined,
				LobbyState,
				RoomSync,
				Request,
				Log,
				Error
			> {};
}

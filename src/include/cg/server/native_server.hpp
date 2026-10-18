#pragma once

#include <array>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include <cg/domain/player.hpp>

#include <cg/driver/response.hpp>

#include <cg/server/message.hpp>
#include <cg/server/room.hpp>
#include <cg/server/server.hpp>

namespace cg::server
{
	class NativeOptions
	{
	public:
		RoomOptions room;
	};

	class NativeServer final : public Server
	{
	public:
		class NativeClient final : public Room::Client
		{
		public:
			explicit NativeClient(domain::Player seat) noexcept;

			NativeClient(const NativeClient&) noexcept = delete;
			auto operator=(const NativeClient&) noexcept -> NativeClient& = delete;
			NativeClient(NativeClient&&) noexcept = delete;
			auto operator=(NativeClient&&) noexcept -> NativeClient& = delete;

			~NativeClient() noexcept override = default;

			[[nodiscard]] auto seat() const noexcept -> domain::Player;

			// Room(服务器)线程调用 -- 非阻塞
			void deliver(const message::Message& message) noexcept override;

			// UI线程调用
			[[nodiscard]] auto try_pop() noexcept -> std::optional<message::Message>;
			[[nodiscard]] auto wait() noexcept -> std::optional<message::Message>;

			// 关闭队列,唤醒等待者
			auto close() noexcept -> void;

		private:
			domain::Player seat_;
			mutable std::mutex mutex_;
			std::condition_variable condition_variable_;
			std::deque<message::Message> queue_;
			bool closed_ = false;
		};

	private:
		NativeOptions options_;
		Room room_;
		std::array<std::unique_ptr<NativeClient>, Room::max_peer> clients_;

	public:
		explicit NativeServer(NativeOptions options) noexcept;
		~NativeServer() noexcept override;

		NativeServer(const NativeServer&) noexcept = delete;
		auto operator=(const NativeServer&) noexcept -> NativeServer& = delete;

		NativeServer(NativeServer&&) noexcept = delete;
		auto operator=(NativeServer&&) noexcept -> NativeServer& = delete;

		auto initialize() noexcept -> bool override;
		auto run() noexcept -> int override;

		[[nodiscard]] auto client(domain::Player player) const noexcept -> NativeClient&;

		auto choose_deck(domain::Player player, std::string deck) noexcept -> void;
		auto surrender(domain::Player player) noexcept -> void;
		auto respond(domain::Player player, std::uint64_t request_id, driver::response::Response response) noexcept -> void;

		auto stop() noexcept -> void;
	};
}

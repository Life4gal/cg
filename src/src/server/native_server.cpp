#include <cg/server/native_server.hpp>

#include <utility>

#include <spdlog/spdlog.h>

#define NATIVE_LOG_PREFIX "[NativeServer]: "

namespace cg::server
{
	// ==============================================================================================
	// NativeClient
	// ==============================================================================================

	NativeServer::NativeClient::NativeClient(const domain::Player seat) noexcept
		: seat_{seat}
	{
		//
	}

	auto NativeServer::NativeClient::seat() const noexcept -> domain::Player
	{
		return seat_;
	}

	void NativeServer::NativeClient::deliver(const message::Message& message) noexcept
	{
		{
			std::scoped_lock lock{mutex_};
			if (closed_)
			{
				return;
			}

			queue_.push_back(message);
		}

		condition_variable_.notify_one();
	}

	auto NativeServer::NativeClient::try_pop() noexcept -> std::optional<message::Message>
	{
		std::scoped_lock lock{mutex_};
		if (queue_.empty())
		{
			return std::nullopt;
		}

		auto message = std::move(queue_.front());
		queue_.pop_front();
		return message;
	}

	auto NativeServer::NativeClient::wait() noexcept -> std::optional<message::Message>
	{
		std::unique_lock lock{mutex_};
		condition_variable_.wait(
			lock,
			[this] noexcept -> bool
			{
				return closed_ || !queue_.empty();
			}
		);

		if (queue_.empty())
		{
			return std::nullopt;
		}

		auto message = std::move(queue_.front());
		queue_.pop_front();
		return message;
	}

	auto NativeServer::NativeClient::close() noexcept -> void
	{
		{
			std::scoped_lock lock{mutex_};
			closed_ = true;
		}

		condition_variable_.notify_all();
	}

	// ==============================================================================================
	// NativeServer
	// ==============================================================================================

	NativeServer::NativeServer(NativeOptions options) noexcept
		: options_{std::move(options)},
		  room_{options_.room}
	{
		//
	}

	NativeServer::~NativeServer() noexcept
	{
		stop();
	}

	auto NativeServer::initialize() noexcept -> bool
	{
		if (!room_.initialize())
		{
			return false;
		}

		for (std::size_t index = 0; index < Room::max_peer; ++index)
		{
			clients_[index] = std::make_unique<NativeClient>(static_cast<domain::Player>(index));
		}

		if (!room_.join(*clients_[std::to_underlying(domain::Player::FIRST)], domain::Player::FIRST, "").has_value())
		{
			return false;
		}
		if (!room_.join(*clients_[std::to_underlying(domain::Player::SECOND)], domain::Player::SECOND, "").has_value())
		{
			return false;
		}

		return true;
	}

	auto NativeServer::run() noexcept -> int
	{
		// 决斗在后台线程运行,这里只需等待其结束或被stop
		room_.wait();
		return 0;
	}

	auto NativeServer::client(const domain::Player player) const noexcept -> NativeClient&
	{
		return *clients_[std::to_underlying(player)];
	}

	auto NativeServer::choose_deck(const domain::Player player, std::string deck) noexcept -> void
	{
		room_.dispatch(player, message::ChooseDeck{.deck = std::move(deck)});
	}

	auto NativeServer::surrender(const domain::Player player) noexcept -> void
	{
		room_.dispatch(player, message::Surrender{});
	}

	auto NativeServer::respond(const domain::Player player, const std::uint64_t request_id, driver::response::Response response) noexcept -> void
	{
		room_.dispatch(player, message::Response{.request_id = request_id, .payload = std::move(response)});
	}

	auto NativeServer::stop() noexcept -> void
	{
		room_.stop();

		for (auto& client: clients_)
		{
			if (client != nullptr)
			{
				client->close();
			}
		}
	}
}

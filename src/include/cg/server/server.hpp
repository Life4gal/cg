#pragma once

namespace cg::server
{
	class Server
	{
	public:
		constexpr Server() noexcept = default;

		constexpr Server(const Server&) noexcept = delete;
		constexpr auto operator=(const Server&) noexcept -> Server& = delete;
		constexpr Server(Server&&) noexcept = default;
		constexpr auto operator=(Server&&) noexcept -> Server& = default;

		constexpr virtual ~Server() noexcept = default;

		virtual auto initialize() noexcept -> bool = 0;

		virtual auto run() noexcept -> int = 0;
	};
}

#pragma once

#include <cg/driver/request.hpp>
#include <cg/driver/response.hpp>

namespace cg::driver
{
	class Driver
	{
	public:
		constexpr Driver() noexcept = default;

		constexpr Driver(const Driver&) noexcept = delete;
		constexpr auto operator=(const Driver&) noexcept -> Driver& = delete;
		constexpr Driver(Driver&&) noexcept = default;
		constexpr auto operator=(Driver&&) noexcept -> Driver& = default;

		constexpr virtual ~Driver() noexcept = default;

		virtual auto decide(const Request& request) noexcept -> Response = 0;
	};
}

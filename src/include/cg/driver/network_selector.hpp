#pragma once

#include <cg/execution/decision.hpp>

#include <cg/driver/request.hpp>
#include <cg/driver/response.hpp>

namespace cg::driver
{
	// TODO
	class NetworkSelector : public execution::Selector
	{
	public:
		NetworkSelector() noexcept = default;

		NetworkSelector(const NetworkSelector&) noexcept = delete;
		auto operator=(const NetworkSelector&) noexcept -> NetworkSelector& = delete;
		NetworkSelector(NetworkSelector&&) noexcept = default;
		auto operator=(NetworkSelector&&) noexcept -> NetworkSelector& = default;

		~NetworkSelector() noexcept override = default;

		virtual auto decide(const request::Request& request) noexcept -> response::Response = 0;
	};
}

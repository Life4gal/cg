#pragma once

#include <expected>

#include <cg/driver/request.hpp>
#include <cg/driver/response.hpp>

#include <nlohmann/json.hpp>

namespace cg::server
{
	[[nodiscard]] auto encode_request(const driver::Request& request) noexcept -> nlohmann::json;

	[[nodiscard]] auto decode_response(const driver::Request& request, const nlohmann::json& message) noexcept -> std::expected<driver::Response, std::string>;
}

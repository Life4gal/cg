#include <cg/server/codec.hpp>

namespace cg::server
{
	auto encode_request(const driver::Request& request) noexcept -> nlohmann::json
	{
		auto json = ValueCodec<driver::Request>::encode(request);
		json["type"] = "request";

		return json;
	}

	auto decode_response(const driver::Request& request, const nlohmann::json& message) noexcept -> std::expected<driver::Response, std::string>
	{
		if (!message.is_object() || message.value("type", "") != "response")
		{
			return std::unexpected<std::string>{"expected a message of type \"response\""};
		}

		if (!message.contains("response") || !message.at("response").is_object())
		{
			return std::unexpected<std::string>{"response message requires an object field \"response\""};
		}

		const auto& response = message.at("response");
		// TODO
		return {};
	}
}

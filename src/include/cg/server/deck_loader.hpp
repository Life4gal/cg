#pragma once

#include <expected>

#include <nlohmann/json_fwd.hpp>

#include <cg/server/deck.hpp>

namespace cg::server
{
	[[nodiscard]] auto load_json_deck(const nlohmann::json& deck) noexcept -> std::expected<Deck, std::string>;
}

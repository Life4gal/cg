#pragma once

#include <string>
#include <vector>
#include <expected>

#include <cg/domain/card.hpp>

#include <nlohmann/json_fwd.hpp>

namespace cg::server
{
	class Deck
	{
	public:
		// 文件名(不含扩展名)
		// 该名字为卡组代号
		std::string filename;
		// 卡组名字,如果没有则为文件名
		// 该名字仅用于显示
		std::string name;

		std::vector<domain::CardCode> main_deck;
		std::vector<domain::CardCode> extra_deck;
		std::vector<domain::CardCode> side_deck;

		[[nodiscard]] static auto load(const nlohmann::json& json) noexcept -> std::expected<Deck, std::string>;
	};
}

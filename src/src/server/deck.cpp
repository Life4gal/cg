#include <cg/server/deck.hpp>

#include <format>

#include <nlohmann/json.hpp>

namespace cg::server
{
	auto Deck::load(const nlohmann::json& json) noexcept -> std::expected<Deck, std::string>
	{
		if (!json.is_object())
		{
			return std::unexpected<std::string>{"卡组JSON必须是一个Object,如`{ \"main\": [...], \"extra\": [...], \"side\": [...] }`"};
		}

		std::string error{};
		const auto reader = [&](const std::string_view key, const bool optional, std::vector<domain::CardCode>& out) noexcept -> void
		{
			try
			{
				if (!json.contains(key))
				{
					if (!optional)
					{
						error = std::format("卡组JSON不存在{}键!", key);
					}

					return;
				}

				const auto& value = json.at(key);
				if (!value.is_array())
				{
					error = std::format("卡组JSON的{}内容必须是一个数组!", key);
					return;
				}

				for (const auto& item: value)
				{
					if (!item.is_number_unsigned())
					{
						error = std::format("卡牌卡密必须是8位正整数,当前为{}!", item.dump());
						return;
					}

					const auto card_code = item.get<std::underlying_type_t<domain::CardCode>>();
					// 需不需要检查是否是8位正整数?
					out.push_back(static_cast<domain::CardCode>(card_code));
				}
			}
			catch (const nlohmann::json::type_error& type_error)
			{
				error = std::format("解析{}时出错: {}", key, type_error.what());
			}
			catch (const nlohmann::json::out_of_range& out_of_range)
			{
				error = std::format("解析{}时出错: {}", key, out_of_range.what());
			}
			catch (const nlohmann::json::parse_error& parse_error)
			{
				error = std::format("解析{}时出错: {}", key, parse_error.what());
			}
		};

		Deck deck
		{
				// 外部设置
				.filename = "",
				// 如果没有则为空,外部检查
				.name = "",
				.main_deck = {},
				.extra_deck = {},
				.side_deck = {},
		};
		// 卡组名
		if (json.contains("name"))
		{
			if (const auto& name = json.at("name");
				name.is_string())
			{
				deck.name = name.get<std::string>();
			}
			else
			{
				deck.name = name.dump();
			}
		}

		// 主卡组
		reader("main", false, deck.main_deck);
		if (!error.empty())
		{
			return std::unexpected{std::move(error)};
		}
		// 额外卡组
		reader("extra", true, deck.extra_deck);
		if (!error.empty())
		{
			return std::unexpected{std::move(error)};
		}
		// 副卡组
		reader("side", true, deck.side_deck);
		if (!error.empty())
		{
			return std::unexpected{std::move(error)};
		}

		return deck;
	}
}

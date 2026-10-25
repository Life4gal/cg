#pragma once

#include <array>
#include <expected>
#include <filesystem>
#include <string>

#include <cg/flow/duel_config.hpp>

namespace cg::server
{
	// 本地对局配置: 卡池路径 + 双方卡组路径 + Duel 初始化参数
	class MatchConfig
	{
	public:
		std::filesystem::path cards_path;
		std::array<std::filesystem::path, 2> deck_paths;

		flow::DuelConfig duel;

		// 从 JSON 配置文件加载; 相对路径以配置文件所在目录为基准
		[[nodiscard]] static auto load(const std::filesystem::path& file) noexcept -> std::expected<MatchConfig, std::string>;
	};
}

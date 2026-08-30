#pragma once

#include <string_view>

namespace cg::engine
{
	using series_code_type = std::uint32_t;

	class SeriesRegistry
	{
	public:
		// 注册字段(如果还未注册),返回字段代码
		[[nodiscard]] static auto register_series(std::string_view series_name) noexcept -> series_code_type;

		// 检查字段是否已注册
		[[nodiscard]] static auto registered(std::string_view series_name) noexcept -> series_code_type;

		// 获取已注册字段数量
		[[nodiscard]] static auto size() noexcept -> std::size_t;

		// 清空注册的字段
		static auto clear() noexcept -> void;
	};
}

#pragma once

#include <string_view>

#include <core/domain/card.hpp>

namespace cg::engine
{
	class SeriesRegistry
	{
	public:
		// 注册字段(如果还未注册),返回字段代码
		[[nodiscard]] static auto register_series(std::string_view series_name) noexcept -> domain::SeriesCode;

		// 检查字段是否已注册
		[[nodiscard]] static auto registered(std::string_view series_name) noexcept -> bool;

		// 检查字段代码是否已注册
		[[nodiscard]] static auto registered(domain::SeriesCode code) noexcept -> bool;

		// 获取字段名称
		[[nodiscard]] static auto series_name(domain::SeriesCode code) noexcept -> std::string_view;

		// 获取已注册字段数量
		[[nodiscard]] static auto size() noexcept -> std::size_t;

		// 清空注册的字段
		static auto clear() noexcept -> void;
	};
}

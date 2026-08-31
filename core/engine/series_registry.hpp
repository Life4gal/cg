#pragma once

#include <string>
#include <unordered_map>

#include <core/utility/string_hash.hpp>

#include <core/domain/card.hpp>

namespace cg::engine
{
	class SeriesRegistry
	{
	public:
		class SeriesCodeWrapper : public utility::Enum<
					domain::SeriesCode,
					// +, +=, -, -=
					utility::EnumSupport::ARITHMETIC
				> {};

	private:
		std::unordered_map<std::string, domain::SeriesCode, utility::StringHash, std::equal_to<>> registered_series_;
		std::unordered_map<domain::SeriesCode, std::string_view> registered_series_name_;
		SeriesCodeWrapper series_code_generator_;

	public:
		SeriesRegistry() noexcept;

		// 注册字段(如果还未注册),返回字段代码
		[[nodiscard]] auto register_series(std::string_view series_name) noexcept -> domain::SeriesCode;

		// 检查字段是否已注册
		[[nodiscard]] auto registered(std::string_view series_name) const noexcept -> bool;

		// 检查字段代码是否已注册
		[[nodiscard]] auto registered(domain::SeriesCode code) const noexcept -> bool;

		// 获取字段名称
		[[nodiscard]] auto series_name(domain::SeriesCode code) const noexcept -> std::string_view;

		// 获取已注册字段数量
		[[nodiscard]] auto size() const noexcept -> std::size_t;

		// 清空注册的字段
		auto clear() noexcept -> void;
	};
}

#include <core/engine/series_registry.hpp>

namespace
{
	using namespace cg;

	constexpr auto series_code_begin = static_cast<domain::SeriesCode>(1);
	constexpr auto series_code_step = static_cast<domain::SeriesCode>(1);
}

namespace cg::engine
{
	SeriesRegistry::SeriesRegistry() noexcept
		: series_code_generator_{series_code_begin} {}

	auto SeriesRegistry::register_series(const std::string_view series_name) noexcept -> domain::SeriesCode
	{
		if (const auto it = registered_series_.find(series_name);
			it != registered_series_.end())
		{
			return it->second;
		}

		const auto next_series_code = static_cast<domain::SeriesCode>(series_code_generator_);
		series_code_generator_ += series_code_step;

		const auto [it, _] = registered_series_.emplace(series_name, next_series_code);
		registered_series_name_.emplace(it->second, it->first);

		return it->second;
	}

	auto SeriesRegistry::registered(const std::string_view series_name) const noexcept -> bool
	{
		return registered_series_.contains(series_name);
	}

	auto SeriesRegistry::registered(const domain::SeriesCode code) const noexcept -> bool
	{
		return registered_series_name_.contains(code);
	}

	auto SeriesRegistry::series_name(const domain::SeriesCode code) const noexcept -> std::string_view
	{
		if (const auto it = registered_series_name_.find(code);
			it != registered_series_name_.end())
		{
			return it->second;
		}

		return {};
	}

	auto SeriesRegistry::size() const noexcept -> std::size_t
	{
		return registered_series_.size();
	}

	auto SeriesRegistry::clear() noexcept -> void
	{
		registered_series_.clear();
	}
}

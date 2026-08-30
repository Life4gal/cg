#include <core/engine/series_registry.hpp>

#include <unordered_map>

#include <core/utility/string_hash.hpp>

namespace
{
	using namespace cg;

	std::unordered_map<std::string, engine::series_code_type, utility::StringHash, std::equal_to<>> g_registered_series;
	engine::series_code_type g_series_code_generator = 1;
}

namespace cg::engine
{
	auto SeriesRegistry::register_series(const std::string_view series_name) noexcept -> series_code_type
	{
		if (const auto it = g_registered_series.find(series_name);
			it != g_registered_series.end())
		{
			return it->second;
		}

		const auto next_series_code = g_series_code_generator;
		g_series_code_generator += 1;

		const auto [it, _] = g_registered_series.emplace(series_name, next_series_code);
		return it->second;
	}

	auto SeriesRegistry::registered(const std::string_view series_name) noexcept -> series_code_type
	{
		return g_registered_series.contains(series_name);
	}

	auto SeriesRegistry::size() noexcept -> std::size_t
	{
		return g_registered_series.size();
	}

	auto SeriesRegistry::clear() noexcept -> void
	{
		g_registered_series.clear();
	}
}

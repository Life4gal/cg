#include <core/engine/series_registry.hpp>

#include <unordered_map>

#include <core/utility/string_hash.hpp>

namespace
{
	using namespace cg;

	class SeriesCodeWrapper : public utility::Enum<
				domain::SeriesCode,
				// +, +=, -, -=
				utility::EnumSupport::ARITHMETIC
			> {};

	constexpr auto series_code_begin = static_cast<domain::SeriesCode>(1);
	constexpr auto series_code_step = static_cast<domain::SeriesCode>(1);

	std::unordered_map<std::string, domain::SeriesCode, utility::StringHash, std::equal_to<>> g_registered_series;
	std::unordered_map<domain::SeriesCode, std::string_view> g_registered_series_name;
	SeriesCodeWrapper g_series_code_generator{series_code_begin};
}

namespace cg::engine
{
	auto SeriesRegistry::register_series(const std::string_view series_name) noexcept -> domain::SeriesCode
	{
		if (const auto it = g_registered_series.find(series_name);
			it != g_registered_series.end())
		{
			return it->second;
		}

		const auto next_series_code = static_cast<domain::SeriesCode>(g_series_code_generator);
		g_series_code_generator += series_code_step;

		const auto [it, _] = g_registered_series.emplace(series_name, next_series_code);
		g_registered_series_name.emplace(it->second, it->first);

		return it->second;
	}

	auto SeriesRegistry::registered(const std::string_view series_name) noexcept -> bool
	{
		return g_registered_series.contains(series_name);
	}

	auto SeriesRegistry::registered(const domain::SeriesCode code) noexcept -> bool
	{
		return g_registered_series_name.contains(code);
	}

	auto SeriesRegistry::series_name(const domain::SeriesCode code) noexcept -> std::string_view
	{
		if (const auto it = g_registered_series_name.find(code);
			it != g_registered_series_name.end())
		{
			return it->second;
		}

		return {};
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

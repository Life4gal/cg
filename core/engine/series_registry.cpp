#include <core/engine/series_registry.hpp>

#include <unordered_map>

#include <core/utility/string_hash.hpp>
#include <core/utility/enum.hpp>

namespace
{
	using namespace cg;

	class SeriesCodeWrapper : public utility::Enum<
				engine::SeriesCode,
				// +, +=, -, -=
				utility::EnumSupport::ARITHMETIC
			> {};

	constexpr auto series_code_begin = static_cast<engine::SeriesCode>(1);
	constexpr auto series_code_step = static_cast<engine::SeriesCode>(1);

	std::unordered_map<std::string, engine::SeriesCode, utility::StringHash, std::equal_to<>> g_registered_series;
	std::unordered_map<engine::SeriesCode, std::string_view> g_registered_series_name;
	SeriesCodeWrapper g_series_code_generator{series_code_begin};
}

namespace cg::engine
{
	auto SeriesRegistry::register_series(const std::string_view series_name) noexcept -> SeriesCode
	{
		if (const auto it = g_registered_series.find(series_name);
			it != g_registered_series.end())
		{
			return it->second;
		}

		const auto next_series_code = static_cast<SeriesCode>(g_series_code_generator);
		g_series_code_generator += series_code_step;

		const auto [it, _] = g_registered_series.emplace(series_name, next_series_code);
		g_registered_series_name.emplace(it->second, it->first);

		return it->second;
	}

	auto SeriesRegistry::registered(const std::string_view series_name) noexcept -> bool
	{
		return g_registered_series.contains(series_name);
	}

	auto SeriesRegistry::registered(const SeriesCode code) noexcept -> bool
	{
		return g_registered_series_name.contains(code);
	}

	auto SeriesRegistry::series_name(const SeriesCode code) noexcept -> std::string_view
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

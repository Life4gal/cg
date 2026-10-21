#include <cg/execution/script_registry.hpp>

#include <charconv>
#include <fstream>

#include <cg/dsl/types.hpp>
#include <cg/dsl/json/reader.hpp>

#include <libassert/assert.hpp>

namespace cg::execution
{
	namespace
	{
		[[nodiscard]] auto parse_code(const std::string& stem) noexcept -> std::optional<domain::CardCode>
		{
			std::underlying_type_t<domain::CardCode> value{};

			if (const auto [ptr, ec] = std::from_chars(stem.data(), stem.data() + stem.size(), value);
				ec != std::errc{} || ptr != stem.data() + stem.size())
			{
				return std::nullopt;
			}

			return static_cast<domain::CardCode>(value);
		}
	}

	auto ScriptRegistry::load_file(const std::filesystem::path& file, dsl::json::Reader& reader) noexcept -> bool
	{
		LIBASSERT_DEBUG_ASSERT(is_regular_file(file));
		LIBASSERT_DEBUG_ASSERT(file.extension() == ".json");

		const auto name = file.filename().string();
		const auto code = parse_code(file.stem().string());
		if (!code.has_value())
		{
			errors_.push_back(std::format("{}: file name is not a card code", name));
			return false;
		}

		std::ifstream stream{file, std::ios::binary};
		if (!stream.good())
		{
			errors_.push_back(std::format("{}: cannot open", name));
			return false;
		}

		std::stringstream buffer{};
		buffer << stream.rdbuf();

		const auto json = nlohmann::json::parse(buffer.str(), nullptr, false);
		if (json.is_discarded())
		{
			errors_.push_back(std::format("{}: invalid JSON", name));
			return false;
		}

		auto script = reader.read_card_script(json, name);
		for (const auto& diagnostic: reader.take_diagnostics())
		{
			errors_.push_back(std::format("{}: {}", diagnostic.path, diagnostic.message));
		}

		if (!script.has_value())
		{
			return false;
		}

		add(*code, *std::move(script));

		return true;
	}

	auto ScriptRegistry::take_errors() noexcept -> std::vector<std::string>
	{
		return std::exchange(errors_, {});
	}

	auto ScriptRegistry::add(const domain::CardCode code, dsl::CardScript script) noexcept -> bool
	{
		return scripts_.insert_or_assign(code, std::move(script)).second;
	}

	auto ScriptRegistry::load_directory(const std::filesystem::path& directory, const dsl::TypeRegistry& types, const dsl::Vocabulary& vocabulary) noexcept -> bool
	{
		errors_.clear();

		dsl::json::Reader reader{types, vocabulary};
		auto ok = true;

		for (const auto& entry: std::filesystem::directory_iterator{directory})
		{
			if (!entry.is_regular_file() || entry.path().extension() != ".json")
			{
				continue;
			}

			if (!load_file(entry.path(), reader))
			{
				ok = false;
			}
		}

		return ok;
	}

	auto ScriptRegistry::load_file(const std::filesystem::path& file, const dsl::TypeRegistry& types, const dsl::Vocabulary& vocabulary) noexcept -> bool
	{
		errors_.clear();

		if (!is_regular_file(file) || file.extension() != ".json")
		{
			return false;
		}

		dsl::json::Reader reader{types, vocabulary};
		return load_file(file, reader);
	}

	auto ScriptRegistry::find(const domain::CardCode code) const noexcept -> const dsl::CardScript*
	{
		if (const auto it = scripts_.find(code);
			it != scripts_.end())
		{
			return &it->second;
		}

		return nullptr;
	}
}

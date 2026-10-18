#include <cg/state/db.hpp>

#include <fstream>

#include <nlohmann/json.hpp>

namespace cg::state
{
	namespace
	{
		[[nodiscard]] auto parse_card_code(const std::string_view stem) noexcept -> std::optional<domain::CardCode>
		{
			std::underlying_type_t<domain::CardCode> value;
			if (const auto result = std::from_chars(stem.data(), stem.data() + stem.size(), value);
				result.ec != std::errc{} || result.ptr != stem.data() + stem.size())
			{
				return std::nullopt;
			}

			return static_cast<domain::CardCode>(value);
		}

		auto parse_json_card_definition(const nlohmann::json& json) noexcept -> DefinitionLoadResult
		{
			DefinitionLoadResult result;

			if (!json.is_object())
			{
				result.errors.emplace_back(
					"",
					"$",
					"card document must be a JSON object"
				);

				return result;
			}

			CardDefinition definition{};

			// TODO
			std::ignore = json;
			std::ignore = definition;

			if (result.errors.empty())
			{
				result.definition = definition;
			}

			return result;
		}
	}

	auto Db::parse(const std::filesystem::path& file) noexcept -> DefinitionLoadResult
	{
		DefinitionLoadResult result;

		const auto stem = file.stem().string();
		const auto code = parse_card_code(stem);
		if (!code.has_value())
		{
			result.errors.emplace_back(
				file,
				"$",
				std::format("invalid card filename \"{}\", must be a decimal card code", file.filename().string())
			);

			return result;
		}

		std::ifstream in{file};
		if (in)
		{
			result.errors.emplace_back(
				file,
				"$",
				std::format("cannot open file \"{}\"", file.string())
			);

			return result;
		}

		if (file.extension() == "json")
		{
			try
			{
				const auto document = nlohmann::json::parse(in);
				result = parse_json_card_definition(document);

				if (!result.errors.empty())
				{
					std::ranges::for_each(
						result.errors,
						[&file](DbError& error) noexcept -> void
						{
							if (error.file.empty())
							{
								error.file = file;
							}
						}
					);
				}
			}
			catch (const nlohmann::json::parse_error& error)
			{
				result.errors.emplace_back(
					file,
					"$",
					std::format("JSON parse error: {}", error.what())
				);

				return result;
			}
		}
		else
		{
			// TODO
		}

		if (result.definition.has_value())
		{
			result.definition->code = *code;
		}
		return result;
	}

	auto Db::load_file(const std::filesystem::path& file) noexcept -> bool
	{
		if (auto result = parse(file);
			result.definition.has_value())
		{
			const auto code = result.definition->code;
			definitions_.insert_or_assign(code, *result.definition);

			return true;
		}
		else
		{
			errors_.append_range(std::move(result.errors));

			return false;
		}
	}

	auto Db::load_directory(const std::filesystem::path& directory) noexcept -> bool
	{
		std::error_code error_code{};
		const std::filesystem::directory_iterator it{directory, error_code};
		if (error_code != std::error_code{})
		{
			DbError error
			{
					.file = directory,
					.field = "$",
					.message = std::format("cannot open directory \"{}\": {}", directory.string(), error_code.message())
			};

			errors_.push_back(std::move(error));

			return false;
		}

		bool all_succeed = true;
		for (const auto& entry: it)
		{
			all_succeed &= load_file(entry.path());
		}

		return all_succeed;
	}

	auto Db::load(const std::filesystem::path& path) noexcept -> bool
	{
		std::error_code error_code{};

		if (!exists(path, error_code))
		{
			return false;
		}

		if (is_regular_file(path, error_code))
		{
			return load_file(path);
		}

		if (is_directory(path, error_code))
		{
			return load_directory(path);
		}

		return false;
	}

	auto Db::ok() const noexcept -> bool
	{
		return errors_.empty();
	}

	auto Db::errors() const noexcept -> std::span<const DbError>
	{
		return errors_;
	}

	auto Db::contains(const domain::CardCode code) const noexcept -> bool
	{
		return definitions_.contains(code);
	}

	auto Db::find(const domain::CardCode code) noexcept -> const CardDefinition*
	{
		const auto it = definitions_.find(code);
		if (it == definitions_.end())
		{
			return nullptr;
		}

		return &it->second;
	}
}

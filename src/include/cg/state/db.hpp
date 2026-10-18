#pragma once

#include <filesystem>
#include <unordered_map>
#include <span>

#include <cg/domain/card_definition.hpp>

namespace cg::state
{
	class DbError
	{
	public:
		std::filesystem::path file;
		std::string field;
		std::string message;
	};

	class DefinitionLoadResult
	{
	public:
		// 成功 -> 非null-opt
		std::optional<domain::CardDefinition> definition;
		// 失败 -> 非空
		std::vector<DbError> errors;
	};

	class Db
	{
	public:
		using definitions_type = std::unordered_map<domain::CardCode, domain::CardDefinition>;

	private:
		std::unordered_map<domain::CardCode, domain::CardDefinition> definitions_;
		std::vector<DbError> errors_;

		static auto parse(const std::filesystem::path& file) noexcept -> DefinitionLoadResult;

		auto load_file(const std::filesystem::path& file) noexcept -> bool;
		auto load_directory(const std::filesystem::path& directory) noexcept -> bool;

	public:
		auto load(const std::filesystem::path& path) noexcept -> bool;

		[[nodiscard]] auto ok() const noexcept -> bool;

		[[nodiscard]] auto errors() const noexcept -> std::span<const DbError>;

		[[nodiscard]] auto contains(domain::CardCode code) const noexcept -> bool;

		[[nodiscard]] auto find(domain::CardCode code) noexcept -> const domain::CardDefinition*;
	};
}

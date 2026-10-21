#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

#include <cg/dsl/card_script.hpp>

namespace cg::dsl
{
	class TypeRegistry;
	class Vocabulary;

	namespace json
	{
		class Reader;
	}
}

namespace cg::execution
{
	// 卡密 -> 卡脚本(执行层注册表)
	class ScriptRegistry
	{
	public:
		using scripts_type = std::unordered_map<domain::CardCode, dsl::CardScript>;

	private:
		scripts_type scripts_;
		std::vector<std::string> errors_;

		auto load_file(const std::filesystem::path& file, dsl::json::Reader& reader) noexcept -> bool;

	public:
		[[nodiscard]] auto take_errors() noexcept -> std::vector<std::string>;

		auto add(domain::CardCode code, dsl::CardScript script) noexcept -> bool;

		auto load_directory(const std::filesystem::path& directory, const dsl::TypeRegistry& types, const dsl::Vocabulary& vocabulary) noexcept -> bool;

		auto load_file(const std::filesystem::path& file, const dsl::TypeRegistry& types, const dsl::Vocabulary& vocabulary) noexcept -> bool;

		[[nodiscard]] auto find(domain::CardCode code) const noexcept -> const dsl::CardScript*;

		[[nodiscard]] auto size() const noexcept -> std::size_t
		{
			return scripts_.size();
		}
	};
}

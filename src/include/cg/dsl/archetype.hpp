#pragma once

#include <string>
#include <unordered_map>

#include <cg/dsl/id.hpp>

namespace cg::dsl
{
	class ArchetypeRegistry
	{
		// 理论上不需要,不过可以用于定位问题
		static std::unordered_map<std::uint64_t, std::string> registry_;

		[[nodiscard]] static auto do_hash(std::string_view text) noexcept -> std::uint64_t;

	public:
		[[nodiscard]] static auto id_of(std::string_view text) noexcept -> ArchetypeId;

		[[nodiscard]] static auto name_of(ArchetypeId id) noexcept -> std::string_view;
	};
}

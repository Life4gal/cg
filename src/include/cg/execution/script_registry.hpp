#pragma once

#include <unordered_map>

#include <cg/dsl/card_script.hpp>

namespace cg::execution
{
	// 卡密 -> 卡脚本(执行层注册表)
	class ScriptRegistry
	{
	public:
		using scripts_type = std::unordered_map<domain::CardCode, dsl::CardScript>;

	private:
		scripts_type scripts_;

	public:
		auto add(domain::CardCode code, dsl::CardScript script) noexcept -> void
		{
			scripts_.insert_or_assign(code, std::move(script));
		}

		[[nodiscard]] auto find(const domain::CardCode code) const noexcept -> const dsl::CardScript*
		{
			if (const auto it = scripts_.find(code);
				it != scripts_.end())
			{
				return &it->second;
			}

			return nullptr;
		}
	};
}

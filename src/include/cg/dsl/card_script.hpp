#pragma once

#include <vector>

#include <cg/dsl/identity.hpp>
#include <cg/dsl/effect.hpp>
#include <cg/dsl/summon_procedure.hpp>

namespace cg::dsl
{
	// ================================================================
	// 卡脚本: 卡的身份 + 召唤手续 + 效果 + 使用次数限制
	// ================================================================

	class CardScript
	{
	public:
		CardIdentity identity;
		std::vector<SummonProcedure> summons;
		std::vector<Effect> effects;
		std::vector<UsageRestriction> usage_restrictions;

		[[nodiscard]] constexpr auto operator==(const CardScript&) const -> bool = default;
	};
}

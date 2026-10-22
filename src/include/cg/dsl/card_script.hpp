#pragma once

#include <cg/dsl/identity.hpp>
#include <cg/dsl/effect.hpp>
#include <cg/dsl/summon_procedure.hpp>

namespace cg::dsl
{
	// ================================================================
	// 卡脚本: 卡的身份 + 召唤手续 + 效果
	// ================================================================

	class CardScript
	{
	public:
		CardIdentity identity;
		SummonProcedureList summons;
		EffectList effects;

		[[nodiscard]] constexpr auto operator==(const CardScript&) const -> bool = default;
	};
}

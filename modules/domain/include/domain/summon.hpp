#pragma once

#include <cstdint>

namespace cg::domain
{
	// 召唤方式
	enum class SummonMethod : std::uint8_t
	{
		// 通常召唤
		NORMAL,
		// 上级召唤
		ADVANCE,
		// 特殊召唤(不包括仪式、融合、同调、超量、灵摆、连接)
		SPECIAL,
		// 仪式召唤
		RITUAL,
		// 融合召唤
		FUSION,
		// 同调召唤
		SYNCHRO,
		// 超量召唤
		XYZ,
		// 灵摆召唤
		PENDULUM,
		// 连接召唤
		LINK,
	};

	// 仪式召唤等级要求
	enum class RitualSummonLevel : std::uint8_t
	{
		// 严格等于
		EQUAL,
		// 至少等于
		AT_LEAST,
	};

	// 召唤素材来源
	enum class SummonMaterialSource : std::uint8_t
	{
		// 墓地也可以作为素材
		GRAVEYARD = 1 << 0,
		// 对手场上也可以作为素材
		OPPONENT_FIELD = 1 << 1,
		// 卡组也可以作为素材
		DECK = 1 << 2,
		// 额外卡组也可以作为素材
		EXTRA_DECK = 1 << 3,
	};
}

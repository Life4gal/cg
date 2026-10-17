#pragma once

#include <cstdint>

namespace cg::domain
{
	// 召唤方式
	enum class SummonMethod : std::uint8_t
	{
		// 通常召唤
		NORMAL,
		// 上级召唤 == 通常召唤 + 需要祭品/素材
		ADVANCE,
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
		// 特殊召唤(不包括仪式、融合、同调、超量、灵摆、连接)(规则特招或者效果特招)
		SPECIAL,
	};

	// 通常召唤类型
	enum class NormalSummonType : std::uint8_t
	{
		// 召唤 -- 表侧攻击表示
		SUMMON,
		// 盖放 -- 里侧守备表示
		SET,
	};

	// 召唤素材来源
	enum class SummonMaterialSource : std::uint8_t
	{
		// 手牌可以作为素材
		HAND,
		// 场上可以作为素材
		FIELD,
		// 墓地可以作为素材
		GRAVEYARD,
		// 对手场上可以作为素材
		OPPONENT_FIELD,
		// 卡组可以作为素材
		DECK,
		// 额外卡组可以作为素材
		EXTRA_DECK,
	};
}

#pragma once

#include <cstdint>

namespace cg::domain
{
	// 召唤方式(占用高8位)
	enum class SummonKind : std::uint32_t
	{
		// 通常召唤
		NORMAL = 0x10'000000,
		// 反转召唤
		FLIP = 0x20'000000,
		// 特殊召唤
		SPECIAL = 0x40'000000,

		// 上级召唤
		ADVANCE = NORMAL | 0x01'000000,
		// 二重召唤
		DUAL = NORMAL | 0x02'000000,

		// 融合召唤
		FUSION = SPECIAL | 0x03'000000,
		// 仪式召唤
		RITUAL = SPECIAL | 0x04'000000,
		// 同调召唤
		SYNCHRO = SPECIAL | 0x05'000000,
		// 超量召唤
		XYZ = SPECIAL | 0x06'000000,
		// 灵摆召唤
		PENDULUM = SPECIAL | 0x07'000000,
		// 连接召唤
		LINK = SPECIAL | 0x08'000000,
	};
}

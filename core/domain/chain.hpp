#pragma once

#include <type_traits>

namespace cg::domain
{
	enum class ChainId : std::uint8_t {};

	// 最多连锁255次
	constexpr auto chain_max_count = static_cast<std::underlying_type_t<ChainId>>(255);

	enum class ChainFlag : std::uint8_t
	{
		// 无效发动
		DISABLE_ACTIVATE = 1 << 0,
		// 无效效果
		DISABLE_EFFECT = 1 << 1,
		// 
	};
}

#pragma once

#include <core/engine/collection.hpp>

namespace cg::engine
{
	class TargetInfo
	{
	public:
		// 当前对象信息所属卡牌 -- 冗余设计,简化接口
		CardReference owner;

		// 本卡指定为对象的卡
		Group targets_to;
		// 以本卡为对象的卡
		Group targets_from;

		explicit TargetInfo(CardReference owner) noexcept;

		// 将目标卡指定为本卡的对象
		auto target_to(CardReference target) noexcept -> void;
		// 取消目标卡为本卡的对象
		auto cancel_target(CardReference target) noexcept -> void;
	};
}

#include <core/engine/target_info.hpp>

#include <core/engine/card.hpp>

namespace cg::engine
{
	TargetInfo::TargetInfo(const CardReference owner) noexcept
		: owner{owner}
	// targets_to{},
	// targets_from{}
	{}

	auto TargetInfo::target_to(const CardReference target) noexcept -> void
	{
		auto& target_target = target.get().target_;

		// 将目标卡加入本卡的`本卡指定为对象的卡`
		targets_to.insert(target);
		// 将本卡加入目标卡的`以本卡为对象的卡`
		target_target.targets_from.insert(owner);
	}

	auto TargetInfo::cancel_target(const CardReference target) noexcept -> void
	{
		auto& target_target = target.get().target_;

		// 将目标卡移出本卡的`本卡指定为对象的卡`
		targets_to.erase(target);
		// 将本卡移出目标卡的`以本卡为对象的卡`
		target_target.targets_from.erase(owner);
	}
}

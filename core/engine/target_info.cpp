#include <core/engine/target_info.hpp>

#include <core/engine/card.hpp>

namespace cg::engine
{
	auto TargetInfo::set_owner_target(Card& card) noexcept -> bool
	{
		return owner_targets.insert(card);
	}

	auto TargetInfo::cancel_owner_target(const Card& card) noexcept -> bool
	{
		return owner_targets.erase(card);
	}

	TargetInfo::TargetInfo() noexcept = default;
	// 	: card_targets_{},
	// 	  owner_targets_{}
	// {
	// 	//
	// }

	auto TargetInfo::set_target(Card& target, Card& owner) noexcept -> bool
	{
		if (card_targets.insert(target))
		{
			// 将本卡加入目标卡的"以本卡为对象的卡"列表中
			target.target_.set_owner_target(owner);

			// TODO: 发出事件,广播该卡被设为目标

			return true;
		}

		return false;
	}

	auto TargetInfo::cancel_target(Card& target, const Card& owner) noexcept -> bool
	{
		if (card_targets.erase(target))
		{
			// 将本卡移出目标卡的"以本卡为对象的卡"列表中
			target.target_.cancel_owner_target(owner);

			return true;
		}

		return false;
	}
}

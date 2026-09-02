#include <core/engine/target_info.hpp>

#include <core/engine/card.hpp>

namespace cg::engine
{
	auto TargetInfo::set_owner_target(Card& card) noexcept -> bool
	{
		return owner_targets_.insert(card);
	}

	auto TargetInfo::cancel_owner_target(const Card& card) noexcept -> bool
	{
		return owner_targets_.erase(card);
	}

	TargetInfo::TargetInfo() noexcept = default;
	// 	: card_targets_{},
	// 	  owner_targets_{}
	// {
	// 	//
	// }

	auto TargetInfo::set_target(Card& target, Card& owner) noexcept -> bool
	{
		if (card_targets_.insert(target))
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
		if (card_targets_.erase(target))
		{
			// 将本卡移出目标卡的"以本卡为对象的卡"列表中
			target.target_.cancel_owner_target(owner);

			return true;
		}

		return false;
	}

	auto TargetInfo::card_targets() const noexcept -> View
	{
		return View{card_targets_};
	}

	auto TargetInfo::owner_targets() const noexcept -> View
	{
		return View{owner_targets_};
	}

	auto TargetInfo::has_target() const noexcept -> bool
	{
		return !card_targets_.empty();
	}

	auto TargetInfo::has_target(const Card& card) const noexcept -> bool
	{
		return card_targets_.contains(card);
	}

	auto TargetInfo::target_count() const noexcept -> size_type
	{
		return static_cast<size_type>(card_targets_.size());
	}
}

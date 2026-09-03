#include <core/engine/equip_info.hpp>

#include <core/engine/card.hpp>

namespace cg::engine
{
	EquipInfo::EquipInfo() noexcept = default;
	// 	: equips_{},
	// 	  owner_{}
	// {
	// 	//
	// }

	auto EquipInfo::add_equip(Card& equip, Card& target) noexcept -> bool
	{
		// 检查是否已装备
		if (equip.equip_.owner() == &target)
		{
			return true;
		}

		// 如果已经是别的卡的装备,将其从原宿主移除
		if (const auto owner = equip.equip().owner();
			owner != nullptr)
		{
			remove_equip(equip, *owner);
		}
		else
		{
			// TODO: 移动装备卡位置?
		}

		// 将目标卡作为装备的所有者
		equip.equip_.owner_ = CardOptional{&target};
		// 将装备添加到装备列表
		target.equip_.equips_.insert(equip);

		// todo: 装备效果?

		return true;
	}

	auto EquipInfo::remove_equip(Card& equip, Card& target) noexcept -> bool
	{
		// 检查是否未装备
		if (equip.equip().owner() != &target)
		{
			return true;
		}

		// 将装备移除出装备列表
		if (const auto erased = target.equip_.equips_.erase(equip);
			!erased)
		{
			// 不存在该装备返回false
			return false;
		}
		// 设置装备的装备目标
		equip.equip_.owner_ = nullptr;

		// todo: 装备效果?

		return true;
	}

	auto EquipInfo::remove_equips(Card& target) noexcept -> void
	{
		auto& target_equips = target.equip_.equips_;

		// 清除所有装备卡的目标
		std::ranges::for_each(
			target_equips,
			[](const auto& equip) noexcept -> void
			{
				equip.get().equip_.owner_ = nullptr;
			}
		);

		// 移除所有装备卡
		target_equips.clear();
	}

	auto EquipInfo::can_equip(Card& card) noexcept -> bool
	{
		// 目标必须是:
		// 1.在场上
		// 2.表侧表示
		// 3.不是衍生物
		if (const auto target_state = card.state();
			!target_state.is_field_zone() or !target_state.is_face_up_form())
		{
			return false;
		}
		if (card.prototype().has_card_type({domain::CardType::TOKEN_MONSTER}))
		{
			return false;
		}

		// todo: 不能成为卡的目标效果

		return true;
	}

	auto EquipInfo::equips() const noexcept -> const Group&
	{
		return equips_;
	}

	auto EquipInfo::owner() const noexcept -> CardOptional
	{
		return owner_;
	}

	auto EquipInfo::has_equip() const noexcept -> bool
	{
		return !equips_.empty();
	}

	auto EquipInfo::has_equip(const Card& equip) const noexcept -> bool
	{
		return equips_.contains(equip);
	}

	auto EquipInfo::equip_count() const noexcept -> std::size_t
	{
		return equips_.size();
	}
}

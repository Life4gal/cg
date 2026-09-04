#include <core/engine/equip_info.hpp>

#include <core/engine/card.hpp>
#include <libassert/assert.hpp>

namespace cg::engine
{
	EquipInfo::EquipInfo(const CardReference owner) noexcept
		: owner{owner}
	// target{},
	// equips{}
	{}

	auto EquipInfo::do_equip(const CardReference equip) noexcept -> void
	{
		// 加入装备列表
		equips.insert(equip);
		// 设置装备卡目标
		equip.get().equip_.target = CardOptional{owner};
	}

	auto EquipInfo::do_unequip(const CardReference equip) noexcept -> void
	{
		// 从装备列表中移除
		equips.erase(equip);
		// 设置装备卡目标
		equip.get().equip_.target = nullptr;
	}

	auto EquipInfo::equip(const CardReference equip) noexcept -> void
	{
		// 本卡不能是装备卡
		ASSERT(target == nullptr);

		// 如果该装备卡已经装备了其他目标,移除目标
		if (auto& equip_equip = equip.get().equip_;
			equip_equip.target != nullptr)
		{
			equip_equip.do_unequip(equip);
		}

		// 将其加入本卡的装备列表
		do_equip(equip);
	}

	auto EquipInfo::unequip(const CardReference equip) noexcept -> void
	{
		// 本卡不能是装备卡
		ASSERT(target == nullptr);

		auto& equip_equip = equip.get().equip_;

		// 该装备卡没有装备目标?
		if (equip_equip.target == nullptr)
		{
			return;
		}

		// 如果该装备卡有装备目标,那一定是本卡
		ASSERT(equip_equip.target == owner);
		do_unequip(equip);
	}

	auto EquipInfo::unequip_all() noexcept -> void
	{
		// 本卡不能是装备卡
		ASSERT(target == nullptr);

		// 清除所有装备卡的目标
		std::ranges::for_each(
			equips,
			[](EquipInfo& ei) noexcept -> void
			{
				ei.target = nullptr;
			},
			[](const CardReference equip) noexcept -> EquipInfo&
			{
				return equip.get().equip_;
			}
		);

		// 移除所有装备卡
		equips.clear();
	}
}

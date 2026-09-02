#pragma once

#include <core/engine/collection.hpp>

namespace cg::engine
{
	class EquipInfo
	{
	public:
		//

	private:
		// 装备卡
		// 基于State::zone().position获取卡,容器无需基于插入顺序有序
		Group equips_;
		// 装备所属(如果当前卡是装备卡)
		CardOptional owner_;

	public:
		EquipInfo() noexcept;

		// 将一张卡作为装备卡装备到目标卡(返回是否装备成功)(如果已经是目标卡装备返回true)
		static auto add_equip(Card& equip, Card& target) noexcept -> bool;
		// 将一张作为目标卡装备的卡移除(返回是否移除成功)(如果不是目标卡装备返回true)
		static auto remove_equip(Card& equip, Card& target) noexcept -> bool;
		// 目标卡是否可以是装备卡
		[[nodiscard]] static auto can_equip(Card& card) noexcept -> bool;

		[[nodiscard]] auto equips() const noexcept -> View;
		[[nodiscard]] auto owner() const noexcept -> CardOptional;

		[[nodiscard]] auto has_equip() const noexcept -> bool;
		[[nodiscard]] auto has_equip(const Card& equip) const noexcept -> bool;
		[[nodiscard]] auto equip_count() const noexcept -> std::size_t;
	};
}

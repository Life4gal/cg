#pragma once

#include <core/engine/collection.hpp>

namespace cg::engine
{
	// 装备信息
	class EquipInfo
	{
	public:
		// 当前装备信息所属卡牌 -- 冗余设计,简化接口
		CardReference owner;

		// 当前装备的目标 -- 如果当前卡是装备卡
		CardOptional target;
		// 当前装备的装备卡牌 -- 当前必须不是装备卡,装备卡不能装备装备卡
		Group equips;

		explicit EquipInfo(CardReference owner) noexcept;

	private:
		auto do_equip(CardReference equip) noexcept -> void;
		auto do_unequip(CardReference equip) noexcept -> void;

	public:
		// 将一张卡作为装备卡装备到本卡
		auto equip(CardReference equip) noexcept -> void;
		// 将一张本卡的装备卡移除
		auto unequip(CardReference equip) noexcept -> void;
		// 将本卡的所有装备卡移除
		auto unequip_all() noexcept -> void;
	};
}

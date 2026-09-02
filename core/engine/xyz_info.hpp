#pragma once

#include <core/engine/collection.hpp>

namespace cg::engine
{
	class XyzInfo
	{
	public:
		using size_type = std::uint32_t;

	private:
		// 超量素材
		// 需要基于State::zone().index获取卡,所以容器必须基于插入顺序有序(不能使用Group)
		Sequence materials_;
		// 素材所属(如果当前卡是超量素材)
		CardOptional overlay_target_;

	public:
		XyzInfo() noexcept;

		// 将一张卡作为超量素材叠放到目标卡(返回是否叠放成功)(如果已经是目标卡超量素材返回true)
		static auto add_overlay(Card& material, Card& target) noexcept -> bool;
		// 将一张作为目标卡超量素材的卡移除(返回是否移除成功)(如果不是目标卡超量素材返回true)
		static auto remove_overlay(Card& material, Card& target) noexcept -> bool;

		[[nodiscard]] auto materials() const noexcept -> View;
		[[nodiscard]] auto overlay_target() const noexcept -> CardOptional;

		[[nodiscard]] auto has_material() const noexcept -> bool;
		[[nodiscard]] auto has_material(const Card& material) const noexcept -> bool;
		[[nodiscard]] auto material_count() const noexcept -> size_type;
	};
}

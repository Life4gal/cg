#include <core/engine/xyz_info.hpp>

#include <core/engine/card.hpp>

namespace cg::engine
{
	XyzInfo::XyzInfo() noexcept = default;
	// 	: materials_{},
	// 	  overlay_target_{}
	// {
	// 	//
	// }

	auto XyzInfo::add_overlay(Card& target, Card& material) noexcept -> bool
	{
		// 检查是否已叠放
		if (material.xyz_.overlay_target == &target)
		{
			return true;
		}

		// 如果已经是别的卡的超量素材,将其从原宿主移除
		if (const auto material_target = material.xyz().overlay_target();
			material_target != nullptr)
		{
			remove_overlay(material, *material_target);
		}
		else
		{
			// 如果不是别的卡的超量素材,将其从场上移除(作为超量素材附加到当前卡上)
			// todo: 将其从场上移除
		}

		// 目标卡当前已有素材数量作为新素材的序列位置
		const auto zone_index = target.xyz_.materials.size();

		// 将目标卡作为超量素材的目标
		material.xyz_.overlay_target = CardOptional{&target};
		// 将超量素材添加到超量素材列表
		target.xyz_.materials.push_back(material);

		// 设置超量素材状态
		auto material_state = material.state();
		// 控制者设置为目标卡的控制者
		material_state.set_controller(target.state().controller());
		// 位置设置为叠放区
		material_state.set_zone(domain::Zone::Overlay{.index = static_cast<domain::Zone::size_type>(zone_index)});

		// todo: 素材效果?

		return true;
	}

	auto XyzInfo::remove_overlay(Card& target, Card& material) noexcept -> bool
	{
		// 检查是否未叠放
		if (material.xyz().overlay_target() != &target)
		{
			return true;
		}

		// 将超量素材移除出超量素材列表
		if (const auto erased = target.xyz_.materials.erase(material);
			!erased)
		{
			// 不存在该素材返回false
			return false;
		}
		// 设置超量素材的叠放目标
		material.xyz_.overlay_target = nullptr;

		// todo: 设置超量素材状态
		// auto material_state = material.state();

		// 重置其他素材的序列
		for (auto begin = target.xyz_.materials.begin(), it = begin; it != target.xyz_.materials.end(); ++it)
		{
			const auto index = std::ranges::distance(begin, it);
			it->get().state().set_zone(domain::Zone::Overlay{.index = static_cast<domain::Zone::size_type>(index)});
		}

		// todo: 素材效果?

		return true;
	}

	auto XyzInfo::remove_overlays(Card& target) noexcept -> void
	{
		auto& target_materials = target.xyz_.materials;

		// 清除所有超量素材的叠放目标
		std::ranges::for_each(
			target_materials,
			[](const CardReference& material) noexcept -> void
			{
				material.get().xyz_.overlay_target = nullptr;
			}
		);

		// 移除所有超量素材
		target_materials.clear();
	}
}

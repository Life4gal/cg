#include <core/engine/xyz_info.hpp>

#include <core/engine/card.hpp>
#include <libassert/assert.hpp>

namespace cg::engine
{
	XyzInfo::XyzInfo(const CardReference owner) noexcept
		: owner{owner}
	// target{},
	// materials{}
	{}

	auto XyzInfo::do_take(const CardReference material) noexcept -> void
	{
		// 加入超量素材
		materials.push_back(material);
		// 设置叠放目标
		material.get().xyz_.target = CardOptional{owner};
	}

	auto XyzInfo::do_remove(const CardReference material) noexcept -> void
	{
		// 从超量素材中移除
		materials.erase(material);
		// 设置叠放目标
		material.get().xyz_.target = nullptr;
	}

	auto XyzInfo::take(const CardReference material) noexcept -> void
	{
		// 本卡不能是超量素材
		ASSERT(target == nullptr);

		// 如果该超量素材已经叠放在其他目标,移除目标
		if (auto& material_xyz = material.get().xyz_;
			material_xyz.target != nullptr)
		{
			material_xyz.do_remove(material);
		}

		// 将其加入本卡的超量素材
		do_take(material);
	}

	auto XyzInfo::remove(CardReference material) noexcept -> void
	{
		// 本卡不能是超量素材
		ASSERT(target == nullptr);

		auto& material_xyz = material.get().xyz_;

		// 该超量素材没有叠放目标?
		if (material_xyz.target == nullptr)
		{
			return;
		}

		// 如果该超量素材有叠放目标,那一定是本卡
		ASSERT(material_xyz.target == owner);
		do_remove(material);
	}

	auto XyzInfo::remove_all() noexcept -> void
	{
		// 本卡不能是超量素材
		ASSERT(target == nullptr);

		// 清除所有超量素材的目标
		std::ranges::for_each(
			materials,
			[](XyzInfo& xi) noexcept -> void
			{
				xi.target = nullptr;
			},
			[](const CardReference material) noexcept -> XyzInfo&
			{
				return material.get().xyz_;
			}
		);

		// 移除所有超量素材
		materials.clear();
	}
}

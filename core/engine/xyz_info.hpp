#pragma once

#include <core/engine/collection.hpp>

namespace cg::engine
{
	class XyzInfo
	{
	public:
		// 当前超量信息所属卡牌 -- 冗余设计,简化接口
		CardReference owner;

		// 当前叠放的目标 -- 如果当前卡是超量素材
		CardOptional target;
		// 当前的超量素材 -- 当前必须不是超量素材,超量素材不能有超量素材
		Sequence materials;

		explicit XyzInfo(CardReference owner) noexcept;

	private:
		auto do_take(CardReference material) noexcept -> void;
		auto do_remove(CardReference material) noexcept -> void;

	public:
		// 将一张卡作为超量素材叠放到本卡 -- 不处理目标卡的超量素材
		auto take(CardReference material) noexcept -> void;
		// 将一张作为本卡超量素材的卡移除
		auto remove(CardReference material) noexcept -> void;
		// 将本卡的所有的超量素材移除
		auto remove_all() noexcept -> void;
	};
}

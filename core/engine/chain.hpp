#pragma once

#include <core/domain/chain.hpp>
#include <core/domain/player.hpp>

#include <core/engine/card_reference.hpp>
#include <core/engine/effect_reference.hpp>

namespace cg::engine
{
	class ChainNode
	{
	public:
		domain::ChainId id;
		domain::ChainFlag flag;

		// 连锁的玩家
		domain::Player player;
		// 连锁的卡
		CardOptional card;
		// 连锁的效果
		EffectOptional effect;

		//

		ChainNode() noexcept;
	};

	class ChainLink
	{
	public:
		using link_type = std::vector<ChainNode>;

	private:
		link_type link_;

		//

	public:
		ChainLink() noexcept;
	};
}

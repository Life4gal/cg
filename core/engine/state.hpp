#pragma once

#include <core/domain/player.hpp>
#include <core/domain/zone.hpp>
#include <core/domain/reason.hpp>

#include <core/engine/card_reference.hpp>
#include <core/engine/effect_reference.hpp>

namespace cg::engine
{
	class State
	{
	public:
		// 当前控制者
		domain::Player controller;
		// 当前所在区域
		domain::Zone zone;

		// 发生移动/状态变化的原因玩家
		domain::Player reason_player;
		// 发生移动/状态变化的原因卡
		CardOptional reason_card;
		// 发生移动/状态变化的原因效果
		EffectOptional reason_effect;
		// 发生移动/状态变化的原因
		domain::ReasonWrapper reason;

		State() noexcept;
	};
}

#pragma once

#include <core/domain/player.hpp>
#include <core/domain/zone.hpp>
#include <core/domain/reason.hpp>

#include <core/engine/card_reference.hpp>
#include <core/engine/effect_reference.hpp>

namespace cg::engine
{
	class CardState
	{
	public:
		// 当前控制者
		domain::Player controller;
		// 当前所在区域
		domain::Zone zone;
		// 如果不在场上(例如卡组/额外卡组/超量素材/墓地/除外区),该卡所处下标
		domain::zone_sequence_type zone_index;
		// 如果在场上,所处位置
		domain::FieldZoneSequence field_zone;
		// 当前表示形式(仅在场上时有意义)
		domain::FieldZoneForm form;

		// 发生移动/状态变化的原因玩家
		domain::Player reason_player;
		// 发生移动/状态变化的原因卡
		CardOptional reason_card;
		// 发生移动/状态变化的原因效果
		EffectOptional reason_effect;
		// 发生移动/状态变化的原因
		domain::ReasonWrapper reason;
	};
}

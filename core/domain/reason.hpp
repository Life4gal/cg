#pragma once

#include <core/utility/enum.hpp>

namespace cg::domain
{
	// 卡牌移动/状态改变的原因
	enum class Reason : std::uint32_t
	{
		// ==== 召唤 ====

		// 通常召唤(通常召唤、上级召唤、盖放)
		NORMAL_SUMMON = 1 << 0,
		// 特殊召唤(通过效果或规则进行的特殊召唤,但不包括额外卡组的特定召唤)
		SPECIAL_SUMMON = 1 << 1,
		// 召唤被无效(召唤被"神之宣告"之类的卡牌无效)
		DISABLE_SUMMON = 1 << 2,

		// 融合召唤
		SUMMON_FUSION = 1 << 3,
		// 仪式召唤
		SUMMON_RITUAL = 1 << 4,
		// 同调召唤
		SUMMON_SYNCHRO = 1 << 5,
		// 超量召唤
		SUMMON_XYZ = 1 << 6,
		// 灵摆召唤
		SUMMON_PENDULUM = 1 << 7,
		// 连接召唤
		SUMMON_LINK = 1 << 8,

		// ==== 卡牌进入墓地 ====

		// 被破坏
		DESTROY = 1 << 9,
		// 被解放
		RELEASE = 1 << 10,
		// 被战斗破坏
		BATTLE = 1 << 11,
		// 被效果
		EFFECT = 1 << 12,
		// 被COST
		COST = 1 << 13,
		// 被丢弃
		DISCARD = 1 << 14,
		// 被作为(召唤)素材
		MATERIAL = 1 << 15,
		// 被作为超量素材移除
		REMOVE_OVERLAY = 1 << 16,

		// ==== 卡牌位置移动 ====

		// 返回手牌/卡组(如"强制脱出装置")
		RETURN = 1 << 17,
		// 抽卡
		DRAW = 1 << 18,
		// 公开(从卡组检索卡后公开)
		REVEAL = 1 << 19,
		// 效果转移
		REDIRECT = 1 << 20,

		// ==== 生命值变化 ====

		// 被效果伤害
		EFFECT_DAMAGE = 1 << 21,
		// 被效果恢复
		EFFECT_RECOVER = 1 << 22,

		// ==== 规则·处理相关 ====

		// 规则(如手牌超过6张时弃牌)
		RULE = 1 << 23,
		// 游戏调整或者重置
		ADJUST = 1 << 24,
		// 替代(尤贝尔)
		REPLACE = 1 << 25,
		// 临时除外(宇宙耀变)
		TEMPORARY = 1 << 26,
		// 失去对象
		LOST_TARGET = 1 << 27,
		// 维持代价(契约书)
		MAINTENANCE = 1 << 28,

		// ==== 特殊操作 ====

		// 反转(怪兽由里侧变为表侧)
		FLIP = 1 << 29,
		// 触发动作(如攻击宣言)
		ACTION = 1 << 30,
	};

	class ReasonWrapper : public utility::Enum<
				Reason,
				// &, &=, |, |=, ^, ^=, ~
				utility::EnumSupport::BIT_OPERATION
			> {};
}

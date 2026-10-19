#pragma once

#include <cstdint>

namespace cg::domain
{
	// ================================================================
	// 事件码
	// - 「期间」触发不在此列(用 Phase / BattleStep / DamageStep
	// - 事件载荷(如「召唤成功的卡」「伤害数值」)经查询原语读取
	// ================================================================
	enum class EventCode : std::uint16_t
	{
		// ========================================
		// 召唤 / 特殊召唤(宣言 → 之际 → 成功;无效窗口)
		// ========================================

		// 通常召唤宣言
		NORMAL_SUMMON_DECLARED,
		// 通常召唤被无效
		NORMAL_SUMMON_NEGATED,
		// 通常召唤成功
		NORMAL_SUMMON_SUCCESS,

		// 特殊召唤宣言
		SPECIAL_SUMMON_DECLARED,
		// 特殊召唤被无效
		SPECIAL_SUMMON_NEGATED,
		// 特殊召唤成功
		SPECIAL_SUMMON_SUCCESS,

		// 翻转召唤宣言
		FLIP_SUMMON_DECLARED,
		// 翻转召唤被无效
		FLIP_SUMMON_NEGATED,
		// 翻转召唤成功
		FLIP_SUMMON_SUCCESS,

		// 翻转(被翻面;Flip效果触发)
		FLIP,

		// ========================================
		// 战斗
		// ========================================

		// 攻击宣言
		ATTACK_DECLARED,
		// 被战斗破坏
		BATTLE_DESTROYED,
		// 战斗伤害
		BATTLE_DAMAGE,

		// ========================================
		// 破坏 / 移动
		// ========================================

		// 破坏(通用)
		DESTROYED,
		// 被效果破坏
		EFFECT_DESTROYED,
		// 送去墓地
		SENT_TO_GRAVEYARD,
		// 除外
		BANISHED,
		// 返回手牌
		RETURNED_TO_HAND,
		// 返回卡组
		RETURNED_TO_DECK,
		// 加入额外卡组
		SENT_TO_EXTRA_DECK,
		// 离开场上
		LEFT_FIELD,
		// 装备
		EQUIPPED,

		// ========================================
		// 动作
		// ========================================

		// 抽卡
		CARD_DRAWN,
		// 怪兽盖放
		MONSTER_SET,
		// 魔陷盖放
		SPELL_TRAP_SET,
		// 表示形式变更
		POSITION_CHANGED,
		// 解放(解放送墓 ≠ 破坏)
		RELEASED,
		// 舍弃
		DISCARDED,

		// ========================================
		// 数值
		// ========================================

		// 效果伤害
		EFFECT_DAMAGE,
		// 恢复生命值
		RECOVERED,

		// ========================================
		// 连锁
		// ========================================

		// 效果发动(响应窗口)
		CHAINING,
		// 连锁结算完成
		CHAIN_SOLVED,

		// ========================================
		// 阶段
		// ========================================

		// 阶段开始(具体阶段期间用Phase/BattleStep/DamageStep)
		PHASE_STARTED,
	};
}

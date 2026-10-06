#pragma once

#include <cstdint>

namespace cg::domain
{
	enum class EventCode : std::uint8_t
	{
		// 自由时点
		FREE_CHAIN,

		// ========================================
		// 连锁
		// ========================================

		// 连锁正在累计的响应窗口
		CHAINING,
		// 连锁正在解析的响应窗口
		CHAIN_RESOLVING,
		// 连锁解析完毕的响应窗口
		CHAIN_RESOLVED,
		// 连锁结束
		CHAIN_END,

		// 连锁链上某个效果被无效
		CHAIN_NEGATED,

		// ========================================
		// 阶段
		// ========================================

		// 任意阶段开始
		PHASE_START,
		// 任意阶段结束
		PHASE_END,

		// 抽卡阶段
		DRAW_PHASE,
		// 准备阶段
		STANDBY_PHASE,
		// 主要阶段1
		MAIN_PHASE_1,
		// 战斗阶段
		BATTLE_PHASE,
		// 主要阶段2
		MAIN_PHASE_2,
		// 结束阶段
		END_PHASE,

		// ========================================
		// 召唤
		// ========================================

		// 通常召唤成功
		NORMAL_SUMMON_SUCCESS,
		// 特殊召唤成功
		SPECIAL_SUMMON_SUCCESS,
		// 反转召唤成功
		FLIP_SUMMON_SUCCESS,

		// ========================================
		// 战斗
		// ========================================

		// 攻击宣言
		ATTACK_DECLARED,
		// 成为攻击目标
		BECOME_ATTACK_TARGET,
		// 伤害步骤开始
		DAMAGE_STEP_BEGIN,
		// 伤害计算前
		BEFORE_DAMAGE_CALCULATE,
		// 伤害计算时
		DAMAGE_CALCULATING,
		// 伤害计算后
		AFTER_DAMAGE_CALCULATE,
		// 伤害步骤结束
		DAMAGE_STEP_END,
		// 战斗破坏中
		BATTLE_DESTROYING,
		// 战斗结束
		BATTLED,

		// ========================================
		// 移动
		// ========================================

		// 被破坏(含战斗和效果)
		DESTROY,
		// 被战斗破坏
		DESTROY_BY_BATTLE,
		// 增加移除超量素材
		CHANGE_OVERLAY,

		// ========================================
		// 移动
		// ========================================

		// 卡被加入卡组
		TO_DECK,
		// 卡被加入手牌
		TO_HAND,
		// 卡被加入墓地
		TO_GRAVEYARD,
		// 卡被加入除外区
		TO_BANISHED,

		// 卡预备离开场地(可替换目的地)
		BEFORE_LEAVE_FIELD,
		// 卡离开场地
		LEAVE_FIELD,
		// 卡离开墓地
		LEAVE_GRAVEYARD,
		// 卡离开除外区
		LEAVE_BANISHED,
	};

	// 卡牌移动/状态变化的原因
	enum class Reason : std::uint32_t
	{
		// 因规则导致
		RULE = 0,
		// 因抽卡导致
		DRAW = 1 << 1,
		// 因COST导致
		COST = 1 << 2,
		// 因舍弃导致
		DISCARD = 1 << 3,
		// 因效果导致
		EFFECT = 1 << 4,
		// 因战斗导致
		BATTLE = 1 << 5,
		// 因破坏导致
		DESTROY = 1 << 6,
		// 因解放导致
		RELEASE = 1 << 7,
		// 因失去对象(如装备卡的装备对象离场)
		LOST_TARGET = 1 << 8,
		// 作为超量素材被取除导致
		REMOVE_OVERLAY = 1 << 9,
		// 代替破坏导致
		REPLACE = 1 << 10,
		// 因召唤手续导致
		SUMMON = 1 << 11,
		// 因作为仪式素材导致
		RITUAL = 1 << 12,
		// 因作为融合素材导致
		FUSION = 1 << 13,
		// 因作为同调素材导致
		SYNCHRO = 1 << 14,
		// 因作为超量素材导致
		XYZ = 1 << 15,
		// 因作为连接素材导致
		LINK = 1 << 16,
	};

	// 时点
	enum class Timing : std::uint8_t
	{
		// ========================================
		// 阶段
		// ========================================

		// 抽卡阶段开始
		DRAW_PHASE,
		// 准备阶段开始
		STANDBY_PHASE,
		// 主要阶段1开始
		MAIN_PHASE_1,
		// 宣言进入战斗阶段
		BATTLE_PHASE_START,
		// 战斗阶段开始
		BATTLE_PHASE,
		// 战斗阶段结束
		BATTLE_PHASE_END,
		// 主要阶段2开始
		MAIN_PHASE_2,
		// 结束阶段开始
		END_PHASE,

		// ========================================
		// 召唤
		// ========================================

		// 通常召唤成功
		NORMAL_SUMMON_SUCCESS,
		// 特殊召唤成功
		SPECIAL_SUMMON_SUCCESS,
		// 反转召唤成功
		FLIP_SUMMON_SUCCESS,

		// ========================================
		// 战斗
		// ========================================

		// 攻击宣言
		ATTACK_DECLARED,
		// 进入伤害步骤
		DAMAGE_STEP_BEGIN,
		// 伤害计算前
		BEFORE_DAMAGE_CALCULATE,
		// 伤害计算后
		AFTER_DAMAGE_CALCULATE,
		// 伤害步骤结束
		DAMAGE_STEP_END,

		// ========================================
		// 行为
		// ========================================

		// 抽卡(非抽卡阶段)
		DRAW,
		// 盖放怪兽
		SET_MONSTER,
		// 盖放魔陷
		SET_SPELL_TRAP,
		// 改变表示形式
		FORM_CHANGE,

		// ========================================
		// 事件
		// ========================================

		// 造成伤害(减少生命值)
		DAMAGE,
		// 恢复生命值
		RECOVER,

		// 卡被破坏
		DESTROY,
		// 卡被装备
		EQUIP,

		// 卡被加入卡组
		TO_DECK,
		// 卡被加入手牌
		TO_HAND,
		// 卡被加入墓地
		TO_GRAVEYARD,
		// 卡被除外
		TO_BANISH,
	};
}

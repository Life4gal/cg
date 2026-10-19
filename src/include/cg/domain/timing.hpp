#pragma once

#include <cstdint>

namespace cg::domain
{
	// 时点
	//
	// - *_DECLARED      : 宣言/尝试执行某动作时(尚未结算)
	// - *_SUCCESS       : 动作成功结算后
	// - *_NEGATED       : 动作被无效
	// - BEFORE_*        : 某结算发生之前
	// - AFTER_*         : 某结算发生之后
	// - *_START / *_END : 某区间的开始/结束时
	//
	// - 「期间」(阶段 / 战斗步骤 / 伤害步骤)   → Phase / BattleStep / DamageStep
	// - 「事件」(发生了什么)                   → EventCode
	// - 「窗口」(谁获得优先权 / 可发动快效果)  → 快效果时点系统(TODO)
	//
	enum class [[deprecated("仅作阐述,不应该被使用")]] Timing : std::uint8_t
	{
		// ========================================
		// 阶段
		//
		// 阶段结束宣言动作会打开一个时点窗口,非回合玩家可在此发动快速效果(如原始生命态·尼比鲁[27204311])
		// 若对手响应后连锁处理完毕,游戏状态会回到原阶段,回合玩家可继续操作,也可再次宣言结束
		// ========================================

		// 抽卡阶段开始
		DRAW_PHASE_START,
		// 准备阶段开始
		STANDBY_PHASE_START,
		// 主要阶段1开始
		MAIN_PHASE_1_START,
		// 主要阶段1结束宣言
		// 回合玩家宣言结束主要阶段1,非回合玩家获得优先权
		MAIN_PHASE_1_END_DECLARED,
		// 战斗阶段开始
		BATTLE_PHASE_START,
		// 战斗阶段结束宣言
		// 回合玩家宣言结束战斗阶段,非回合玩家获得优先权
		BATTLE_PHASE_END_DECLARED,
		// 主要阶段2开始
		MAIN_PHASE_2_START,
		// 主要阶段2结束宣言
		// 回合玩家宣言结束主要阶段2,非回合玩家获得优先权
		MAIN_PHASE_2_END_DECLARED,
		// 结束阶段开始
		END_PHASE_START,

		// ========================================
		// 召唤
		//
		// 召唤流程分为「宣言 → 之际(时) → 成功」三个节点
		// 「之际(时)」是可发动无效召唤类效果(如神之宣告[41420027])的时点;此时卡牌放置到场上,但是怪兽不算登场
		// 「成功」是可发动召唤成功类效果(如激流葬[53582587])的时点,此时怪兽已登场
		//
		// 反转召唤本质是表示形式变更,但规则上可对应「之际(时)」发动神之宣告等无效召唤的效果
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

		// 反转召唤宣言
		FLIP_SUMMON_DECLARED,
		// 反转召唤被无效
		FLIP_SUMMON_NEGATED,
		// 反转召唤成功
		FLIP_SUMMON_SUCCESS,

		// ========================================
		// 战斗
		//
		// 攻击宣言后进入伤害步骤;伤害步骤内部为五时点:
		// 开始 → 计算前 → 计算时 → 计算后 → 结束时
		// 「计算时」只能发动影响攻守数值的效果(如欧尼斯特[37742478])
		// 「计算后」可发动战斗诱发的效果(如异次元的女战士[07572887])
		// 「结束时」可发动送墓/破坏时效果
		// ========================================

		// 攻击宣言
		ATTACK_DECLARED,
		// 伤害步骤开始时
		DAMAGE_STEP_START,
		// 伤害计算前(里侧怪兽在此翻转)
		BEFORE_DAMAGE_CALCULATE,
		// 伤害计算时(比较攻守、确定破坏,只能发动影响攻守的效果)
		PERFORM_DAMAGE_CALCULATE,
		// 伤害计算后(战斗诱发的效果)
		AFTER_DAMAGE_CALCULATE,
		// 伤害步骤结束时(送墓、破坏时效果)
		DAMAGE_STEP_END,
		// 直接攻击
		DIRECT_ATTACK,

		// ========================================
		// 行为
		//
		// 玩家主动执行的动作所产生的时点
		// 注意区别于"事件"——事件是动作结算后的结果
		// 行为时点通常发生在动作宣言或结算的瞬间
		// ========================================

		// 抽卡（非抽卡阶段的抽卡，如效果抽卡）
		DRAW,
		// 怪兽被盖放
		MONSTER_SET,
		// 魔陷被盖放
		SPELL_TRAP_SET,
		// 表示形式被改变
		POSITION_CHANGED,
		// 卡被解放(祭品)
		RELEASE,
		// 卡被舍弃(从手牌丢弃)
		DISCARD,

		// ========================================
		// 事件
		//
		// 游戏状态发生变化后的时点,通常用于发动"XX时"类效果
		// 注意区分伤害来源和破坏来源
		// ========================================

		// 效果伤害
		EFFECT_DAMAGE,
		// 战斗伤害
		BATTLE_DAMAGE,
		// 恢复生命值
		RECOVER,

		// 通用破坏(通用,不区分来源)
		DESTROY,
		// 被战斗破坏
		BATTLE_DESTROY,
		// 被效果破坏
		EFFECT_DESTROY,

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
		// 卡返回额外卡组
		TO_EXTRA_DECK,

		// 卡从场上/墓地等返回手牌
		RETURN_TO_HAND,
		// 卡返回卡组
		RETURN_TO_DECK,
	};
}

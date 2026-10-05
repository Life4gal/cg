#pragma once

#include <cstdint>

namespace cg::domain
{
	// 效果类型
	enum class EffectKind : std::uint8_t
	{
		// 发动 -- 1速魔法陷阱卡
		ACTIVATE,
		// 启动 -- 1速怪兽卡
		IGNITION,
		// 诱发 -- 触发条件满足时可发动
		TRIGGER,
		// 速攻/诱发即时 -- 任意响应窗口可发动
		QUICK,
		// 反转 -- TRIGGER+FLIP-EVENT(影依兽[3717252])
		FLIP,
		// 静态 -- 不入连锁,持续适用,不产生事件(群雄割据[90846359])
		STATIC,
		// 持续 -- 不入连锁,满足条件执行,产生事件(我我我枪手[12014404])
		CONTINUOUS,
	};

	// 咒文速度
	enum class EffectSpeed : std::uint8_t
	{
		// 无速度,不入连锁(如STATIC效果)
		NONE = 0,
		NORMAL = 1,
		QUICK = 2,
		COUNTER = 3,
	};

	// 限定静态效果适用时卡牌所在区域(只有效果所属卡牌位于指定区域才有效果)
	enum class StaticEffectRegion : std::uint8_t
	{
		// 不限定区域,任何区域都适用
		ALL,
		// 怪兽区域
		MONSTER,
		// 灵摆区域
		PENDULUM,
		// 无所谓什么区域,只要在场上就行
		FIELD,
	};

	// 限定静态效果适用玩家
	enum class StaticEffectTargetRange : std::uint8_t
	{
		// 只波及自己
		SELF,
		// 只波及对手
		OPPONENT,
		// 双方都波及
		ALL,
	};

	// 限定静态效果适用生命周期
	enum class StaticEffectLifetime : std::uint8_t
	{
		// 永久持续(只有效果还适用)
		PERMANENT,
		// 到本回合结束时失效
		END_TURN,
		// 到本阶段结束时失效
		END_PHASE,
		// 未装备时失效
		UNEQUIPPED,
		// 不在场上时失效
		NOT_ON_FIELD,
	};

	// 效果提供的抗性
	enum class Protection : std::uint8_t
	{
		// 不被效果破坏
		DESTROY = 1 << 0,
		// 不被战斗破坏
		BATTLE = 1 << 1,
		// 不受效果影响
		UNAFFECTED_BY_EFFECTS = 1 << 2,
		// 不能成为效果的对象
		TARGET = 1 << 3,
		// 不受其他卡效果影响
		IMMUNE_TO_OTHER_EFFECTS = 1 << 4,
	};

	// 效果提供的破坏替代
	enum class DestroyReplacement : std::uint8_t
	{
		// 移除一个超量素材代替破坏
		REMOVE_OVERLAY,
		// 墓地除外代替破坏
		BANISH_FROM_GRAVEYARD,
		// 破坏装备卡代替破坏
		DESTROY_EQUIP,
		// 破坏装备卡代替破坏,仅限战斗破坏
		DESTROY_EQUIP_BATTLE_ONLY,
	};

	// 效果产生的限制效果
	enum class Restriction : std::uint8_t
	{
		// ========================================
		// 召唤
		// ========================================

		// 不能通常召唤
		NORMAL_SUMMON,
		// 不能特殊召唤
		SPECIAL_SUMMON,
		// 不能反转召唤
		FLIP_SUMMON,

		// ========================================
		// 效果
		// ========================================

		// 不能发动效果
		ACTIVATE,
		// 不能触发效果
		TRIGGER,

		// ========================================
		// 行为
		// ========================================

		// 不能抽卡
		DRAW,
		// 不能进行连锁
		CHAIN,
		// 不能盖放怪兽
		SET_MONSTER,
		// 不能盖放魔陷
		SET_SPELL_TRAP,
		// 不能攻击
		ATTACK,
		// 不能直接攻击
		DIRECT_ATTACK,
		// 不能进入战斗阶段
		BATTLE_PHASE,
		// 不能改变表示形式
		FORM_CHANGE,
		// 不能改变控制权
		CONTROL_CHANGE,
		// 将卡组的卡牌送入墓地
		DISCARD_DECK_TO_GRAVEYARD,

		// ========================================
		// 移动
		// ========================================

		// 不能加入手牌
		TO_HAND,
		// 不能加入墓地
		TO_GRAVEYARD,
		// 不能加入除外区
		TO_BANISHED,
	};

	// 效果类别
	enum class EffectCategory : std::uint8_t
	{
		// 抽卡
		DRAW,
		// 卡组检索
		SEARCH,
		// 加入卡组
		TO_DECK,
		// 加入额外卡组
		TO_EXTRA_DECK,
		// 加入手牌
		TO_HAND,
		// 加入墓地
		TO_GRAVEYARD,
		// 加入除外区
		TO_BANISHED,
		// 舍弃手牌
		DISCARD,
		// 破坏
		DESTROY,
		// 改变表示形式
		FORM_CHANGE,
		// 改变控制权
		CONTROL_CHANGE,
		// 特殊召唤
		SPECIAL_SUMMON,
		// 解放怪兽
		RELEASE,
		// 无效发动/无效效果
		NEGATE,
		// 禁用效果
		DISABLE,
		// 宣言(卡名/种族/属性等)
		// 装备
		EQUIP,
		// 生成衍生物
		TOKEN,
		// 放置指示物
		COUNTER,
		// 恢复生命值
		RECOVER,
		// 造成伤害(减少生命值)
		DAMAGE,
	};

	// 效果附加属性/标志
	enum class EffectProperty : std::uint16_t
	{
		// 可以在伤害步骤发动(欧尼斯特[37742478])
		ACTIVATE_IN_DAMAGE_STEP = 1 << 0,
		// 可以在伤害计算时发动(星尘龙[44508094])
		ACTIVATE_IN_DAMAGE_CALCULATION = 1 << 1,
		// 以卡牌为对象(死者苏生[83764718])
		TARGET_CARD = 1 << 2,
		// 以玩家为对象(冥王结界波[54693926] -- 受到的伤害变为0)
		TARGET_PLAYER = 1 << 3,
		// 延时处理(加速同调星尘龙[30983281])
		DELAYED_TRIGGER = 1 << 4,
		// 无法被无效(如规则自肃)
		CANNOT_BE_DISABLED = 1 << 5,
		// 仅在效果源所在区域适用(召唤僧[423585] -- `只要这张卡在怪兽区存在,这张卡就不能解放`仅在处于怪兽区有效)
		SOURCE_ZONE_ONLY = 1 << 6,
		// 作为超量素材时适用(`拥有这张卡作为素材的怪兽获得以下效果`)
		ACTIVATE_AS_OVERLAY = 1 << 7,
		// 免疫穿透(禁止令[43711255])
		IGNORE_IMMUNITY = 1 << 8,
		// 揭示未知卡(三战之才[25311006])
		REVEAL = 1 << 9,
	};
}

#pragma once

#include <functional>

#include <core/event.hpp>

namespace cg::core
{
	class Duel;
	class Card;
	class View;
	class Effect;

	// 效果上下文
	class EffectContext
	{
	public:
		// 触发此效果的玩家
		Player player;
		// 触发此效果的原因
		Reason reason;
		// 触发此效果的事件类型
		EventType event_type;

		// 触发此效果的卡片(如被破坏的怪兽)
		std::reference_wrapper<const Card> trigger_card;
		// 效果涉及的全部卡片(如被同时破坏的多张卡)
		std::reference_wrapper<const View> cards;
		// 引发此效果的原因效果(所属效果链)
		std::reference_wrapper<const Effect> reason_effect;

		//
	};

	// 效果类型
	enum class EffectCategory : std::uint64_t
	{
		// 0~7

		// 通常召唤
		SUMMON = 1 << 0,
		// 特殊召唤
		SPECIAL_SUMMON = 1 << 1,
		// 生成衍生物
		TOKEN = 1 << 2,

		// 8~15

		// 变更表示形式
		CHANGE_FORM = 1 << 8,
		// 变更控制权
		CHANGE_CONTROL = 1 << 9,
		// 变更攻击力
		CHANGE_ATTACK = 1 << 10,
		// 变更守备力
		CHANGE_DEFENSE = 1 << 11,

		// 16~23

		// 卡组抽卡
		DRAW = 1 << 7,
		// 卡组检索
		SEARCH = 1 << 8,
		// 卡组破坏
		DECK_DESTROY = 1 << 9,
		// 手牌破坏
		HAND_DESTROY = 1 << 10,

		// 24~31

		// 破坏
		DESTROY = 1 << 24,
		// 解放
		RELEASE = 1 << 25,
		// 除外
		REMOVE = 1 << 26,

		// 32~39

		// 返回手牌
		TO_HAND = 1 << 32,
		// 返回卡组
		TO_DECK = 1 << 33,
		// 送去墓地
		TO_GRAVE = 1 << 34,

		// 40~47

		// 效果伤害
		DAMAGE = 1 << 40,
		// 生命回复
		RECOVER = 1 << 41,

		// 48~55

		// 指示物
		COUNTER = 1 << 48,
		// 硬币
		COIN = 1 << 49,
		// 骰子
		DICE = 1 << 50,

		// 56~63
	};

	// 效果代码
	enum class EffectCode : std::uint32_t
	{
		// 不受影响
		IMMUNE_EFFECT,
		// 无效
		DISABLE,
		// 无法被无效
		CANNOT_DISABLE,
		// 设置所有者
		SET_CONTROL,
		// 无法改变所有者
		CANNOT_CHANGE_CONTROL,
		// 无法发动
		CANNOT_ACTIVATE,
		// 无法诱发
		CANNOT_TRIGGER,
		// 无法生效
		DISABLE_EFFECT,
		// 无法连锁
		DISABLE_CHAIN,

		// TODO
	};

	// 效果类型
	enum class EffectType : std::uint8_t
	{
		// 启动效果
		IGNITION = 1 << 0,
		// 发动效果
		ACTIVATE = 1 << 1,
		// 诱发效果
		TRIGGER = 1 << 2,
		// 诱发即时效果
		QUICK = 1 << 3,
		// 永续效果
		CONTINUOUS = 1 << 4,
		// 规则效果(如光之创造神)
		RULE = 1 << 5,
	};

	// 效果标志
	enum class EffectFlag : std::uint8_t
	{
		// 选发
		OPTIONAL = 1 << 0,
		// 必发
		FORCE = 1 << 1,
		// 取对象
		TARGET = 1 << 2,
		// 翻转
		FLIP = 1 << 3,
		// 超量素材时适用
		XYZ_MATERIAL = 1 << 4,
		// 赋予效果
		GRANT = 1 << 5,
	};

	// Y-龙头[code:65622692]:
	// -①：1回合1次，可以从以下的效果中选择1个发动。●以自己场上的1只「X－ヘッド・キャノン／X-头部加农炮」为对象，将这张卡视为装备卡装备给那只怪兽。装备怪兽要被战斗·效果破坏的场合，将这张卡破坏作为代替。●将装备状态的这张卡特殊召唤。
	// = EffectType::IGNITION + EffectFlag::TARGET | EffectFlag::GRANT
	//
	// 死者苏生[code:83764718]:
	// -①：可以以自己或对手墓地的1只怪兽为对象发动。将那只怪兽特殊召唤到自己的场上。
	// = EffectType::ACTIVATE + EffectFlag::TARGET
	//
	// 影依龙[code:77723643]:
	// -②：这张卡被效果送入墓地的场合，可以以场上的1张魔法陷阱卡为对象发动。将那张卡破坏。
	// = EffectType::TRIGGER + EffectFlag::OPTIONAL | EffectFlag::TARGET
	//
	// 三眼怪[code:26202165]:
	// -①：这张卡从场上送入墓地的场合必定发动。从卡组将1只攻击力1500以下的怪兽加入手牌。这个回合，自己不能发动用这个效果加入手牌的卡以及其同名卡的效果。
	// = EffectType::TRIGGER + EffectFlag::FORCE
	// 神圣魔术师:[code:31560081]:
	// -①：这张卡翻转的场合，必定以自己墓地的1张魔法卡为对象发动。将那张卡加入手牌。
	// = EffectType::TRIGGER + EffectFlag::FORCE | EffectFlag::TARGET | EffectFlag::FLIP
	//
	// 效果遮蒙者[code:97268402]:
	// -①：对手的主要阶段，可以将这张卡从手牌送入墓地，以对手场上的1只效果怪兽为对象发动。那只对手怪兽的效果直到回合结束时为止无效。
	// = EffectType::QUICK + EffectFlag::OPTIONAL | EffectFlag::TARGET
	// No.39 希望皇 霍普[code:84013237]:
	// -①：自己或对手的怪兽的攻击宣言时，可以将这张卡的1个多维素材取除发动。那次攻击无效。
	// = EffectType::QUICK + EffectFlag::OPTIONAL | EffectFlag::XYZ_MATERIAL
	// 
	// No.23 冥界的灵骑士 兰斯洛特[code:66547759]:
	// -③：1回合1次，这张卡以外的怪兽的效果或魔法陷阱卡发动时，必定将这张卡的1个多维素材取除发动。使其发动无效。
	// = EffectType::QUICK + EffectFlag::FORCE
	//
	// 人造人-念力震慑者[code:77585513]:
	// -①：只要这张卡在怪兽区存在，就不能发动双方场上的陷阱卡的效果且场上的陷阱卡的效果无效化。
	// = EffectType::CONTINUOUS

	// 效果
	class Effect
	{
	public:
		using process_order_type = std::uint32_t;
		using size_type = std::uint8_t;

		// 条件: 检查是否可以发动
		using condition_callback_type = std::function<bool(const EffectContext&)>;
		// 代价: 支付cost
		using cost_callback_type = std::function<void(const EffectContext&)>;
		// 目标: 选择对象
		using target_callback_type = std::function<bool(const EffectContext&)>;
		// 操作: 处理效果
		using operation_callback_type = std::function<void(const EffectContext&)>;

		// -- 所属决斗 -- 
		std::reference_wrapper<Duel> duel;

		// -- 所有者 -- 

		// 效果所属玩家
		Player player;
		// 效果拥有者
		std::reference_wrapper<Card> owner;
		// 效果发动/处理者
		std::reference_wrapper<Card> handler;

		// -- 效果 -- 

		// 效果代码
		EffectCode code;
		// 效果处理顺序,小的顺序先处理
		process_order_type process_order;

		// -- 生效位置 -- 

		// 
		Location location;
		// 


		// -- 发动次数限制 -- 

		// 最大发动次数
		size_type max_limit;
		// 剩余发动次数
		size_type limit;

		// -- 回调 -- 

		condition_callback_type condition_callback;
		cost_callback_type cost_callback;
		target_callback_type target_callback;
		operation_callback_type operation_callback;
	};
}

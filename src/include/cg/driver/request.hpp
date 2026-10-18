#pragma once

#include <cg/utility/enum.hpp>

#include <cg/domain/player.hpp>
#include <cg/domain/phase.hpp>
#include <cg/domain/field.hpp>
#include <cg/domain/summon.hpp>
#include <cg/domain/selection_context.hpp>

#include <cg/state/id.hpp>

#include <cg/driver/action.hpp>

namespace cg::driver::request
{
	// =================================================================
	// 行动阶段
	// =================================================================

	// 查询可用动作
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class QueryIdleCommand
	{
	public:
		domain::Player player;
	};

	// 提交动作
	// JSON:
	// {
	//     "player": "FIRST",
	//     "action": {
	//         "card": 1000,
	//         "placement": {
	//             "column": 0,
	//             "position": "FACE_UP_ATTACK"
	//         },
	//         "variant_index": 1,
	//         "type": "normal_summon"
	//     }
	// }
	class SubmitAction
	{
	public:
		domain::Player player;
		action::Action action;
	};

	// =================================================================
	// 通用选择
	// =================================================================

	// // 选择了一张卡牌
	// class SelectCard
	// {
	// public:
	// 	domain::Player player;
	// 	state::CardInstanceId card;
	// };

	// 从候选集中选择[min, max]张卡牌
	// JSON:
	// {
	//     "player": "FIRST",
	//     "candidates": [
	//         1000,
	//         1001
	//     ],
	//     "min": 1,
	//     "max": 2
	// }
	class SelectCards
	{
	public:
		domain::Player player;
		std::vector<state::CardInstanceId> candidates;
		domain::DeckLocation::size_type min;
		domain::DeckLocation::size_type max;
	};

	// 是/否选择
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class SelectYesNo
	{
	public:
		domain::Player player;
	};

	// 选择一个数值
	// JSON:
	// {
	//     "player": "FIRST",
	//     "candidates": [
	//         1,
	//         2,
	//         3
	//     ]
	// }
	class SelectNumbers
	{
	public:
		domain::Player player;
		std::vector<std::int64_t> candidates;
	};

	// 对指定卡牌进行排序
	// JSON:
	// {
	//     "player": "FIRST",
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class OrderCards
	{
	public:
		domain::Player player;
		std::vector<state::CardInstanceId> candidates;
	};

	// =================================================================
	// 选择落点
	// =================================================================

	// 选择主要怪兽区落点
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000,
	//     "location": 0
	// }
	class SelectMonsterZoneLocation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		domain::MonsterLocation location;
	};

	// 选择任意怪兽区落点
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000,
	//     "location": {
	//         "column": 0,
	//         "variant_index": 0,
	//         "type": "monster"
	//     }
	// }
	class SelectAnyMonsterZoneLocation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		domain::AnyMonsterLocation location;
	};

	// 选择怪兽表示形式
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000,
	//     "position": "FACE_UP_ATTACK"
	// }
	class SelectMonsterPosition
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		domain::Position position;
	};

	// 选择魔陷区落点
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000,
	//     "location": 0
	// }
	class SelectSpellTrapLocation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		domain::SpellTrapLocation location;
	};

	// =================================================================
	// 表示形式变更
	// =================================================================

	// 查询可变更表示形式的怪兽
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class QueryPositionChangeableCards
	{
	public:
		domain::Player player;
	};

	// 查询目标怪兽可变更的表示形式
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class QueryPositionChangeablePositions
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// =================================================================
	// 通常召唤
	//
	// 选择手中的怪兽卡 -> 选择召唤方式 -> 选择落点
	// =================================================================

	// 查询可以通常召唤的手卡
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class QueryNormalSummonableCards
	{
	public:
		domain::Player player;
	};

	// 查询可用的通常召唤类型
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class QueryNormalSummonType
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询目标卡是否可以按照指定类型召唤
	// JSON:
	// {
	//     "player": "FIRST",
	//     "type": "SUMMON",
	//     "card": 1000
	// }
	class CheckNormalSummonType
	{
	public:
		domain::Player player;
		domain::NormalSummonType type;
		state::CardInstanceId card;
	};

	// 查询可用的通常召唤落点
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class QueryNormalSummonLocation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询可用的通常召唤落点
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class QueryNormalSummonPlacement
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// =================================================================
	// 上级召唤
	//
	// 选择手中的高星怪兽卡 -> 选择祭品 -> 选择召唤方式 -> 选择落点
	// =================================================================

	// 查询可以上级召唤的手卡
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class QueryAdvanceSummonableCards
	{
	public:
		domain::Player player;
	};

	// 查询指定卡上级召唤可解放的祭品
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class QueryAdvanceCandidates
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询可用的上级召唤类型
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class QueryAdvanceSummonType
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询目标卡是否可以按照指定类型上级召唤
	// JSON:
	// {
	//     "player": "FIRST",
	//     "type": "SUMMON",
	//     "card": 1000
	// }
	class CheckAdvanceSummonType
	{
	public:
		domain::Player player;
		domain::NormalSummonType type;
		state::CardInstanceId card;
	};

	// 选择上级召唤祭品
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000,
	//     "tributes": [
	//         1001,
	//         1002
	//     ]
	// }
	class SelectAdvanceTributes
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		std::vector<state::CardInstanceId> tributes;
	};

	// 查询上级召唤可用的落点
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class QueryAdvanceSummonLocation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// =================================================================
	// 反转召唤 / 二重召唤
	// =================================================================

	// 查询可以反转召唤的怪兽
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class QueryFlipSummonableCards
	{
	public:
		domain::Player player;
	};

	// 查询可以二重召唤的怪兽
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class QueryDualSummonableCards
	{
	public:
		domain::Player player;
	};

	// =================================================================
	// 额外卡组特招
	//
	// 选择额外卡组怪兽 -> 选择素材来源 -> 选择素材 -> 选择落点
	// =================================================================

	// 查询可以额外卡组特招的卡
	// JSON:
	// {
	//     "player": "FIRST",
	//     "method": "SYNCHRO"
	// }
	class QueryExtraDeckSummonableCards
	{
	public:
		domain::Player player;
		domain::SummonMethod method;
	};

	// 查询指定卡可用的素材来源
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000,
	//     "method": "SYNCHRO"
	// }
	class QueryExtraDeckSummonMaterialSources
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		domain::SummonMethod method;
	};

	// 查询指定额外卡组怪兽在指定来源下可用的素材候选
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000,
	//     "method": "SYNCHRO",
	//     "material_source": "HAND|FIELD"
	// }
	class QueryExtraDeckSummonMaterials
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		domain::SummonMethod method;
		utility::EnumMask<domain::SummonMaterialSource> material_source;
	};

	// 查询指定卡可用的额外卡组特招落点
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000,
	//     "method": "SYNCHRO"
	// }
	class QueryExtraDeckSummonLocation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		domain::SummonMethod method;
	};

	// =================================================================
	// 灵摆召唤
	// =================================================================

	// 查询当前可进行的灵摆召唤候选
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class QueryPendulumSummonableCards
	{
	public:
		domain::Player player;
	};

	// =================================================================
	// 规则/效果特招
	// =================================================================

	// 查询可以规则/效果特招的卡
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class QuerySpecialSummonableCards
	{
	public:
		domain::Player player;
	};

	// 查询指定卡可用的规则/效果特招落点
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class QuerySpecialSummonLocation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询指定卡可用的规则/效果特招落点(含表示形式)
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class QuerySpecialSummonPlacements
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// =================================================================
	// 盖放魔法/陷阱 / 灵摆放置
	//
	// 选择手卡 -> 查询落点 -> 选择落点
	// =================================================================

	// 查询可盖放的魔法/陷阱手卡
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class QuerySetSpellTrapCards
	{
	public:
		domain::Player player;
	};

	// 查询可用的魔法/陷阱盖放落点
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class QuerySpellTrapSetLocation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询可以放置到灵摆区的卡
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class QueryActivatePendulumCards
	{
	public:
		domain::Player player;
	};

	// 查询指定卡可用的灵摆侧
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class QueryActivatePendulumSide
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// =================================================================
	// 效果发动
	// =================================================================

	// 查询可发动的效果
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class QueryActivatableEffects
	{
	public:
		domain::Player player;
		// 为空表示查询全部
		std::optional<state::CardInstanceId> card;
	};

	// 查询指定效果的可选对象
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000,
	//     "effect": 1
	// }
	class QueryEffectTargets
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		state::EffectId effect;
	};

	// 选择发动效果
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000,
	//     "effect": 1
	// }
	class SelectEffectActivation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		state::EffectId effect;
	};

	// 选择效果对象
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000,
	//     "effect": 1,
	//     "targets": [
	//         1001,
	//         1002
	//     ]
	// }
	class SelectEffectTargets
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		state::EffectId effect;
		std::vector<state::CardInstanceId> targets;
	};

	// =================================================================
	// 连锁
	// =================================================================

	// 查询可连锁的效果
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class QueryChainableEffects
	{
	public:
		domain::Player player;
	};

	// 选择连锁
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000,
	//     "effect": 1
	// }
	class SelectChain
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		state::EffectId effect;
	};

	// =================================================================
	// 攻击
	// =================================================================

	// 查询可攻击的怪兽
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class QueryAttackableCards
	{
	public:
		domain::Player player;
	};

	// 查询指定攻击者是否可以直接攻击
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class CheckDirectAttackable
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询指定攻击者可攻击的目标
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000
	// }
	class QueryAttackTargets
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 选择攻击宣言
	// JSON:
	// {
	//     "player": "FIRST",
	//     "card": 1000,
	//     "target": 1001
	// }
	class SelectAttack
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		// 为空表示直接攻击
		std::optional<state::CardInstanceId> target;
	};

	// =================================================================
	// 阶段
	// =================================================================

	// 查询当前可进入的阶段
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class QueryPhaseTransitions
	{
	public:
		domain::Player player;
	};

	// 选择进入阶段
	// JSON:
	// {
	//     "player": "FIRST",
	//     "to": "BATTLE"
	// }
	class SelectPhaseTransition
	{
	public:
		domain::Player player;
		domain::Phase to;
	};

	// =================================================================
	// 流程控制
	// =================================================================

	// 取消当前选择(回退到上一个决策点)
	// JSON:
	// {
	//     "player": "FIRST",
	//     "context": "NORMAL_SUMMON"
	// }
	class CancelSelection
	{
	public:
		domain::Player player;
		domain::SelectionContext context;
	};

	// 确认当前选择(用于需要两步确认的流程)
	// JSON:
	// {
	//     "player": "FIRST",
	//     "context": "NORMAL_SUMMON"
	// }
	class ConfirmSelection
	{
	public:
		domain::Player player;
		domain::SelectionContext context;
	};

	// 放弃响应(超时/主动放弃)
	// JSON:
	// {
	//     "player": "FIRST"
	// }
	class PassResponse
	{
	public:
		domain::Player player;
	};

	// =================================================================
	// Request
	// =================================================================

	// JSON:
	// {
	//     "player": "FIRST",
	//     "variant_index": 0,
	//     "type": "query_idle_command"
	// }
	class Request : public std::variant<
				// =================================================================
				// 行动阶段
				// =================================================================
				QueryIdleCommand,
				SubmitAction,
				// =================================================================
				// 通用选择
				// =================================================================
				SelectCards,
				SelectYesNo,
				SelectNumbers,
				OrderCards,
				// =================================================================
				// 选择落点
				// =================================================================
				SelectMonsterZoneLocation,
				SelectAnyMonsterZoneLocation,
				SelectMonsterPosition,
				SelectSpellTrapLocation,
				// =================================================================
				// 表示形式变更
				// =================================================================
				QueryPositionChangeableCards,
				QueryPositionChangeablePositions,
				// =================================================================
				// 通常召唤
				// =================================================================
				QueryNormalSummonableCards,
				QueryNormalSummonType,
				CheckNormalSummonType,
				QueryNormalSummonLocation,
				QueryNormalSummonPlacement,
				// =================================================================
				// 上级召唤
				// =================================================================
				QueryAdvanceSummonableCards,
				QueryAdvanceCandidates,
				QueryAdvanceSummonType,
				CheckAdvanceSummonType,
				SelectAdvanceTributes,
				QueryAdvanceSummonLocation,
				// =================================================================
				// 反转召唤 / 二重召唤
				// =================================================================
				QueryFlipSummonableCards,
				QueryDualSummonableCards,
				// =================================================================
				// 额外卡组特招
				// =================================================================
				QueryExtraDeckSummonableCards,
				QueryExtraDeckSummonMaterialSources,
				QueryExtraDeckSummonMaterials,
				QueryExtraDeckSummonLocation,
				// =================================================================
				// 灵摆召唤
				// =================================================================
				QueryPendulumSummonableCards,
				// =================================================================
				// 规则/效果特招
				// =================================================================
				QuerySpecialSummonableCards,
				QuerySpecialSummonLocation,
				QuerySpecialSummonPlacements,
				// =================================================================
				// 盖放魔法/陷阱 / 灵摆放置
				// =================================================================
				QuerySetSpellTrapCards,
				QuerySpellTrapSetLocation,
				QueryActivatePendulumCards,
				QueryActivatePendulumSide,
				// =================================================================
				// 效果发动
				// =================================================================
				QueryActivatableEffects,
				QueryEffectTargets,
				SelectEffectActivation,
				SelectEffectTargets,
				// =================================================================
				// 连锁
				// =================================================================
				QueryChainableEffects,
				SelectChain,
				// =================================================================
				// 攻击
				// =================================================================
				QueryAttackableCards,
				CheckDirectAttackable,
				QueryAttackTargets,
				SelectAttack,
				// =================================================================
				// 阶段
				// =================================================================
				QueryPhaseTransitions,
				SelectPhaseTransition,
				// =================================================================
				// 流程控制
				// =================================================================
				CancelSelection,
				ConfirmSelection,
				PassResponse
			> {};

	[[nodiscard]] constexpr auto player_of(const Request& request) noexcept -> domain::Player
	{
		return std::visit(
			[]<typename T>(const T& any) noexcept -> domain::Player
			{
				return any.player;
			},
			request
		);
	}
}

#pragma once

#include <map>

#include <cg/domain/phase.hpp>
#include <cg/domain/reject_reason.hpp>

#include <cg/driver/action.hpp>

namespace cg::driver::response
{
	// =================================================================
	// 行动阶段
	// =================================================================

	// 查询可用动作
	// JSON:
	// {
	//     "candidates": [
	//         {
	//             "card": 1000,
	//             "placement": {
	//                 "column": 0,
	//                 "position": "FACE_UP_ATTACK"
	//             },
	//             "variant_index": 1,
	//             "type": "normal_summon"
	//         }
	//     ]
	// }
	class QueryIdleCommand
	{
	public:
		// candidates.empty() => 无可用动作
		std::vector<action::Action> candidates;
	};

	// 提交动作
	// JSON:
	// {
	//     "reject": "WRONG_PHASE"
	// }
	class SubmitAction
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// =================================================================
	// 通用选择
	// =================================================================

	// // 选择了一张卡牌
	// class SelectCard {};

	// 从候选集中选择[min, max]张卡牌
	// JSON:
	// {
	//     "reject": "INVALID_TARGET"
	// }
	class SelectCards
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 是/否选择
	// JSON:
	// {
	//     "reject": "CANCELLED"
	// }
	class SelectYesNo
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 对指定卡牌进行排序
	// JSON:
	// {
	//     "reject": "INVALID_TARGET"
	// }
	class OrderCards
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// =================================================================
	// 选择落点
	// =================================================================

	// 选择主要怪兽区落点
	// JSON:
	// {
	//     "reject": "ZONE_OCCUPIED"
	// }
	class SelectMonsterZoneLocation
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 选择任意怪兽区落点
	// JSON:
	// {
	//     "reject": "ZONE_OCCUPIED"
	// }
	class SelectAnyMonsterZoneLocation
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 选择怪兽表示形式
	// JSON:
	// {
	//     "reject": "WRONG_POSITION"
	// }
	class SelectMonsterPosition
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 选择魔陷区落点
	// JSON:
	// {
	//     "reject": "ZONE_OCCUPIED"
	// }
	class SelectSpellTrapLocation
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// =================================================================
	// 表示形式变更
	// =================================================================

	// 查询可变更表示形式的怪兽
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QueryPositionChangeableCards
	{
	public:
		// candidates.empty() => 无可变更的怪兽
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询目标怪兽可变更的表示形式
	// JSON:
	// {
	//     "candidates": [
	//         "FACE_UP_ATTACK",
	//         "FACE_UP_DEFENSE"
	//     ]
	// }
	class QueryPositionChangeablePositions
	{
	public:
		// candidates.empty() => 无可变更的形式
		std::vector<domain::Position> candidates;
	};

	// =================================================================
	// 通常召唤
	//
	// 选择手中的怪兽卡 -> 选择召唤方式 -> 选择落点
	// =================================================================

	// 查询可以通常召唤的手卡
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QueryNormalSummonableCards
	{
	public:
		// candidates.empty() => 无法召唤
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询可用的通常召唤类型
	// JSON:
	// {
	//     "candidates": [
	//         "SUMMON",
	//         "SET"
	//     ]
	// }
	class QueryNormalSummonType
	{
	public:
		// candidates.empty() => 无法召唤
		std::vector<domain::NormalSummonType> candidates;
	};

	// 查询目标卡是否可以按照指定类型召唤
	// JSON:
	// {
	//     "reject": "WRONG_PHASE"
	// }
	class CheckNormalSummonType
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 查询可用的通常召唤落点
	// JSON:
	// {
	//     "candidates": [
	//         0,
	//         1,
	//         2
	//     ]
	// }
	class QueryNormalSummonLocation
	{
	public:
		// candidates.empty() => 无可用落点
		std::vector<domain::MonsterLocation> candidates;
	};

	// 查询可用的通常召唤落点
	// JSON:
	// {
	//     "candidates": [
	//         {
	//             "column": 0,
	//             "position": "FACE_UP_ATTACK"
	//         },
	//         {
	//             "column": 1,
	//             "position": "FACE_DOWN_DEFENSE"
	//         }
	//     ]
	// }
	class QueryNormalSummonPlacement
	{
	public:
		// candidates.empty() => 无可用落点
		std::vector<domain::MonsterPlacement> candidates;
	};

	// =================================================================
	// 上级召唤
	//
	// 选择手中的高星怪兽卡 -> 选择祭品 -> 选择召唤方式 -> 选择落点
	// =================================================================

	// 查询可以上级召唤的手卡
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QueryTributeSummonableCards
	{
	public:
		// candidates.empty() => 无法上级召唤
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询指定卡上级召唤可解放的祭品
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QueryTributeSummonTributeCandidates
	{
	public:
		// candidates.empty() => 无可用祭品
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询可用的上级召唤类型
	// JSON:
	// {
	//     "candidates": [
	//         "SUMMON",
	//         "SET"
	//     ]
	// }
	class QueryTributeSummonType
	{
	public:
		// candidates.empty() => 无法上级召唤
		std::vector<domain::NormalSummonType> candidates;
	};

	// 查询目标卡是否可以按照指定类型上级召唤
	// JSON:
	// {
	//     "reject": "WRONG_PHASE"
	// }
	class CheckTributeSummonType
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 选择上级召唤祭品
	// JSON:
	// {
	//     "reject": "INVALID_TARGET"
	// }
	class SelectTributeSummonTributes
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 查询上级召唤可用的落点
	// JSON:
	// {
	//     "candidates": [
	//         0,
	//         1,
	//         2
	//     ]
	// }
	class QueryTributeSummonLocation
	{
	public:
		// candidates.empty() => 无可用落点
		std::vector<domain::MonsterLocation> candidates;
	};

	// =================================================================
	// 反转召唤 / 二重召唤
	// =================================================================

	// 查询可以反转召唤的怪兽
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QueryFlipSummonableCards
	{
	public:
		// candidates.empty() => 无法反转召唤
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询可以二重召唤的怪兽
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QueryGeminiSummonableCards
	{
	public:
		// candidates.empty() => 无法二重召唤
		std::vector<state::CardInstanceId> candidates;
	};

	// =================================================================
	// 额外卡组特招
	//
	// 选择额外卡组怪兽 -> 选择素材来源 -> 选择素材 -> 选择落点
	// =================================================================

	// 查询可以额外卡组特招的卡
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QueryExtraDeckSummonableCards
	{
	public:
		// candidates.empty() => 无法特招
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询指定卡可用的素材来源
	// JSON:
	// {
	//     "candidates": [
	//         "HAND",
	//         "FIELD",
	//         "GRAVEYARD"
	//     ]
	// }
	class QueryExtraDeckSummonMaterialSources
	{
	public:
		// candidates.empty() => 无可用素材来源
		std::vector<domain::SummonMaterialSource> candidates;
	};

	// 查询指定额外卡组怪兽在指定来源下可用的素材候选
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QueryExtraDeckSummonMaterials
	{
	public:
		// candidates.empty() => 无可用素材
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询指定卡可用的额外卡组特招落点
	// JSON:
	// {
	//     "player": "FIRST",
	//     "candidates": [
	//         {
	//             "column": 0,
	//             "variant_index": 0,
	//             "type": "monster"
	//         },
	//         {
	//             "column": 2,
	//             "variant_index": 1,
	//             "type": "extra_monster"
	//         }
	//     ]
	// }
	class QueryExtraDeckSummonLocation
	{
	public:
		domain::Player player;
		// candidates.empty() => 无可用落点
		std::vector<domain::AnyMonsterLocation> candidates;
	};

	// =================================================================
	// 灵摆召唤
	// =================================================================

	// 查询当前可进行的灵摆召唤候选
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QueryPendulumSummonableCards
	{
	public:
		// candidates.empty() => 无法灵摆召唤
		std::vector<state::CardInstanceId> candidates;
	};

	// =================================================================
	// 规则/效果特招
	// =================================================================

	// 查询可以规则/效果特招的卡
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QuerySpecialSummonableCards
	{
	public:
		// candidates.empty() => 无法特招
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询指定卡可用的规则/效果特招落点
	// JSON:
	// {
	//     "candidates": [
	//         0,
	//         1,
	//         2
	//     ]
	// }
	class QuerySpecialSummonLocation
	{
	public:
		// candidates.empty() => 无可用落点
		std::vector<domain::MonsterLocation> candidates;
	};

	// 查询指定卡可用的规则/效果特招落点(含表示形式)
	// JSON:
	// {
	//     "candidates": [
	//         {
	//             "column": 0,
	//             "position": "FACE_UP_ATTACK"
	//         },
	//         {
	//             "column": 1,
	//             "position": "FACE_DOWN_DEFENSE"
	//         }
	//     ]
	// }
	class QuerySpecialSummonPlacements
	{
	public:
		// candidates.empty() => 无可用落点
		std::vector<domain::MonsterPlacement> candidates;
	};

	// =================================================================
	// 盖放魔法/陷阱 / 灵摆放置
	//
	// 选择手卡 -> 查询落点 -> 选择落点
	// =================================================================

	// 查询可盖放的魔法/陷阱手卡
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QuerySetSpellTrapCards
	{
	public:
		// candidates.empty() => 无可盖放的卡
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询可用的魔法/陷阱盖放落点
	// JSON:
	// {
	//     "candidates": [
	//         0,
	//         1
	//     ]
	// }
	class QuerySpellTrapSetLocation
	{
	public:
		// candidates.empty() => 无可用落点
		std::vector<domain::SpellTrapLocation> candidates;
	};

	// 查询可以放置到灵摆区的卡
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QueryActivatePendulumCards
	{
	public:
		// candidates.empty() => 无可放置的卡
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询指定卡可用的灵摆侧
	// JSON:
	// {
	//     "candidates": [
	//         "LEFT",
	//         "RIGHT"
	//     ]
	// }
	class QueryActivatePendulumSide
	{
	public:
		// candidates.empty() => 无可用灵摆侧
		std::vector<domain::PendulumSide> candidates;
	};

	// =================================================================
	// 效果发动
	// =================================================================

	// 查询可发动的效果
	// JSON:
	// {
	//     "candidates": {
	//         "1000": [
	//             1,
	//             2
	//         ],
	//         "1001": [
	//             3
	//         ]
	//     }
	// }
	class QueryActivatableEffects
	{
	public:
		// candidates.empty() => 无可发动的效果
		std::map<state::CardInstanceId, std::vector<dsl::EffectId>> candidates;
	};

	// 查询指定效果的可选对象
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ],
	//     "min": 1,
	//     "max": 1
	// }
	class QueryEffectTargets
	{
	public:
		// candidates.empty() => 无可选对象
		std::vector<state::CardInstanceId> candidates;
		// 数量约束[min,max]
		domain::DeckLocation::size_type min;
		domain::DeckLocation::size_type max;
	};

	// 选择发动效果
	// JSON:
	// {
	//     "reject": "CANNOT_ACTIVATE"
	// }
	class SelectEffectActivation
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 选择效果对象
	// JSON:
	// {
	//     "reject": "INVALID_TARGET"
	// }
	class SelectEffectTargets
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// =================================================================
	// 连锁
	// =================================================================

	// 查询可连锁的效果
	// JSON:
	// {
	//     "candidates": {
	//         "1000": [
	//             1,
	//             2
	//         ],
	//         "1001": [
	//             3
	//         ]
	//     }
	// }
	class QueryChainableEffects
	{
	public:
		// candidates.empty() => 无可连锁效果
		std::map<state::CardInstanceId, std::vector<dsl::EffectId>> candidates;
	};

	// 选择连锁
	// JSON:
	// {
	//     "reject": "CANNOT_ACTIVATE"
	// }
	class SelectChain
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// =================================================================
	// 攻击
	// =================================================================

	// 查询可攻击的怪兽
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QueryAttackableCards
	{
	public:
		// candidates.empty() => 无可攻击的怪兽
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询指定攻击者是否可以直接攻击
	// JSON:
	// {
	//     "reject": "WRONG_PHASE"
	// }
	class CheckDirectAttackable
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 查询指定攻击者可攻击的目标
	// JSON:
	// {
	//     "candidates": [
	//         1000,
	//         1001
	//     ]
	// }
	class QueryAttackTargets
	{
	public:
		// candidates.empty() => 无可攻击目标
		std::vector<state::CardInstanceId> candidates;
	};

	// 选择攻击宣言
	// JSON:
	// {
	//     "reject": "INVALID_TARGET"
	// }
	class SelectAttack
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// =================================================================
	// 阶段
	// =================================================================

	// 查询当前可进入的阶段
	// JSON:
	// {
	//     "candidates": [
	//         "MAIN_1",
	//         "BATTLE",
	//         "END"
	//     ]
	// }
	class QueryPhaseTransitions
	{
	public:
		// candidates.empty() => 无可用阶段转换(不满足切换阶段要求,例如战斗阶段必须攻击)
		std::vector<domain::Phase> candidates;
	};

	// 选择进入阶段
	// JSON:
	// {
	//     "reject": "WRONG_PHASE"
	// }
	class SelectPhaseTransition
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// =================================================================
	// 流程控制
	// =================================================================

	// 取消当前选择(回退到上一个决策点)
	// JSON:
	// {
	//     "reject": "CANCELLED"
	// }
	class CancelSelection
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 确认当前选择(用于需要两步确认的流程)
	// JSON:
	// {
	//     "reject": "WRONG_PHASE"
	// }
	class ConfirmSelection
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 放弃响应(超时/主动放弃)
	// JSON:
	// {
	//     "reject": "TIMEOUT"
	// }
	class PassResponse
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// =================================================================
	// Response
	// =================================================================

	// JSON:
	// {
	//     "candidates": [
	//         {
	//             "card": 1000,
	//             "placement": {
	//                 "column": 0,
	//                 "position": "FACE_UP_ATTACK"
	//             },
	//             "variant_index": 1,
	//             "type": "normal_summon"
	//         }
	//     ],
	//     "variant_index": 0,
	//     "type": "query_idle_command"
	// }
	class Response : public std::variant<
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
				QueryTributeSummonableCards,
				QueryTributeSummonTributeCandidates,
				QueryTributeSummonType,
				CheckTributeSummonType,
				SelectTributeSummonTributes,
				QueryTributeSummonLocation,
				// =================================================================
				// 反转召唤 / 二重召唤
				// =================================================================
				QueryFlipSummonableCards,
				QueryGeminiSummonableCards,
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
				PassResponse> {};
}

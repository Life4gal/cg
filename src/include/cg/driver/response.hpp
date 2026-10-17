#pragma once

#include <map>

#include <cg/domain/reject_reason.hpp>

#include <cg/driver/action.hpp>

namespace cg::driver::response
{
	// =================================================================
	// 行动阶段
	// =================================================================

	// 查询可用动作
	class QueryIdleCommand
	{
	public:
		// candidates.empty() => 无可用动作
		std::vector<action::Action> candidates;
	};

	// 提交动作
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
	class SelectCards
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 是/否选择
	class SelectYesNo
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 对指定卡牌进行排序
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
	class SelectMonsterZoneLocation
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 选择任意怪兽区落点
	class SelectAnyMonsterZoneLocation
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 选择怪兽表示形式
	class SelectMonsterPosition
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 选择魔陷区落点
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
	class QueryPositionChangeableCards
	{
	public:
		// candidates.empty() => 无可变更的怪兽
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询目标怪兽可变更的表示形式
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
	class QueryNormalSummonableCards
	{
	public:
		// candidates.empty() => 无法召唤
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询可用的通常召唤类型
	class QueryNormalSummonType
	{
	public:
		// candidates.empty() => 无法召唤
		std::vector<domain::NormalSummonType> candidates;
	};

	// 查询目标卡是否可以按照指定类型召唤
	class CheckNormalSummonType
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 查询可用的通常召唤落点
	class QueryNormalSummonLocation
	{
	public:
		// candidates.empty() => 无可用落点
		std::vector<domain::MonsterLocation> candidates;
	};

	// 查询可用的通常召唤落点
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
	class QueryAdvanceSummonableCards
	{
	public:
		// candidates.empty() => 无法上级召唤
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询指定卡上级召唤可解放的祭品
	class QueryAdvanceCandidates
	{
	public:
		// candidates.empty() => 无可用祭品
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询可用的上级召唤类型
	class QueryAdvanceSummonType
	{
	public:
		// candidates.empty() => 无法上级召唤
		std::vector<domain::NormalSummonType> candidates;
	};

	// 查询目标卡是否可以按照指定类型上级召唤
	class CheckAdvanceSummonType
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 选择上级召唤祭品
	class SelectAdvanceTributes
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 查询上级召唤可用的落点
	class QueryAdvanceSummonLocation
	{
	public:
		// candidates.empty() => 无可用落点
		std::vector<domain::MonsterLocation> candidates;
	};

	// =================================================================
	// 反转召唤 / 二重召唤
	// =================================================================

	// 查询可以反转召唤的怪兽
	class QueryFlipSummonableCards
	{
	public:
		// candidates.empty() => 无法反转召唤
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询可以二重召唤的怪兽
	class QueryDualSummonableCards
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
	class QueryExtraDeckSummonableCards
	{
	public:
		// candidates.empty() => 无法特招
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询指定卡可用的素材来源
	class QueryExtraDeckSummonMaterialSources
	{
	public:
		// candidates.empty() => 无可用素材来源
		std::vector<domain::SummonMaterialSource> candidates;
	};

	// 查询指定额外卡组怪兽在指定来源下可用的素材候选
	class QueryExtraDeckSummonMaterials
	{
	public:
		// candidates.empty() => 无可用素材
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询指定卡可用的额外卡组特招落点
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
	class QuerySpecialSummonableCards
	{
	public:
		// candidates.empty() => 无法特招
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询指定卡可用的规则/效果特招落点
	class QuerySpecialSummonLocation
	{
	public:
		// candidates.empty() => 无可用落点
		std::vector<domain::MonsterLocation> candidates;
	};

	// 查询指定卡可用的规则/效果特招落点(含表示形式)
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
	class QuerySetSpellTrapCards
	{
	public:
		// candidates.empty() => 无可盖放的卡
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询可用的魔法/陷阱盖放落点
	class QuerySpellTrapSetLocation
	{
	public:
		// candidates.empty() => 无可用落点
		std::vector<domain::SpellTrapLocation> candidates;
	};

	// 查询可以放置到灵摆区的卡
	class QueryActivatePendulumCards
	{
	public:
		// candidates.empty() => 无可放置的卡
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询指定卡可用的灵摆侧
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
	class QueryActivatableEffects
	{
	public:
		// candidates.empty() => 无可发动的效果
		std::map<state::CardInstanceId, std::vector<state::EffectId>> candidates;
	};

	// 查询指定效果的可选对象
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
	class SelectEffectActivation
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 选择效果对象
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
	class QueryChainableEffects
	{
	public:
		// candidates.empty() => 无可连锁效果
		std::map<state::CardInstanceId, std::vector<state::EffectId>> candidates;
	};

	// 选择连锁
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
	class QueryAttackableCards
	{
	public:
		// candidates.empty() => 无可攻击的怪兽
		std::vector<state::CardInstanceId> candidates;
	};

	// 查询指定攻击者是否可以直接攻击
	class CheckDirectAttackable
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 查询指定攻击者可攻击的目标
	class QueryAttackTargets
	{
	public:
		// candidates.empty() => 无可攻击目标
		std::vector<state::CardInstanceId> candidates;
	};

	// 选择攻击宣言
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
	class QueryPhaseTransitions
	{
	public:
		// candidates.empty() => 无可用阶段转换(不满足切换阶段要求,例如战斗阶段必须攻击)
		std::vector<domain::Phase> candidates;
	};

	// 选择进入阶段
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
	class CancelSelection
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 确认当前选择(用于需要两步确认的流程)
	class ConfirmSelection
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};

	// 放弃响应(超时/主动放弃)
	class PassResponse
	{
	public:
		// reject == null-opt => 成功
		std::optional<domain::RejectReason> reject;
	};
}

namespace cg::driver
{
	// 行动阶段征询,回合玩家从行动列表中选择一个动作进行
	class IdleCommandResponse
	{
	public:
		Action action;

		[[nodiscard]] constexpr auto operator==(const IdleCommandResponse& other) const noexcept -> bool = default;
	};

	// 从候选集中选择[min, max]张卡牌
	class SelectCardResponse
	{
	public:
		std::vector<state::CardInstanceId> select_cards;

		[[nodiscard]] constexpr auto operator==(const SelectCardResponse& other) const noexcept -> bool = default;
	};

	// 是/否选择
	class SelectYesNoResponse
	{
	public:
		bool yes;

		[[nodiscard]] constexpr auto operator==(const SelectYesNoResponse& other) const noexcept -> bool = default;
	};

	// 选择一个数字(如名推理[58577036]选择一个等级)
	class SelectNumberResponse
	{
	public:
		std::int64_t number;

		[[nodiscard]] constexpr auto operator==(const SelectNumberResponse& other) const noexcept -> bool = default;
	};

	// 选择一个卡名(禁止令[43711255]宣言一个卡名)
	class SelectCardNameResponse
	{
	public:
		std::string card_name;

		[[nodiscard]] constexpr auto operator==(const SelectCardNameResponse& other) const noexcept -> bool = default;
	};

	// 选择怪兽落点(仅主怪兽区域)
	class SelectMonsterLocationResponse
	{
	public:
		domain::MonsterLocation location;

		[[nodiscard]] constexpr auto operator==(const SelectMonsterLocationResponse& other) const noexcept -> bool = default;
	};

	// 选择怪兽落点(包括额外怪兽区)
	class SelectMonsterLocationExResponse
	{
	public:
		domain::AnyMonsterLocation location;

		[[nodiscard]] constexpr auto operator==(const SelectMonsterLocationExResponse& other) const noexcept -> bool = default;
	};

	// 选择怪兽表示形式
	class SelectMonsterPositionResponse
	{
	public:
		domain::Position position;

		[[nodiscard]] constexpr auto operator==(const SelectMonsterPositionResponse& other) const noexcept -> bool = default;
	};

	// 选择魔陷落点
	class SelectSpellTrapLocationResponse
	{
	public:
		domain::SpellTrapLocation location;

		[[nodiscard]] constexpr auto operator==(const SelectSpellTrapLocationResponse& other) const noexcept -> bool = default;
	};

	// 选择要发动的效果(如果同一时刻有多个效果可以发动)
	class SelectActivateEffectResponse
	{
	public:
		state::EffectId select_effect;

		[[nodiscard]] constexpr auto operator==(const SelectActivateEffectResponse& other) const noexcept -> bool = default;
	};

	class Response : public std::variant<
				// ==========================================
				// 动作
				// ==========================================
				IdleCommandResponse,

				// ==========================================
				// 选择
				// ==========================================
				SelectCardResponse,
				SelectYesNoResponse,
				SelectNumberResponse,
				SelectCardNameResponse,
				SelectMonsterLocationResponse,
				SelectMonsterLocationExResponse,
				SelectMonsterPositionResponse,
				SelectSpellTrapLocationResponse,
				SelectActivateEffectResponse

				// ==========================================
				//
				// ==========================================
			> {};
}

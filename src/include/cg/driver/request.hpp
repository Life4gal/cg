#pragma once

#include <string>

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
	class QueryIdleCommand
	{
	public:
		domain::Player player;
	};

	// 提交动作
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
	class SelectCards
	{
	public:
		domain::Player player;
		std::vector<state::CardInstanceId> candidates;
		domain::DeckLocation::size_type min;
		domain::DeckLocation::size_type max;
	};

	// 是/否选择
	class SelectYesNo
	{
	public:
		domain::Player player;
	};

	// 对指定卡牌进行排序
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
	class SelectMonsterZoneLocation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		domain::MonsterLocation location;
	};

	// 选择任意怪兽区落点
	class SelectAnyMonsterZoneLocation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		domain::AnyMonsterLocation location;
	};

	// 选择怪兽表示形式
	class SelectMonsterPosition
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		domain::Position position;
	};

	// 选择魔陷区落点
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
	class QueryPositionChangeableCards
	{
	public:
		domain::Player player;
	};

	// 查询目标怪兽可变更的表示形式
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
	class QueryNormalSummonableCards
	{
	public:
		domain::Player player;
	};

	// 查询可用的通常召唤类型
	class QueryNormalSummonType
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询目标卡是否可以按照指定类型召唤
	class CheckNormalSummonType
	{
	public:
		domain::Player player;
		domain::NormalSummonType type;
		state::CardInstanceId card;
	};

	// 查询可用的通常召唤落点
	class QueryNormalSummonLocation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询可用的通常召唤落点
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
	class QueryAdvanceSummonableCards
	{
	public:
		domain::Player player;
	};

	// 查询指定卡上级召唤可解放的祭品
	class QueryAdvanceCandidates
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询可用的上级召唤类型
	class QueryAdvanceSummonType
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询目标卡是否可以按照指定类型上级召唤
	class CheckAdvanceSummonType
	{
	public:
		domain::Player player;
		domain::NormalSummonType type;
		state::CardInstanceId card;
	};

	// 选择上级召唤祭品
	class SelectAdvanceTributes
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		std::vector<state::CardInstanceId> tributes;
	};

	// 查询上级召唤可用的落点
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
	class QueryFlipSummonableCards
	{
	public:
		domain::Player player;
	};

	// 查询可以二重召唤的怪兽
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
	class QueryExtraDeckSummonableCards
	{
	public:
		domain::Player player;
		domain::SummonMethod method;
	};

	// 查询指定卡可用的素材来源
	class QueryExtraDeckSummonMaterialSources
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		domain::SummonMethod method;
	};

	// 查询指定额外卡组怪兽在指定来源下可用的素材候选
	class QueryExtraDeckSummonMaterials
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		domain::SummonMethod method;
		utility::EnumMask<domain::SummonMaterialSource> material_source;
	};

	// 查询指定卡可用的额外卡组特招落点
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
	class QueryPendulumSummonableCards
	{
	public:
		domain::Player player;
	};

	// =================================================================
	// 规则/效果特招
	// =================================================================

	// 查询可以规则/效果特招的卡
	class QuerySpecialSummonableCards
	{
	public:
		domain::Player player;
	};

	// 查询指定卡可用的规则/效果特招落点
	class QuerySpecialSummonLocation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询指定卡可用的规则/效果特招落点(含表示形式)
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
	class QuerySetSpellTrapCards
	{
	public:
		domain::Player player;
	};

	// 查询可用的魔法/陷阱盖放落点
	class QuerySpellTrapSetLocation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询可以放置到灵摆区的卡
	class QueryActivatePendulumCards
	{
	public:
		domain::Player player;
	};

	// 查询指定卡可用的灵摆侧
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
	class QueryActivatableEffects
	{
	public:
		domain::Player player;
		// 为空表示查询全部
		std::optional<state::CardInstanceId> card;
	};

	// 查询指定效果的可选对象
	class QueryEffectTargets
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		state::EffectId effect;
	};

	// 选择发动效果
	class SelectEffectActivation
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
		state::EffectId effect;
	};

	// 选择效果对象
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
	class QueryChainableEffects
	{
	public:
		domain::Player player;
	};

	// 选择连锁
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
	class QueryAttackableCards
	{
	public:
		domain::Player player;
	};

	// 查询指定攻击者是否可以直接攻击
	class CheckDirectAttackable
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 查询指定攻击者可攻击的目标
	class QueryAttackTargets
	{
	public:
		domain::Player player;
		state::CardInstanceId card;
	};

	// 选择攻击宣言
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
	class QueryPhaseTransitions
	{
	public:
		domain::Player player;
	};

	// 选择进入阶段
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
	class CancelSelection
	{
	public:
		domain::Player player;
		domain::SelectionContext context;
	};

	// 确认当前选择(用于需要两步确认的流程)
	class ConfirmSelection
	{
	public:
		domain::Player player;
		domain::SelectionContext context;
	};

	// 放弃响应(超时/主动放弃)
	class PassResponse
	{
	public:
		domain::Player player;
	};
}

namespace cg::driver
{
	// 行动阶段征询,回合玩家从行动列表中选择一个动作进行
	class IdleCommandRequest
	{
	public:
		domain::Player player;
		std::vector<Action> actions;

		[[nodiscard]] constexpr auto operator==(const IdleCommandRequest& other) const noexcept -> bool = default;
	};

	// 从候选集中选择[min, max]张卡牌
	class SelectCardRequest
	{
	public:
		domain::Player player;
		std::vector<state::CardInstanceId> candidates;
		domain::DeckLocation::size_type min;
		domain::DeckLocation::size_type max;

		[[nodiscard]] constexpr auto operator==(const SelectCardRequest& other) const noexcept -> bool = default;
	};

	// TODO: 其他涉及到选卡的操作(例如将指定数量的卡进行排序)

	// 是/否选择
	class SelectYesNoRequest
	{
	public:
		domain::Player player;

		[[nodiscard]] constexpr auto operator==(const SelectYesNoRequest& other) const noexcept -> bool = default;
	};

	// 选择一个数字(如名推理[58577036]选择一个等级)
	class SelectNumberRequest
	{
	public:
		using value_type = std::int64_t;

		domain::Player player;
		value_type min;
		value_type max;

		[[nodiscard]] constexpr auto operator==(const SelectNumberRequest& other) const noexcept -> bool = default;
	};

	// 选择一个卡名(禁止令[43711255]宣言一个卡名)
	class SelectCardNameRequest
	{
	public:
		domain::Player player;
		// 当前可以宣言的卡名
		std::vector<std::string> candidates;

		[[nodiscard]] constexpr auto operator==(const SelectCardNameRequest& other) const noexcept -> bool = default;
	};

	// 选择怪兽落点(仅主怪兽区域)
	class SelectMonsterLocationRequest
	{
	public:
		domain::Player player;
		// 可用落点
		std::vector<domain::MonsterLocation> candidates;

		[[nodiscard]] constexpr auto operator==(const SelectMonsterLocationRequest& other) const noexcept -> bool = default;
	};

	// 选择怪兽落点(包括额外怪兽区)
	class SelectMonsterLocationExRequest
	{
	public:
		domain::Player player;
		// 可用落点
		std::vector<domain::AnyMonsterLocation> candidates;

		[[nodiscard]] constexpr auto operator==(const SelectMonsterLocationExRequest& other) const noexcept -> bool = default;
	};

	// 选择怪兽表示形式
	class SelectMonsterPositionRequest
	{
	public:
		domain::Player player;
		// 可用表示形式
		std::vector<domain::Position> candidates;

		[[nodiscard]] constexpr auto operator==(const SelectMonsterPositionRequest& other) const noexcept -> bool = default;
	};

	// 选择魔陷落点
	class SelectSpellTrapLocationRequest
	{
	public:
		domain::Player player;
		// 可用落点
		std::vector<domain::SpellTrapLocation> candidates;

		[[nodiscard]] constexpr auto operator==(const SelectSpellTrapLocationRequest& other) const noexcept -> bool = default;
	};

	// 选择要发动的效果(如果同一时刻有多个效果可以发动)
	class SelectActivateEffectRequest
	{
	public:
		domain::Player player;
		// 可发动效果
		std::vector<state::EffectId> candidates;

		[[nodiscard]] constexpr auto operator==(const SelectActivateEffectRequest& other) const noexcept -> bool = default;
	};

	class Request : public std::variant<
				// ==========================================
				// 动作
				// ==========================================
				IdleCommandRequest,

				// ==========================================
				// 选择
				// ==========================================
				SelectCardRequest,
				SelectYesNoRequest,
				SelectNumberRequest,
				SelectCardNameRequest,
				SelectMonsterLocationRequest,
				SelectMonsterLocationExRequest,
				SelectMonsterPositionRequest,
				SelectSpellTrapLocationRequest,
				SelectActivateEffectRequest

				// ==========================================
				// 
				// ==========================================
			> {};

	[[nodiscard]] constexpr auto player_of(const Request& request) noexcept -> domain::Player
	{
		return std::visit(
			[](const auto& any) noexcept -> domain::Player
			{
				return any.player;
			},
			request
		);
	}
}

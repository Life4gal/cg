#pragma once

#include <vector>
#include <optional>

#include <cg/utility/enum.hpp>

#include <cg/domain/player.hpp>
#include <cg/domain/card.hpp>
#include <cg/domain/field.hpp>
#include <cg/domain/summon.hpp>

#include <cg/state/id.hpp>

namespace cg::driver::action
{
	// =================================================================
	// 阶段
	// =================================================================

	// 推进到下一阶段
	// JSON:
	// {}
	class AdvancePhase
	{
	public:
		[[nodiscard]] constexpr auto operator==(const AdvancePhase& other) const noexcept -> bool = default;
	};

	// =================================================================
	// 召唤
	// =================================================================

	// 通常召唤
	// JSON:
	// {
	//     "card": 1000,
	//     "placement": {
	//         "column": 0,
	//         "position": "FACE_UP_ATTACK"
	//     }
	// }
	class NormalSummon
	{
	public:
		state::CardInstanceId card;
		// 要么表侧攻击表示,要么里侧守备表示
		domain::MonsterPlacement placement;

		[[nodiscard]] constexpr auto operator==(const NormalSummon& other) const noexcept -> bool = default;
	};

	// 上级召唤
	// JSON:
	// {
	//     "card": 1000,
	//     "materials": [
	//         1001,
	//         1002
	//     ],
	//     "placement": {
	//         "column": 0,
	//         "position": "FACE_UP_ATTACK"
	//     }
	// }
	class AdvanceSummon
	{
	public:
		state::CardInstanceId card;
		// 祭品
		std::vector<state::CardInstanceId> materials;
		// 要么表侧攻击表示,要么里侧守备表示
		domain::MonsterPlacement placement;

		[[nodiscard]] constexpr auto operator==(const AdvanceSummon& other) const noexcept -> bool = default;
	};

	// 反转召唤
	// JSON:
	// {
	//     "card": 1000
	// }
	class FlipSummon
	{
	public:
		state::CardInstanceId card;

		[[nodiscard]] constexpr auto operator==(const FlipSummon& other) const noexcept -> bool = default;
	};

	// 二重召唤
	// JSON:
	// {
	//     "card": 1000
	// }
	class DualSummon
	{
	public:
		state::CardInstanceId card;

		[[nodiscard]] constexpr auto operator==(const DualSummon& other) const noexcept -> bool = default;
	};

	// 额外卡组特招
	// JSON:
	// {
	//     "card": 1000,
	//     "method": "SYNCHRO",
	//     "materials": [
	//         1001,
	//         1002
	//     ],
	//     "placement": {
	//         "column": 0,
	//         "position": "FACE_UP_ATTACK",
	//         "variant_kind": 0
	//     }
	// }
	class ExtraDeckSummon
	{
	public:
		state::CardInstanceId card;
		domain::SummonMethod method;
		// 素材
		std::vector<state::CardInstanceId> materials;
		// 必须是表侧表示,可以在怪兽区和额外怪兽区
		domain::AnyMonsterPlacement placement;

		[[nodiscard]] constexpr auto operator==(const ExtraDeckSummon& other) const noexcept -> bool = default;
	};

	// 灵摆召唤
	// JSON:
	// {
	//     "cards": [
	//         1000,
	//         1001
	//     ],
	//     "placements": [
	//         {
	//             "column": 0,
	//             "position": "FACE_UP_ATTACK"
	//         },
	//         {
	//             "column": 1,
	//             "position": "FACE_UP_DEFENSE"
	//         }
	//     ]
	// }
	class PendulumSummon
	{
	public:
		// 要灵摆召唤的卡
		std::vector<state::CardInstanceId> cards;
		// 每张卡对应的落点,必须表侧表示
		std::vector<domain::MonsterPlacement> placements;

		[[nodiscard]] constexpr auto operator==(const PendulumSummon& other) const noexcept -> bool = default;
	};

	// 怪兽规则/效果特招
	// JSON:
	// {
	//     "card": 1000,
	//     "controller": "FIRST",
	//     "placement": {
	//         "column": 0,
	//         "position": "FACE_UP_ATTACK"
	//     }
	// }
	class SpecialSummon
	{
	public:
		state::CardInstanceId card;
		// 必须是表侧表示,非额外卡组特招不能放置到额外怪兽区
		domain::MonsterPlacement placement;
		// controller决定placement是我方场地还是对方场地
		domain::Player controller;

		[[nodiscard]] constexpr auto operator==(const SpecialSummon& other) const noexcept -> bool = default;
	};

	// =================================================================
	// 表示形式变更
	// =================================================================

	// 表示形式变更
	// JSON:
	// {
	//     "card": 1000,
	//     "position": "FACE_UP_DEFENSE"
	// }
	class ChangePosition
	{
	public:
		state::CardInstanceId card;
		domain::Position position;

		[[nodiscard]] constexpr auto operator==(const ChangePosition& other) const noexcept -> bool = default;
	};

	// =================================================================
	// 战斗
	// =================================================================

	// 进行攻击宣言
	// JSON:
	// {
	//     "attacker": 1000,
	//     "target": 1001
	// }
	class AttackDeclare
	{
	public:
		state::CardInstanceId attacker;
		// 如果为空则表示直接攻击
		std::optional<state::CardInstanceId> target;

		[[nodiscard]] constexpr auto operator==(const AttackDeclare& other) const noexcept -> bool = default;
	};

	// =================================================================
	// 效果
	// =================================================================

	// 发动效果
	// JSON:
	// {
	//     "card": 1000,
	//     "effect": 1
	// }
	class ActiveEffect
	{
	public:
		state::CardInstanceId card;
		state::EffectId effect;

		// TODO: 区分怪兽/魔陷效果?
		// 怪兽效果可以手发(无需放置到场上)

		[[nodiscard]] constexpr auto operator==(const ActiveEffect& other) const noexcept -> bool = default;
	};

	// =================================================================
	// 魔陷 / 灵摆
	// =================================================================

	// 放置魔陷
	// JSON:
	// {
	//     "card": 1000,
	//     "location": 0
	// }
	class SetSpellTrap
	{
	public:
		state::CardInstanceId card;
		// 使用location而不是placement,放置只能里侧,表侧是发动
		domain::SpellTrapLocation location;

		[[nodiscard]] constexpr auto operator==(const SetSpellTrap& other) const noexcept -> bool = default;
	};

	// 放置灵摆怪兽(到灵摆区)
	// JSON:
	// {
	//     "card": 1000,
	//     "side": "LEFT"
	// }
	class ActivatePendulum
	{
	public:
		state::CardInstanceId card;
		domain::PendulumSide side;

		[[nodiscard]] constexpr auto operator==(const ActivatePendulum& other) const noexcept -> bool = default;
	};

	// =================================================================
	// Action
	// =================================================================

	// JSON:
	// {
	//     "card": 1000,
	//     "placement": {
	//         "column": 0,
	//         "position": "FACE_UP_ATTACK"
	//     },
	//     "variant_kind": 1
	// }
	class Action : public std::variant<
				// =================================================================
				// 阶段
				// =================================================================
				AdvancePhase,
				// =================================================================
				// 召唤
				// =================================================================
				NormalSummon,
				AdvanceSummon,
				FlipSummon,
				DualSummon,
				ExtraDeckSummon,
				PendulumSummon,
				SpecialSummon,
				// =================================================================
				// 表示形式变更
				// =================================================================
				ChangePosition,
				// =================================================================
				// 战斗
				// =================================================================
				AttackDeclare,
				// =================================================================
				// 效果
				// =================================================================
				ActiveEffect,
				// =================================================================
				// 魔陷 / 灵摆
				// =================================================================
				SetSpellTrap,
				ActivatePendulum
			> {};
}

namespace cg::driver
{
	// 推进到下一阶段
	// JSON:
	// {}
	class AdvancePhaseAction
	{
	public:
		[[nodiscard]] constexpr auto operator==(const AdvancePhaseAction& other) const noexcept -> bool = default;
	};

	// 通常召唤
	// JSON:
	// {
	//     "card": 1000,
	//     "placement": {
	//         "column": 0,
	//         "position": "FACE_UP_ATTACK"
	//     }
	// }
	class NormalSummonAction
	{
	public:
		state::CardInstanceId card;
		// 要么表侧攻击表示,要么里侧守备表示
		domain::MonsterPlacement placement;

		[[nodiscard]] constexpr auto operator==(const NormalSummonAction& other) const noexcept -> bool = default;
	};

	// 上级召唤
	// JSON:
	// {
	//     "card": 1000,
	//     "materials": [
	//         1001,
	//         1002
	//     ],
	//     "placement": {
	//         "column": 0,
	//         "position": "FACE_UP_ATTACK"
	//     }
	// }
	class AdvanceSummonAction
	{
	public:
		state::CardInstanceId card;
		// 祭品
		std::vector<state::CardInstanceId> materials;
		// 要么表侧攻击表示,要么里侧守备表示
		domain::MonsterPlacement placement;

		[[nodiscard]] constexpr auto operator==(const AdvanceSummonAction& other) const noexcept -> bool = default;
	};

	// 反转召唤
	// JSON:
	// {
	//     "card": 1000
	// }
	class FlipSummonAction
	{
	public:
		state::CardInstanceId card;

		[[nodiscard]] constexpr auto operator==(const FlipSummonAction& other) const noexcept -> bool = default;
	};

	// 二重召唤
	// JSON:
	// {
	//     "card": 1000
	// }
	class DualSummonAction
	{
	public:
		state::CardInstanceId card;

		[[nodiscard]] constexpr auto operator==(const DualSummonAction& other) const noexcept -> bool = default;
	};

	// 额外卡组特招
	// JSON:
	// {
	//     "card": 1000,
	//     "material_source": "HAND|FIELD",
	//     "method": "SYNCHRO",
	//     "placement": {
	//         "column": 0,
	//         "position": "FACE_UP_ATTACK",
	//         "variant_kind": 0
	//     }
	// }
	class ExtraDeckSummonAction
	{
	public:
		state::CardInstanceId card;
		domain::SummonMethod method;
		// 只指定素材来源
		// 素材不在此时选择
		utility::EnumMask<domain::SummonMaterialSource> material_source;
		// 必须是表侧表示,可以在怪兽区和额外怪兽区
		domain::AnyMonsterPlacement placement;

		[[nodiscard]] constexpr auto operator==(const ExtraDeckSummonAction& other) const noexcept -> bool = default;
	};

	// 怪兽规则/效果特招
	// JSON:
	// {
	//     "card": 1000,
	//     "controller": "FIRST",
	//     "placement": {
	//         "column": 0,
	//         "position": "FACE_UP_ATTACK"
	//     }
	// }
	class SpecialSummonAction
	{
	public:
		state::CardInstanceId card;
		// 必须是表侧表示,非额外卡组特招不能放置到额外怪兽区
		domain::MonsterPlacement placement;
		// controller决定placement是我方场地还是对方场地
		domain::Player controller;

		[[nodiscard]] constexpr auto operator==(const SpecialSummonAction& other) const noexcept -> bool = default;
	};

	// 灵摆召唤
	// JSON:
	// {}
	class PendulumSummonAction
	{
	public:
		// TODO

		[[nodiscard]] constexpr auto operator==(const PendulumSummonAction& other) const noexcept -> bool = default;
	};

	// 进行攻击宣言
	// JSON:
	// {
	//     "attacker": 1000,
	//     "target": 1001
	// }
	class AttackDeclareAction
	{
	public:
		state::CardInstanceId attacker;
		// 如果为空则表示直接攻击
		std::optional<state::CardInstanceId> target;

		[[nodiscard]] constexpr auto operator==(const AttackDeclareAction& other) const noexcept -> bool = default;
	};

	// 发动效果
	// JSON:
	// {
	//     "card": 1000,
	//     "effect": 1
	// }
	class ActiveEffectAction
	{
	public:
		state::CardInstanceId card;
		state::EffectId effect;

		// TODO: 区分怪兽/魔陷效果?
		// 怪兽效果可以手发(无需放置到场上)

		[[nodiscard]] constexpr auto operator==(const ActiveEffectAction& other) const noexcept -> bool = default;
	};

	// 放置灵摆怪兽(到灵摆区)
	// JSON:
	// {
	//     "card": 1000,
	//     "side": "LEFT"
	// }
	class ActivatePendulumAction
	{
	public:
		state::CardInstanceId card;
		domain::PendulumSide side;

		[[nodiscard]] constexpr auto operator==(const ActivatePendulumAction& other) const noexcept -> bool = default;
	};

	// 放置魔陷
	// JSON:
	// {
	//     "card": 1000,
	//     "location": 0
	// }
	class SetSpellTrapAction
	{
	public:
		state::CardInstanceId card;
		// 使用location而不是placement,放置只能里侧,表侧是发动
		domain::SpellTrapLocation location;

		[[nodiscard]] constexpr auto operator==(const SetSpellTrapAction& other) const noexcept -> bool = default;
	};

	// JSON:
	// {
	//     "card": 1000,
	//     "placement": {
	//         "column": 0,
	//         "position": "FACE_UP_ATTACK"
	//     },
	//     "variant_kind": 1
	// }
	class Action : public std::variant<
				// ==========================================
				// 阶段
				// ==========================================

				AdvancePhaseAction,

				// ==========================================
				// 召唤
				// ==========================================

				NormalSummonAction,
				AdvanceSummonAction,
				FlipSummonAction,
				DualSummonAction,
				ExtraDeckSummonAction,
				SpecialSummonAction,
				PendulumSummonAction,

				// ==========================================
				// 战斗
				// ==========================================

				AttackDeclareAction,

				// ==========================================
				// 效果
				// ==========================================

				ActiveEffectAction,

				// ==========================================
				// 魔陷
				// ==========================================

				ActivatePendulumAction,
				SetSpellTrapAction

				// ==========================================
				// 
				// ==========================================
			> {};
}

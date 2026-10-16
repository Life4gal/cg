#pragma once

#include <vector>
#include <optional>

#include <cg/domain/player.hpp>
#include <cg/domain/card.hpp>
#include <cg/domain/field.hpp>
#include <cg/domain/summon.hpp>

#include <cg/state/id.hpp>

namespace cg::driver
{
	// 推进到下一阶段
	class AdvancePhaseAction
	{
	public:
		[[nodiscard]] constexpr auto operator==(const AdvancePhaseAction& other) const noexcept -> bool = default;
	};

	// 通常召唤
	class NormalSummonAction
	{
	public:
		state::CardInstanceId card;
		// 要么表侧攻击表示,要么里侧守备表示
		domain::MonsterPlacement placement;

		[[nodiscard]] constexpr auto operator==(const NormalSummonAction& other) const noexcept -> bool = default;
	};

	// 上级召唤
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
	class FlipSummonAction
	{
	public:
		state::CardInstanceId card;

		[[nodiscard]] constexpr auto operator==(const FlipSummonAction& other) const noexcept -> bool = default;
	};

	// 二重召唤
	class DualSummonAction
	{
	public:
		state::CardInstanceId card;

		[[nodiscard]] constexpr auto operator==(const DualSummonAction& other) const noexcept -> bool = default;
	};

	// 额外卡组特招
	class ExtraDeckSummonAction
	{
	public:
		state::CardInstanceId card;
		domain::SummonMethod method;
		// 只指定素材来源
		// 素材不在此时选择
		domain::SummonMaterialSource material_source;
		// 必须是表侧表示,可以在怪兽区和额外怪兽区
		domain::AnyMonsterPlacement placement;

		[[nodiscard]] constexpr auto operator==(const ExtraDeckSummonAction& other) const noexcept -> bool = default;
	};

	// 怪兽规则/效果特招
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
	class PendulumSummonAction
	{
	public:
		// TODO

		[[nodiscard]] constexpr auto operator==(const PendulumSummonAction& other) const noexcept -> bool = default;
	};

	// 进行攻击宣言
	class AttackDeclareAction
	{
	public:
		state::CardInstanceId attacker;
		// 如果为空则表示直接攻击
		std::optional<state::CardInstanceId> target;

		[[nodiscard]] constexpr auto operator==(const AttackDeclareAction& other) const noexcept -> bool = default;
	};

	// 发动效果
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
	class ActivatePendulumAction
	{
	public:
		state::CardInstanceId card;
		domain::PendulumSide side;

		[[nodiscard]] constexpr auto operator==(const ActivatePendulumAction& other) const noexcept -> bool = default;
	};

	// 放置魔陷
	class SetSpellTrapAction
	{
	public:
		state::CardInstanceId card;
		// 使用location而不是placement,放置只能里侧,表侧是发动
		domain::SpellTrapLocation location;

		[[nodiscard]] constexpr auto operator==(const SetSpellTrapAction& other) const noexcept -> bool = default;
	};

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

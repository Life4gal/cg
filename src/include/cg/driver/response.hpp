#pragma once

#include <cg/driver/action.hpp>

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

		[[nodiscard]] constexpr auto operator==(const SelectMonsterLocationResponse& other) const noexcept -> bool = default
	};

	// 选择怪兽落点(包括额外怪兽区)
	class SelectMonsterLocationExResponse
	{
	public:
		domain::AnyMonsterLocation location;

		[[nodiscard]] constexpr auto operator==(const SelectMonsterLocationExResponse& other) const noexcept -> bool = default
	};

	// 选择怪兽表示形式
	class SelectMonsterPositionResponse
	{
	public:
		domain::Position position;

		[[nodiscard]] constexpr auto operator==(const SelectMonsterPositionResponse& other) const noexcept -> bool = default
	};

	// 选择魔陷落点
	class SelectSpellTrapLocationResponse
	{
	public:
		domain::SpellTrapLocation location;

		[[nodiscard]] constexpr auto operator==(const SelectSpellTrapLocationResponse& other) const noexcept -> bool = default
	};

	// 选择要发动的效果(如果同一时刻有多个效果可以发动)
	class SelectActivateEffectResponse
	{
	public:
		state::EffectId select_effect;

		[[nodiscard]] constexpr auto operator==(const SelectActivateEffectResponse& other) const noexcept -> bool = default
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

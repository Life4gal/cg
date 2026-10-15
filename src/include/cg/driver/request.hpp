#pragma once

#include <string>

#include <cg/driver/action.hpp>

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
		std::uint8_t min;
		std::uint8_t max;

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
		domain::Player player;
		std::int64_t min;
		std::int64_t max;

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

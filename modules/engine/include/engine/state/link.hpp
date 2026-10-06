#pragma once

#include <variant>
#include <vector>

#include <domain/player.hpp>
#include <domain/zone.hpp>

#include <engine/state/id.hpp>

namespace cg::engine
{
	class GameState;

	// 被箭头指向的主要怪兽区格
	class MainMonsterLinkTargetCell
	{
	public:
		// 该怪兽格所属玩家
		domain::Player player;
		// 该怪兽区格位置
		domain::MonsterLocation location;

		[[nodiscard]] constexpr auto operator==(const MainMonsterLinkTargetCell& other) const noexcept -> bool = default;
	};

	// 被箭头指向的额外怪兽区格
	class ExtraMonsterLinkTargetCell
	{
	public:
		// 该怪兽区格位置
		domain::ExtraMonsterLocation location;

		[[nodiscard]] constexpr auto operator==(const ExtraMonsterLinkTargetCell& other) const noexcept -> bool = default;
	};

	class LinkTargetCell : public std::variant<
				// 主要怪兽区
				MainMonsterLinkTargetCell,
				// 额外怪兽区
				ExtraMonsterLinkTargetCell
			> {};

	// 获取指定连接怪兽连接箭头指向的怪兽区格
	// 如果实例不存在 -> 返回空
	// 如果不是连接怪兽 -> 返回空
	// 如果不存在控制者 -> 返回空
	[[nodiscard]] auto collect_link_targets(const GameState& state, CardInstanceId card) noexcept -> std::vector<LinkTargetCell>;
}

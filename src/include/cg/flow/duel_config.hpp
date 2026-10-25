#pragma once

#include <array>
#include <vector>

#include <cg/domain/card.hpp>
#include <cg/domain/player.hpp>

namespace cg::flow
{
	// 单方卡组(决斗开始时使用;副卡组仅参与合法性检查)
	class DuelDeck
	{
	public:
		std::vector<domain::CardCode> main_deck;
		std::vector<domain::CardCode> extra_deck;
		std::vector<domain::CardCode> side_deck;
	};

	// 卡组规则
	class DeckRules
	{
	public:
		std::size_t main_min{40};
		std::size_t main_max{60};
		std::size_t extra_max{15};
		std::size_t side_max{15};
		std::size_t max_copies{3};
	};

	// 一局决斗的初始化配置
	class DuelConfig
	{
	public:
		// 随机数种子
		std::uint64_t seed{0};

		// 双方卡组
		std::array<DuelDeck, 2> decks;
		// 卡组规则
		DeckRules deck_rules{};

		// 先手玩家
		domain::Player first_player{domain::Player::FIRST};
		// 第一回合是否抽卡
		bool first_turn_draw{false};

		// 初始LP
		std::underlying_type_t<domain::AttackDefense> life_points{8000};
		// 初始手牌数
		std::size_t opening_hand{5};
	};
}

#pragma once

#include <core/domain/turn.hpp>

#include <core/engine/collection.hpp>

namespace cg::engine
{
	class BattleInfo
	{
	public:
		using size_type = std::uint32_t;

		// 本回合攻击过的卡
		Group attacked_cards;
		// 本回合交战过的卡
		Group battled_cards;
		// 攻击过的次数
		size_type attacked_count;
		// 攻击宣言过的次数
		size_type attack_announced_count;
		// 上一次攻击的回合号
		domain::TurnId attack_turn_id;
		// 上一次攻击被无效的回合号
		domain::TurnId attack_canceled_turn_id;

		BattleInfo() noexcept;

		auto record_attack_announced(domain::TurnId this_turn_id) noexcept -> void;
		auto record_attack_canceled(domain::TurnId this_turn_id) noexcept -> void;
		auto record_attacked_card(Card& card) noexcept -> void;
	};
}

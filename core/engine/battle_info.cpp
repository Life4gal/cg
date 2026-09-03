#include <core/engine/battle_info.hpp>

namespace cg::engine
{
	BattleInfo::BattleInfo() noexcept
		: //attacked_cards{},
		//battled_cards{},
		attacked_count{0},
		attack_announced_count{0},
		attack_turn_id{static_cast<domain::TurnId>(0)},
		attack_canceled_turn_id{static_cast<domain::TurnId>(0)}
	{
		//
	}

	auto BattleInfo::record_attack_announced(const domain::TurnId this_turn_id) noexcept -> void
	{
		attack_announced_count += 1;
		attack_turn_id = this_turn_id;
	}

	auto BattleInfo::record_attack_canceled(const domain::TurnId this_turn_id) noexcept -> void
	{
		attack_canceled_turn_id = this_turn_id;
	}

	auto BattleInfo::record_attacked_card(Card& card) noexcept -> void
	{
		attacked_cards.insert(card);
		battled_cards.insert(card);
		attacked_count += 1;
	}
}

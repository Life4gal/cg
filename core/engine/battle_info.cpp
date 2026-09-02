#include <core/engine/battle_info.hpp>

namespace cg::engine
{
	BattleInfo::BattleInfo() noexcept
		: //attacked_cards_{},
		//battled_cards_{},
		attacked_count_{0},
		attack_announced_count_{0},
		attack_turn_id_{static_cast<domain::TurnId>(0)},
		attack_canceled_turn_id_{static_cast<domain::TurnId>(0)}
	{
		//
	}

	auto BattleInfo::record_attack_announced() noexcept -> void
	{
		attack_announced_count_ += 1;
		// todo: 获取当前回合号
		attack_turn_id_ = static_cast<domain::TurnId>(0);
	}

	auto BattleInfo::record_attack_canceled() noexcept -> void
	{
		// todo: 获取当前回合号
		attack_canceled_turn_id_ = static_cast<domain::TurnId>(0);
	}

	auto BattleInfo::record_attacked_card(Card& card) noexcept -> void
	{
		attacked_cards_.insert(card);
		battled_cards_.insert(card);
		attacked_count_ += 1;
	}

	auto BattleInfo::attacked_cards() const noexcept -> View
	{
		return View{attacked_cards_};
	}

	auto BattleInfo::attacked_count() const noexcept -> size_type
	{
		return attacked_count_;
	}

	auto BattleInfo::attack_announced_count() const noexcept -> size_type
	{
		return attack_announced_count_;
	}

	auto BattleInfo::attack_turn_id() const noexcept -> domain::TurnId
	{
		return attack_turn_id_;
	}

	auto BattleInfo::attack_canceled_turn_id() const noexcept -> domain::TurnId
	{
		return attack_canceled_turn_id_;
	}

	auto BattleInfo::is_attacked_this_turn() const noexcept -> bool
	{
		// todo: 获取当前回合号
		return attack_turn_id() == static_cast<domain::TurnId>(0);
	}

	auto BattleInfo::is_attack_canceled_this_turn() const noexcept -> bool
	{
		// todo: 获取当前回合号
		return attack_canceled_turn_id() == static_cast<domain::TurnId>(0);
	}
}

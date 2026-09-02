#pragma once

#include <core/domain/turn.hpp>

#include <core/engine/collection.hpp>

namespace cg::engine
{
	class BattleInfo
	{
	public:
		using size_type = std::uint32_t;

	private:
		// 本回合攻击过的卡
		Group attacked_cards_;
		// 本回合交战过的卡
		Group battled_cards_;
		// 攻击过的次数
		size_type attacked_count_;
		// 攻击宣言过的次数
		size_type attack_announced_count_;
		// 上一次攻击的回合号
		domain::TurnId attack_turn_id_;
		// 上一次攻击被无效的回合号
		domain::TurnId attack_canceled_turn_id_;

	public:
		BattleInfo() noexcept;

		auto record_attack_announced() noexcept -> void;
		auto record_attack_canceled() noexcept -> void;
		auto record_attacked_card(Card& card) noexcept -> void;

		[[nodiscard]] auto attacked_cards() const noexcept -> View;
		[[nodiscard]] auto attacked_count() const noexcept -> size_type;
		[[nodiscard]] auto attack_announced_count() const noexcept -> size_type;
		[[nodiscard]] auto attack_turn_id() const noexcept -> domain::TurnId;
		[[nodiscard]] auto attack_canceled_turn_id() const noexcept -> domain::TurnId;

		[[nodiscard]] auto is_attacked_this_turn() const noexcept -> bool;
		[[nodiscard]] auto is_attack_canceled_this_turn() const noexcept -> bool;
	};
}

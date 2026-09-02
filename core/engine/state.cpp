#include <core/engine/state.hpp>

namespace cg::engine
{
	State::State() noexcept
		: controller_{domain::Player::FIRST},
		  // zone_{},
		  reason_player_{domain::Player::FIRST},
		  // reason_card_{},
		  // reason_effect_{},
		  reason_{domain::Reason::NORMAL_SUMMON}
	{
		//
	}

	auto State::set_controller(const domain::Player controller) noexcept -> void
	{
		controller_ = controller;
	}

	auto State::set_zone(const domain::Zone zone) noexcept -> void
	{
		zone_ = zone;
	}

	auto State::set_reason_player(const domain::Player reason_player) noexcept -> void
	{
		reason_player_ = reason_player;
	}

	auto State::set_reason_card(const CardOptional reason_card) noexcept -> void
	{
		reason_card_ = reason_card;
	}

	auto State::set_reason_effect(const EffectOptional reason_effect) noexcept -> void
	{
		reason_effect_ = reason_effect;
	}

	auto State::set_reason(const domain::ReasonWrapper reason) noexcept -> void
	{
		reason_ = reason;
	}

	auto State::controller() const noexcept -> domain::Player
	{
		return controller_;
	}

	auto State::zone() const noexcept -> domain::Zone
	{
		return zone_;
	}

	auto State::reason_player() const noexcept -> domain::Player
	{
		return reason_player_;
	}

	auto State::reason_card() const noexcept -> CardOptional
	{
		return reason_card_;
	}

	auto State::reason_effect() const noexcept -> EffectOptional
	{
		return reason_effect_;
	}

	auto State::reason() const noexcept -> domain::ReasonWrapper
	{
		return reason_;
	}

	auto State::is_controller(const domain::Player expected_player) const noexcept -> bool
	{
		return controller() == expected_player;
	}

	auto State::is_zone(const domain::Zone expected_zone) const noexcept -> bool
	{
		return zone() == expected_zone;
	}

	auto State::is_reason_player(const domain::Player expected_player) const noexcept -> bool
	{
		return reason_player() == expected_player;
	}

	auto State::is_reason_card(const CardOptional expected_card) const noexcept -> bool
	{
		return reason_card() == expected_card;
	}

	auto State::is_reason_effect(const EffectOptional expected_effect) const noexcept -> bool
	{
		return reason_effect() == expected_effect;
	}

	auto State::is_reason(const domain::ReasonWrapper expected_reason) const noexcept -> bool
	{
		return reason().all_of(expected_reason);
	}

	auto State::has_reason(const domain::ReasonWrapper expected_reason) const noexcept -> bool
	{
		return reason().any_of(expected_reason);
	}

	auto State::is_field_zone() const noexcept -> bool
	{
		return zone().is_field_zone();
	}

	auto State::is_face_up_form() const noexcept -> bool
	{
		return zone().is_face_up_form();
	}

	auto State::is_face_down_form() const noexcept -> bool
	{
		return zone().is_face_down_form();
	}

	auto State::is_attack_form() const noexcept -> bool
	{
		return zone().is_attack_form();
	}

	auto State::is_defense_form() const noexcept -> bool
	{
		return zone().is_defense_form();
	}
}

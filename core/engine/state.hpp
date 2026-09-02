#pragma once

#include <core/domain/player.hpp>
#include <core/domain/zone.hpp>
#include <core/domain/reason.hpp>

#include <core/engine/card_reference.hpp>
#include <core/engine/effect_reference.hpp>

namespace cg::engine
{
	class State
	{
	public:
		//

	private:
		// 当前控制者
		domain::Player controller_;
		// 当前所在区域
		domain::Zone zone_;

		// 发生移动/状态变化的原因玩家
		domain::Player reason_player_;
		// 发生移动/状态变化的原因卡
		CardOptional reason_card_;
		// 发生移动/状态变化的原因效果
		EffectOptional reason_effect_;
		// 发生移动/状态变化的原因
		domain::ReasonWrapper reason_;

	public:
		State() noexcept;

		auto set_controller(domain::Player controller) noexcept -> void;
		auto set_zone(domain::Zone zone) noexcept -> void;
		auto set_reason_player(domain::Player reason_player) noexcept -> void;
		auto set_reason_card(CardOptional reason_card) noexcept -> void;
		auto set_reason_effect(EffectOptional reason_effect) noexcept -> void;
		auto set_reason(domain::ReasonWrapper reason) noexcept -> void;

		[[nodiscard]] auto controller() const noexcept -> domain::Player;
		[[nodiscard]] auto zone() const noexcept -> domain::Zone;
		[[nodiscard]] auto reason_player() const noexcept -> domain::Player;
		[[nodiscard]] auto reason_card() const noexcept -> CardOptional;
		[[nodiscard]] auto reason_effect() const noexcept -> EffectOptional;
		[[nodiscard]] auto reason() const noexcept -> domain::ReasonWrapper;

		[[nodiscard]] auto is_controller(domain::Player expected_player) const noexcept -> bool;
		[[nodiscard]] auto is_zone(domain::Zone expected_zone) const noexcept -> bool;
		[[nodiscard]] auto is_reason_player(domain::Player expected_player) const noexcept -> bool;
		[[nodiscard]] auto is_reason_card(CardOptional expected_card) const noexcept -> bool;
		[[nodiscard]] auto is_reason_effect(EffectOptional expected_effect) const noexcept -> bool;
		[[nodiscard]] auto is_reason(domain::ReasonWrapper expected_reason) const noexcept -> bool;
		[[nodiscard]] auto has_reason(domain::ReasonWrapper expected_reason) const noexcept -> bool;

		[[nodiscard]] auto is_field_zone() const noexcept -> bool;
		[[nodiscard]] auto is_face_up_form() const noexcept -> bool;
		[[nodiscard]] auto is_face_down_form() const noexcept -> bool;
		[[nodiscard]] auto is_attack_form() const noexcept -> bool;
		[[nodiscard]] auto is_defense_form() const noexcept -> bool;
	};
}

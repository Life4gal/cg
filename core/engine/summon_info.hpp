#pragma once

#include <core/domain/summon.hpp>
#include <core/domain/zone.hpp>
#include <core/domain/player.hpp>
#include <core/domain/turn.hpp>

#include <core/engine/collection.hpp>

namespace cg::engine
{
	class SummonInfo
	{
	public:
		//

	private:
		// 召唤的类型
		domain::SummonKind kind_;
		// 从哪里召唤
		domain::Zone from_zone_;
		// 召唤的玩家
		domain::Player player_;
		// 召唤的回合
		domain::TurnId turn_id_;
		// 召唤使用的材料
		Group materials_;

	public:
		SummonInfo() noexcept;

		auto set_kind(domain::SummonKind kind) noexcept -> void;
		auto set_from_zone(domain::Zone from_zone) noexcept -> void;
		auto set_player(domain::Player player) noexcept -> void;
		auto set_turn_id(domain::TurnId turn_id) noexcept -> void;
		auto set_materials(Group materials) noexcept -> void;

		[[nodiscard]] auto kind() const noexcept -> domain::SummonKind;
		[[nodiscard]] auto from_zone() const noexcept -> domain::Zone;
		[[nodiscard]] auto player() const noexcept -> domain::Player;
		[[nodiscard]] auto turn_id() const noexcept -> domain::TurnId;
		[[nodiscard]] auto materials() const noexcept -> View;

		[[nodiscard]] auto is_kind(domain::SummonKind expected_kind) const noexcept -> bool;
		[[nodiscard]] auto is_from_zone(domain::Zone expected_from_zone) const noexcept -> bool;
		[[nodiscard]] auto is_player(domain::Player expected_player) const noexcept -> bool;
		[[nodiscard]] auto is_turn_id(domain::TurnId expected_turn_id) const noexcept -> bool;
		[[nodiscard]] auto has_material(const Card& card) const noexcept -> bool;
		[[nodiscard]] auto material_count() const noexcept -> std::size_t;
	};
}

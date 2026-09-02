#include <core/engine/summon_info.hpp>

namespace cg::engine
{
	SummonInfo::SummonInfo() noexcept
		: kind_{domain::SummonKind::NORMAL},
		  // from_zone_{},
		  player_{domain::Player::FIRST},
		  turn_id_{static_cast<domain::TurnId>(0)}
	// materials_{}
	{
		//	
	}

	auto SummonInfo::set_kind(const domain::SummonKind kind) noexcept -> void
	{
		kind_ = kind;
	}

	auto SummonInfo::set_from_zone(const domain::Zone from_zone) noexcept -> void
	{
		from_zone_ = from_zone;
	}

	auto SummonInfo::set_player(const domain::Player player) noexcept -> void
	{
		player_ = player;
	}

	auto SummonInfo::set_turn_id(const domain::TurnId turn_id) noexcept -> void
	{
		turn_id_ = turn_id;
	}

	auto SummonInfo::set_materials(Group materials) noexcept -> void
	{
		materials_ = std::move(materials);
	}

	auto SummonInfo::kind() const noexcept -> domain::SummonKind
	{
		return kind_;
	}

	auto SummonInfo::from_zone() const noexcept -> domain::Zone
	{
		return from_zone_;
	}

	auto SummonInfo::player() const noexcept -> domain::Player
	{
		return player_;
	}

	auto SummonInfo::turn_id() const noexcept -> domain::TurnId
	{
		return turn_id_;
	}

	auto SummonInfo::materials() const noexcept -> View
	{
		return View{materials_};
	}

	auto SummonInfo::is_kind(const domain::SummonKind expected_kind) const noexcept -> bool
	{
		return kind_ == expected_kind;
	}

	auto SummonInfo::is_from_zone(const domain::Zone expected_from_zone) const noexcept -> bool
	{
		return from_zone() == expected_from_zone;
	}

	auto SummonInfo::is_player(const domain::Player expected_player) const noexcept -> bool
	{
		return player() == expected_player;
	}

	auto SummonInfo::is_turn_id(const domain::TurnId expected_turn_id) const noexcept -> bool
	{
		return turn_id() == expected_turn_id;
	}

	auto SummonInfo::has_material(const Card& card) const noexcept -> bool
	{
		return materials_.contains(card);
	}

	auto SummonInfo::material_count() const noexcept -> std::size_t
	{
		return materials_.size();
	}
}

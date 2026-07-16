#include <core/card_monster.hpp>

namespace cg
{
	MonsterCard::MonsterCard(const MonsterInfo info) noexcept
		: info_{info}
	{
		//
	}

	auto MonsterCard::monster_info() const noexcept -> MonsterInfo
	{
		return info_;
	}

	auto MonsterCard::monster_category() const noexcept -> MonsterCategory
	{
		return monster_info().category;
	}

	auto MonsterCard::monster_ability() const noexcept -> MonsterAbility
	{
		return monster_info().ability;
	}

	auto MonsterCard::monster_attribute() const noexcept -> MonsterAttribute
	{
		return monster_info().attribute;
	}

	auto MonsterCard::monster_race() const noexcept -> MonsterRace
	{
		return monster_info().race;
	}

	NormalMonsterCard::NormalMonsterCard(const MonsterInfo info, const MonsterLevel level) noexcept
		: MonsterCard{info},
		  level_{level},
		  level_current_{level}
	{
		//
	}

	auto NormalMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto NormalMonsterCard::get_activate_targets(const Dual& dual, const Player& owner) const noexcept -> TargetCards
	{
		// todo
		return {};
	}

	auto NormalMonsterCard::try_activate(const Dual& dual, const Player& owner, TargetCards& targets) noexcept -> bool
	{
		// todo
		return false;
	}

	auto NormalMonsterCard::can_summon(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto NormalMonsterCard::get_summon_materials(const Dual& dual, const Player& owner) const noexcept -> SummonMaterialCards
	{
		// todo
		return {};
	}

	auto NormalMonsterCard::try_summon(Dual& dual, Player& owner, SummonMaterialCards& materials) noexcept -> bool
	{
		// todo
		return false;
	}

	auto NormalMonsterCard::level_origin() const noexcept -> MonsterLevel
	{
		return level_;
	}

	auto NormalMonsterCard::level_current() const noexcept -> MonsterLevel
	{
		return level_current_;
	}

	auto NormalMonsterCard::set_level(const MonsterLevel new_level) noexcept -> void
	{
		level_current_ = new_level;
	}

	RitualMonsterCard::RitualMonsterCard(const MonsterInfo info, const MonsterLevel level) noexcept
		: MonsterCard{info},
		  level_{level},
		  level_current_{level}
	{
		//
	}

	auto RitualMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto RitualMonsterCard::get_activate_targets(const Dual& dual, const Player& owner) const noexcept -> TargetCards
	{
		// todo
		return {};
	}

	auto RitualMonsterCard::try_activate(const Dual& dual, const Player& owner, TargetCards& targets) noexcept -> bool
	{
		// todo
		return false;
	}

	auto RitualMonsterCard::can_summon(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto RitualMonsterCard::get_summon_materials(const Dual& dual, const Player& owner) const noexcept -> SummonMaterialCards
	{
		// todo
		return {};
	}

	auto RitualMonsterCard::try_summon(Dual& dual, Player& owner, SummonMaterialCards& materials) noexcept -> bool
	{
		// todo
		return false;
	}

	auto RitualMonsterCard::level_origin() const noexcept -> MonsterLevel
	{
		return level_;
	}

	auto RitualMonsterCard::level_current() const noexcept -> MonsterLevel
	{
		return level_current_;
	}

	auto RitualMonsterCard::set_level(const MonsterLevel new_level) noexcept -> void
	{
		level_current_ = new_level;
	}

	FusionMonsterCard::FusionMonsterCard(const MonsterInfo info, const MonsterLevel level) noexcept
		: MonsterCard{info},
		  level_{level},
		  level_current_{level}
	{
		//
	}

	auto FusionMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto FusionMonsterCard::get_activate_targets(const Dual& dual, const Player& owner) const noexcept -> TargetCards
	{
		// todo
		return {};
	}

	auto FusionMonsterCard::try_activate(const Dual& dual, const Player& owner, TargetCards& targets) noexcept -> bool
	{
		// todo
		return false;
	}

	auto FusionMonsterCard::can_summon(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto FusionMonsterCard::get_summon_materials(const Dual& dual, const Player& owner) const noexcept -> SummonMaterialCards
	{
		// todo
		return {};
	}

	auto FusionMonsterCard::try_summon(Dual& dual, Player& owner, SummonMaterialCards& materials) noexcept -> bool
	{
		// todo
		return false;
	}

	auto FusionMonsterCard::level_origin() const noexcept -> MonsterLevel
	{
		return level_;
	}

	auto FusionMonsterCard::level_current() const noexcept -> MonsterLevel
	{
		return level_current_;
	}

	auto FusionMonsterCard::set_level(const MonsterLevel new_level) noexcept -> void
	{
		level_current_ = new_level;
	}

	SynchroMonsterCard::SynchroMonsterCard(const MonsterInfo info, const MonsterLevel level) noexcept
		: MonsterCard{info},
		  level_{level},
		  level_current_{level}
	{
		//
	}

	auto SynchroMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto SynchroMonsterCard::get_activate_targets(const Dual& dual, const Player& owner) const noexcept -> TargetCards
	{
		// todo
		return {};
	}

	auto SynchroMonsterCard::try_activate(const Dual& dual, const Player& owner, TargetCards& targets) noexcept -> bool
	{
		// todo
		return false;
	}

	auto SynchroMonsterCard::can_summon(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto SynchroMonsterCard::get_summon_materials(const Dual& dual, const Player& owner) const noexcept -> SummonMaterialCards
	{
		// todo
		return {};
	}

	auto SynchroMonsterCard::try_summon(Dual& dual, Player& owner, SummonMaterialCards& materials) noexcept -> bool
	{
		// todo
		return false;
	}

	auto SynchroMonsterCard::level_origin() const noexcept -> MonsterLevel
	{
		return level_;
	}

	auto SynchroMonsterCard::level_current() const noexcept -> MonsterLevel
	{
		return level_current_;
	}

	auto SynchroMonsterCard::set_level(const MonsterLevel new_level) noexcept -> void
	{
		level_current_ = new_level;
	}

	XyzMonsterCard::XyzMonsterCard(const MonsterInfo info, const MonsterRank rank, XyzMaterialCards materials) noexcept
		: MonsterCard{info},
		  rank_{rank},
		  materials_{std::move(materials)}
	{
		//
	}

	auto XyzMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto XyzMonsterCard::get_activate_targets(const Dual& dual, const Player& owner) const noexcept -> TargetCards
	{
		// todo
		return {};
	}

	auto XyzMonsterCard::try_activate(const Dual& dual, const Player& owner, TargetCards& targets) noexcept -> bool
	{
		// todo
		return false;
	}

	auto XyzMonsterCard::can_summon(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto XyzMonsterCard::get_summon_materials(const Dual& dual, const Player& owner) const noexcept -> SummonMaterialCards
	{
		// todo
		return {};
	}

	auto XyzMonsterCard::try_summon(Dual& dual, Player& owner, SummonMaterialCards& materials) noexcept -> bool
	{
		// todo
		return false;
	}

	auto XyzMonsterCard::rank() const noexcept -> MonsterRank
	{
		return rank_;
	}

	auto XyzMonsterCard::materials() const noexcept -> const XyzMaterialCards&
	{
		return materials_;
	}

	auto XyzMonsterCard::add_material(XyzMaterialCard material) noexcept -> void
	{
		materials_.add(std::move(material));
	}

	auto XyzMonsterCard::remove_material(const size_type index) noexcept -> bool
	{
		return materials_.remove(index);
	}

	PendulumMonsterCard::PendulumMonsterCard(const MonsterInfo info, const MonsterLevel level, const MonsterPendulum left_pendulum, const MonsterPendulum right_pendulum) noexcept
		: MonsterCard{info},
		  level_{level},
		  level_current_{level},
		  left_pendulum_{left_pendulum},
		  right_pendulum_{right_pendulum}
	{
		//
	}

	auto PendulumMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto PendulumMonsterCard::get_activate_targets(const Dual& dual, const Player& owner) const noexcept -> TargetCards
	{
		// todo
		return {};
	}

	auto PendulumMonsterCard::try_activate(const Dual& dual, const Player& owner, TargetCards& targets) noexcept -> bool
	{
		// todo
		return false;
	}

	auto PendulumMonsterCard::can_summon(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto PendulumMonsterCard::get_summon_materials(const Dual& dual, const Player& owner) const noexcept -> SummonMaterialCards
	{
		// todo
		return {};
	}

	auto PendulumMonsterCard::try_summon(Dual& dual, Player& owner, SummonMaterialCards& materials) noexcept -> bool
	{
		// todo
		return false;
	}

	auto PendulumMonsterCard::level_origin() const noexcept -> MonsterLevel
	{
		return level_;
	}

	auto PendulumMonsterCard::level_current() const noexcept -> MonsterLevel
	{
		return level_current_;
	}

	auto PendulumMonsterCard::set_level(const MonsterLevel new_level) noexcept -> void
	{
		level_current_ = new_level;
	}

	auto PendulumMonsterCard::left_pendulum() const noexcept -> MonsterPendulum
	{
		return left_pendulum_;
	}

	auto PendulumMonsterCard::right_pendulum() const noexcept -> MonsterPendulum
	{
		return right_pendulum_;
	}

	LinkMonsterCard::LinkMonsterCard(const MonsterInfo info, const MonsterLinkMarker link_marker) noexcept
		: MonsterCard{info},
		  link_marker_{link_marker}
	{
		//
	}

	auto LinkMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto LinkMonsterCard::get_activate_targets(const Dual& dual, const Player& owner) const noexcept -> TargetCards
	{
		// todo
		return {};
	}

	auto LinkMonsterCard::try_activate(const Dual& dual, const Player& owner, TargetCards& targets) noexcept -> bool
	{
		// todo
		return false;
	}

	auto LinkMonsterCard::can_summon(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto LinkMonsterCard::get_summon_materials(const Dual& dual, const Player& owner) const noexcept -> SummonMaterialCards
	{
		// todo
		return {};
	}

	auto LinkMonsterCard::try_summon(Dual& dual, Player& owner, SummonMaterialCards& materials) noexcept -> bool
	{
		// todo
		return false;
	}

	auto LinkMonsterCard::link_marker() const noexcept -> MonsterLinkMarker
	{
		return link_marker_;
	}
}

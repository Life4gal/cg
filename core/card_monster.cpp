#include <core/card_monster.hpp>

namespace cg
{
	MonsterCard::MonsterCard(
		const MonsterCategory category,
		const MonsterAbility ability,
		const MonsterAttribute attribute,
		const MonsterRace race
	) noexcept
		: monster_category_{category},
		  monster_ability_{ability},
		  monster_attribute_{attribute},
		  monster_race_{race}
	{
		//
	}

	auto MonsterCard::monster_category() const noexcept -> MonsterCategory
	{
		return monster_category_;
	}

	auto MonsterCard::monster_ability() const noexcept -> MonsterAbility
	{
		return monster_ability_;
	}

	auto MonsterCard::monster_attribute() const noexcept -> MonsterAttribute
	{
		return monster_attribute_;
	}

	auto MonsterCard::monster_race() const noexcept -> MonsterRace
	{
		return monster_race_;
	}

	NormalMonsterCard::NormalMonsterCard(
		const MonsterCategory category,
		const MonsterAbility ability,
		const MonsterAttribute attribute,
		const MonsterRace race,
		const MonsterLevel level
	) noexcept
		: MonsterCard{category, ability, attribute, race},
		  monster_level_{level},
		  monster_level_current_{level}
	{
		//
	}

	auto NormalMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto NormalMonsterCard::try_activate(Dual& dual, Player& owner) noexcept -> void
	{
		// todo
	}

	auto NormalMonsterCard::can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto NormalMonsterCard::try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void
	{
		// todo
	}

	auto NormalMonsterCard::level_origin() const noexcept -> MonsterLevel
	{
		return monster_level_;
	}

	auto NormalMonsterCard::level_current() const noexcept -> MonsterLevel
	{
		return monster_level_current_;
	}

	auto NormalMonsterCard::set_level(const MonsterLevel new_level) noexcept -> void
	{
		monster_level_current_ = new_level;
	}

	RitualMonsterCard::RitualMonsterCard(
		const MonsterCategory category,
		const MonsterAbility ability,
		const MonsterAttribute attribute,
		const MonsterRace race,
		const MonsterLevel level
	) noexcept
		: MonsterCard{category, ability, attribute, race},
		  monster_level_{level},
		  monster_level_current_{level}
	{
		//
	}

	auto RitualMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto RitualMonsterCard::try_activate(Dual& dual, Player& owner) noexcept -> void
	{
		// todo
	}

	auto RitualMonsterCard::can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto RitualMonsterCard::try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void
	{
		// todo
	}

	auto RitualMonsterCard::level_origin() const noexcept -> MonsterLevel
	{
		return monster_level_;
	}

	auto RitualMonsterCard::level_current() const noexcept -> MonsterLevel
	{
		return monster_level_current_;
	}

	auto RitualMonsterCard::set_level(const MonsterLevel new_level) noexcept -> void
	{
		monster_level_current_ = new_level;
	}

	FusionMonsterCard::FusionMonsterCard(
		const MonsterCategory category,
		const MonsterAbility ability,
		const MonsterAttribute attribute,
		const MonsterRace race,
		const MonsterLevel level
	) noexcept
		: MonsterCard{category, ability, attribute, race},
		  monster_level_{level},
		  monster_level_current_{level}
	{
		//
	}

	auto FusionMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto FusionMonsterCard::try_activate(Dual& dual, Player& owner) noexcept -> void
	{
		// todo
	}

	auto FusionMonsterCard::can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto FusionMonsterCard::try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void
	{
		// todo
	}

	auto FusionMonsterCard::level_origin() const noexcept -> MonsterLevel
	{
		return monster_level_;
	}

	auto FusionMonsterCard::level_current() const noexcept -> MonsterLevel
	{
		return monster_level_current_;
	}

	auto FusionMonsterCard::set_level(const MonsterLevel new_level) noexcept -> void
	{
		monster_level_current_ = new_level;
	}

	SynchroMonsterCard::SynchroMonsterCard(
		const MonsterCategory category,
		const MonsterAbility ability,
		const MonsterAttribute attribute,
		const MonsterRace race,
		const MonsterLevel level
	) noexcept
		: MonsterCard{category, ability, attribute, race},
		  monster_level_{level},
		  monster_level_current_{level}
	{
		//
	}

	auto SynchroMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto SynchroMonsterCard::try_activate(Dual& dual, Player& owner) noexcept -> void
	{
		// todo
	}

	auto SynchroMonsterCard::can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto SynchroMonsterCard::try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void
	{
		// todo
	}

	auto SynchroMonsterCard::level_origin() const noexcept -> MonsterLevel
	{
		return monster_level_;
	}

	auto SynchroMonsterCard::level_current() const noexcept -> MonsterLevel
	{
		return monster_level_current_;
	}

	auto SynchroMonsterCard::set_level(const MonsterLevel new_level) noexcept -> void
	{
		monster_level_current_ = new_level;
	}

	XyzMonsterCard::XyzMonsterCard(
		const MonsterCategory category,
		const MonsterAbility ability,
		const MonsterAttribute attribute,
		const MonsterRace race,
		const MonsterRank rank
	) noexcept
		: MonsterCard{category, ability, attribute, race},
		  monster_rank_{rank}
	{
		//
	}

	auto XyzMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto XyzMonsterCard::try_activate(Dual& dual, Player& owner) noexcept -> void
	{
		// todo
	}

	auto XyzMonsterCard::can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto XyzMonsterCard::try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void
	{
		// todo
	}

	auto XyzMonsterCard::rank() const noexcept -> MonsterRank
	{
		return monster_rank_;
	}

	PendulumMonsterCard::PendulumMonsterCard(
		const MonsterCategory category,
		const MonsterAbility ability,
		const MonsterAttribute attribute,
		const MonsterRace race,
		const MonsterLevel level,
		const MonsterPendulum left_pendulum,
		const MonsterPendulum right_pendulum
	) noexcept
		: MonsterCard{category, ability, attribute, race},
		  monster_level_{level},
		  monster_level_current_{level},
		  left_monster_pendulum_{left_pendulum},
		  right_monster_pendulum_{right_pendulum}
	{
		//
	}

	auto PendulumMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto PendulumMonsterCard::try_activate(Dual& dual, Player& owner) noexcept -> void
	{
		// todo
	}

	auto PendulumMonsterCard::can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto PendulumMonsterCard::try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void
	{
		// todo
	}

	auto PendulumMonsterCard::level_origin() const noexcept -> MonsterLevel
	{
		return monster_level_;
	}

	auto PendulumMonsterCard::level_current() const noexcept -> MonsterLevel
	{
		return monster_level_current_;
	}

	auto PendulumMonsterCard::set_level(const MonsterLevel new_level) noexcept -> void
	{
		monster_level_current_ = new_level;
	}

	auto PendulumMonsterCard::left_pendulum() const noexcept -> MonsterPendulum
	{
		return left_monster_pendulum_;
	}

	auto PendulumMonsterCard::right_pendulum() const noexcept -> MonsterPendulum
	{
		return right_monster_pendulum_;
	}

	LinkMonsterCard::LinkMonsterCard(
		const MonsterCategory category,
		const MonsterAbility ability,
		const MonsterAttribute attribute,
		const MonsterRace race,
		const MonsterLinkMarker link_marker
	) noexcept
		: MonsterCard{category, ability, attribute, race},
		  monster_link_marker_{link_marker}
	{
		//
	}

	auto LinkMonsterCard::can_activate(const Dual& dual, const Player& owner) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto LinkMonsterCard::try_activate(Dual& dual, Player& owner) noexcept -> void
	{
		// todo
	}

	auto LinkMonsterCard::can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool
	{
		// todo
		return false;
	}

	auto LinkMonsterCard::try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void
	{
		// todo
	}

	auto LinkMonsterCard::link_marker() const noexcept -> MonsterLinkMarker
	{
		return monster_link_marker_;
	}
}

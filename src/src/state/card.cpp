#include <cg/state/card.hpp>

#include <libassert/assert.hpp>

namespace cg::state
{
	CardInstance::CardInstance(const CardInstanceId id, const domain::Player owner, const CardDefinition& definition) noexcept
		: definition_{definition},
		  id_{id},
		  owner_{owner},
		  controller_{owner},
		  // placement_{},
		  // overlays_{},
		  move_
		  {
				  //
		  },
		  summon_
		  {
				  //
		  },
		  form_
		  {
				  //
		  } {}

	auto CardInstance::definition() const noexcept -> const CardDefinition&
	{
		return definition_.get();
	}

	auto CardInstance::id() const noexcept -> CardInstanceId
	{
		return id_;
	}

	auto CardInstance::owner() const noexcept -> domain::Player
	{
		return owner_;
	}

	auto CardInstance::controller() const noexcept -> domain::Player
	{
		return controller_;
	}

	auto CardInstance::placement() const noexcept -> domain::Placement
	{
		return placement_;
	}

	auto CardInstance::code() const noexcept -> domain::CardCode
	{
		// TODO

		// 否则基于定义
		return definition_.get().code;
	}

	auto CardInstance::is_card_type(const domain::CardType type) const noexcept -> bool
	{
		// 当前形态是怪兽(如被当作怪兽的魔法/陷阱)
		if (type == domain::CardType::MONSTER && form_.monster.has_value())
		{
			return true;
		}

		// 否则基于定义
		return definition_.get().kind.contains(type);
	}

	auto CardInstance::is_monster_card() const noexcept -> bool
	{
		// 如果当前形态是怪兽
		if (form_.monster.has_value())
		{
			return true;
		}
		// TODO

		// 否则基于定义
		return definition_.get().kind.contains(domain::CardType::MONSTER);
	}

	auto CardInstance::is_spell_card() const noexcept -> bool
	{
		// 如果当前形态是永续魔法
		if (form_.continuous_spell)
		{
			return true;
		}
		// TODO

		// 否则基于定义
		return definition_.get().kind.contains(domain::CardType::SPELL);
	}

	auto CardInstance::is_trap_card() const noexcept -> bool
	{
		// TODO

		// 否则基于定义
		return definition_.get().kind.contains(domain::CardType::TRAP);
	}

	auto CardInstance::is_card_subtype(const domain::CardSubtype subtype) const noexcept -> bool
	{
		// TODO

		// 否则基于定义
		return definition_.get().subtypes.contains(subtype);
	}

	auto CardInstance::attribute() const noexcept -> domain::Attribute
	{
		// TODO

		// 否则基于定义
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());
		return definition_.get().monster->attribute;
	}

	auto CardInstance::is_attribute(const domain::Attribute attribute) const noexcept -> bool
	{
		// TODO

		// 否则基于定义
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());
		return definition_.get().monster->attribute == attribute;
	}

	auto CardInstance::race() const noexcept -> domain::Race
	{
		// TODO

		// 否则基于定义
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());
		return definition_.get().monster->race;
	}

	auto CardInstance::is_race(const domain::Race race) const noexcept -> bool
	{
		// TODO

		// 否则基于定义
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());
		return definition_.get().monster->race == race;
	}

	auto CardInstance::level() const noexcept -> std::underlying_type_t<domain::LevelRank>
	{
		// TODO

		// 否则基于定义
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());
		return definition_.get().monster->level;
	}

	auto CardInstance::is_level(const std::underlying_type_t<domain::LevelRank> level) const noexcept -> bool
	{
		// TODO

		// 否则基于定义
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());
		return definition_.get().monster->level == level;
	}

	auto CardInstance::attack() const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		// TODO

		// 否则基于定义
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());
		return definition_.get().monster->attack;
	}

	auto CardInstance::is_attack(const std::underlying_type_t<domain::AttackDefense> attack) const noexcept -> bool
	{
		// TODO

		// 否则基于定义
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());
		return definition_.get().monster->attack == attack;
	}

	auto CardInstance::defense() const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		// TODO

		// 否则基于定义
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());
		return definition_.get().monster->defense;
	}

	auto CardInstance::is_defense(const std::underlying_type_t<domain::AttackDefense> defense) const noexcept -> bool
	{
		// TODO

		// 否则基于定义
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());
		return definition_.get().monster->defense == defense;
	}

	auto CardInstance::set_placement(const domain::Placement& placement) noexcept -> void
	{
		placement_ = placement;
	}

	auto CardInstance::set_move_provenance(const MoveProvenance& provenance) noexcept -> void
	{
		move_ = provenance;
	}
}

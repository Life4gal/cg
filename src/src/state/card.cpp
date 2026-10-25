#include <cg/state/card.hpp>

#include <algorithm>
#include <ranges>

#include <libassert/assert.hpp>

namespace cg::state
{
	CardInstance::Modifier::Modifier(CardInstance& instance) noexcept
		: instance_{instance} {}

	auto CardInstance::Modifier::ref() noexcept -> CardInstance&
	{
		std::ignore = this;

		return instance_.get();
	}

	auto CardInstance::Modifier::set_controller(const domain::Player controller) noexcept -> void
	{
		ref().controller_ = controller;
	}

	auto CardInstance::Modifier::set_placement(const domain::Placement& placement) noexcept -> void
	{
		auto& instance = ref();

		instance.placement_ = placement;
		if (const auto zone = domain::zone_of(placement);
			zone == domain::Zone::DECK || zone == domain::Zone::EXTRA_DECK || zone == domain::Zone::HAND)
		{
			instance.properly_summoned_ = false;
		}
	}

	auto CardInstance::Modifier::set_move_provenance(const MoveProvenance& provenance) noexcept -> void
	{
		ref().move_ = provenance;
	}

	auto CardInstance::Modifier::set_summon_record(SummonRecord record) noexcept -> void
	{
		auto& instance = ref();

		instance.summon_ = std::move(record);
		if (!instance.properly_summoned_)
		{
			const auto properly_summoned =
					(instance.summon_.method == domain::SummonMethod::FUSION && instance.origin_is_card_subtype(domain::CardSubtype::FUSION)) ||
					(instance.summon_.method == domain::SummonMethod::SYNCHRO && instance.origin_is_card_subtype(domain::CardSubtype::SYNCHRO)) ||
					(instance.summon_.method == domain::SummonMethod::XYZ && instance.origin_is_card_subtype(domain::CardSubtype::XYZ)) ||
					(instance.summon_.method == domain::SummonMethod::LINK && instance.origin_is_card_subtype(domain::CardSubtype::LINK)) ||
					(instance.summon_.method == domain::SummonMethod::RITUAL && instance.origin_is_card_subtype(domain::CardSubtype::RITUAL)
					);

			instance.properly_summoned_ = properly_summoned;
		}
	}

	auto CardInstance::Modifier::add_overlay(const CardInstance& material) noexcept -> void
	{
		ref().overlays_.push_back(material.id());
	}

	auto CardInstance::Modifier::remove_overlay(const CardInstance& material) noexcept -> bool
	{
		return std::erase(ref().overlays_, material.id()) != 0;
	}

	CardInstance::CardInstance(const CardInstanceId id, const domain::Player owner, const dsl::CardIdentity& definition) noexcept
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
		  properly_summoned_{false},
		  token_{false} {}

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

	auto CardInstance::is_token() const noexcept -> bool
	{
		return token_;
	}

	auto CardInstance::origin_code() const noexcept -> domain::CardCode
	{
		return definition_.get().code;
	}

	auto CardInstance::origin_kind() const noexcept -> domain::CardType
	{
		return definition_.get().kind;
	}

	auto CardInstance::origin_is_card_type(const domain::CardType type) const noexcept -> bool
	{
		return origin_kind() == type;
	}

	auto CardInstance::origin_is_monster_card() const noexcept -> bool
	{
		return origin_is_card_type(domain::CardType::MONSTER);
	}

	auto CardInstance::origin_is_spell_card() const noexcept -> bool
	{
		return origin_is_card_type(domain::CardType::SPELL);
	}

	auto CardInstance::origin_is_trap_card() const noexcept -> bool
	{
		return origin_is_card_type(domain::CardType::TRAP);
	}

	auto CardInstance::origin_subtypes() const noexcept -> utility::EnumMask<domain::CardSubtype>
	{
		return definition_.get().subtypes;
	}

	auto CardInstance::origin_is_card_subtype(const domain::CardSubtype subtype) const noexcept -> bool
	{
		return origin_subtypes().contains(subtype);
	}

	auto CardInstance::origin_is_card_subtype(const utility::EnumMask<domain::CardSubtype> subtypes) const noexcept -> bool
	{
		return !origin_subtypes().intersect(subtypes).empty();
	}

	auto CardInstance::origin_attribute() const noexcept -> domain::Attribute
	{
		LIBASSERT_DEBUG_ASSERT(origin_is_monster_card());
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());

		return definition_.get().monster->attribute; // NOLINT(bugprone-unchecked-optional-access)
	}

	auto CardInstance::origin_is_attribute(const domain::Attribute attribute) const noexcept -> bool
	{
		return origin_attribute() == attribute;
	}

	auto CardInstance::origin_race() const noexcept -> domain::Race
	{
		LIBASSERT_DEBUG_ASSERT(origin_is_monster_card());
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());

		return definition_.get().monster->race; // NOLINT(bugprone-unchecked-optional-access)
	}

	auto CardInstance::origin_is_race(const domain::Race race) const noexcept -> bool
	{
		return origin_race() == race;
	}

	auto CardInstance::origin_level() const noexcept -> std::underlying_type_t<domain::LevelRank>
	{
		LIBASSERT_DEBUG_ASSERT(origin_is_monster_card());
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());

		return std::to_underlying(definition_.get().monster->level); // NOLINT(bugprone-unchecked-optional-access)
	}

	auto CardInstance::origin_is_level(const std::underlying_type_t<domain::LevelRank> level) const noexcept -> bool
	{
		return origin_level() == level;
	}

	auto CardInstance::origin_attack() const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		LIBASSERT_DEBUG_ASSERT(origin_is_monster_card());
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());

		return std::to_underlying(definition_.get().monster->attack); // NOLINT(bugprone-unchecked-optional-access)
	}

	auto CardInstance::origin_is_attack(const std::underlying_type_t<domain::AttackDefense> attack) const noexcept -> bool
	{
		return origin_attack() == attack;
	}

	auto CardInstance::origin_defense() const noexcept -> std::underlying_type_t<domain::AttackDefense>
	{
		LIBASSERT_DEBUG_ASSERT(origin_is_monster_card());
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());

		return std::to_underlying(definition_.get().monster->defense); // NOLINT(bugprone-unchecked-optional-access)
	}

	auto CardInstance::origin_is_defense(const std::underlying_type_t<domain::AttackDefense> defense) const noexcept -> bool
	{
		return origin_defense() == defense;
	}

	auto CardInstance::origin_pendulum_scale() const noexcept -> dsl::CardIdentity::Monster::PendulumScale
	{
		LIBASSERT_DEBUG_ASSERT(origin_is_monster_card());
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster->pendulum_scale.has_value()); // NOLINT(bugprone-unchecked-optional-access)

		return *definition_.get().monster->pendulum_scale; // NOLINT(bugprone-unchecked-optional-access)
	}

	auto CardInstance::origin_has_pendulum_scale() const noexcept -> bool
	{
		const auto& definition = definition_.get();

		return definition.monster.has_value() && definition.monster->pendulum_scale.has_value();
	}

	auto CardInstance::origin_is_pendulum_scale(const domain::PendulumSide side, const std::underlying_type_t<domain::PendulumScale> scale) const noexcept -> bool
	{
		const auto [left, right] = origin_pendulum_scale();
		if (side == domain::PendulumSide::LEFT)
		{
			return std::to_underlying(left) == scale;
		}
		if (side == domain::PendulumSide::RIGHT)
		{
			return std::to_underlying(right) == scale;
		}
		std::unreachable();
	}

	auto CardInstance::origin_link_marker() const noexcept -> utility::EnumMask<domain::LinkMarker>
	{
		LIBASSERT_DEBUG_ASSERT(origin_is_monster_card());
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster.has_value());
		LIBASSERT_DEBUG_ASSERT(definition_.get().monster->link_marker.has_value()); // NOLINT(bugprone-unchecked-optional-access)

		return *definition_.get().monster->link_marker; // NOLINT(bugprone-unchecked-optional-access)
	}

	auto CardInstance::origin_has_link_marker(const domain::LinkMarker mark) const noexcept -> bool
	{
		const auto l = origin_link_marker();
		return l.contains(mark);
	}

	auto CardInstance::origin_has_link_marker(const utility::EnumMask<domain::LinkMarker> marks) const noexcept -> bool
	{
		const auto ms = origin_link_marker();
		return !ms.intersect(marks).empty();
	}

	auto CardInstance::move_provenance() const noexcept -> const MoveProvenance&
	{
		return move_;
	}

	auto CardInstance::summon_record() const noexcept -> const SummonRecord&
	{
		return summon_;
	}

	auto CardInstance::properly_summoned() const noexcept -> bool
	{
		return properly_summoned_;
	}

	auto CardInstance::overlays() const noexcept -> const std::vector<CardInstanceId>&
	{
		return overlays_;
	}
}

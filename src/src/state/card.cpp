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

		const auto old_placement = instance.placement_;
		const auto& new_placement = placement;

		const auto old_zone = domain::zone_of(old_placement);
		const auto new_zone = domain::zone_of(new_placement);

		const auto old_position = domain::position_of(old_placement);
		const auto new_position = domain::position_of(new_placement);

		const auto is_field_zone = [](const domain::Zone zone) noexcept -> bool
		{
			return
					zone == domain::Zone::MONSTER ||
					zone == domain::Zone::EXTRA_MONSTER ||
					zone == domain::Zone::SPELL_TRAP ||
					zone == domain::Zone::FIELD_SPELL;
		};
		const auto is_face_up = [](const std::optional<domain::Position>& position) noexcept -> bool
		{
			return
					position.has_value() &&
					(*position == domain::Position::FACE_UP_ATTACK || *position == domain::Position::FACE_UP_DEFENSE);
		};

		// 表侧存在周期结束:离场或从表侧翻到里侧
		// 单纯的攻守表示变化、区域移动或控制权转移不结束周期
		if (
			is_field_zone(old_zone) &&
			(!is_field_zone(new_zone) || (is_face_up(old_position) && !is_face_up(new_position)))
		)
		{
			instance.face_up_generation_ += 1;
		}

		instance.placement_ = new_placement;
		if (const auto zone = domain::zone_of(new_placement);
			zone == domain::Zone::DECK || zone == domain::Zone::EXTRA_DECK || zone == domain::Zone::HAND)
		{
			// 返回卡组/额外卡组/手牌后不再视为正规召唤过
			instance.properly_summoned_ = false;
		}

		if (const auto zone = domain::zone_of(new_placement);
			zone != domain::Zone::MONSTER &&
			zone != domain::Zone::EXTRA_MONSTER &&
			zone != domain::Zone::SPELL_TRAP &&
			zone != domain::Zone::FIELD_SPELL)
		{
			// 移动到非场上区域重置状态

			// 重置不可攻击
			instance.cannot_attack_turn_.reset();
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

	auto CardInstance::Modifier::prevent_attack(const domain::TurnId turn_id) noexcept -> void
	{
		ref().cannot_attack_turn_ = turn_id;
	}

	auto CardInstance::Modifier::mark_attacked(const domain::TurnId turn_id) noexcept -> void
	{
		ref().attacked_turn_ = turn_id;
	}

	auto CardInstance::Modifier::set_move_provenance(const MoveProvenance& provenance) noexcept -> void
	{
		ref().move_ = provenance;
	}

	auto CardInstance::Modifier::set_summon_record(SummonRecord record) noexcept -> void
	{
		auto& instance = ref();

		instance.summon_ = std::move(record);
		instance.cannot_attack_turn_.reset();

		if (!instance.properly_summoned_)
		{
			instance.properly_summoned_ = [&] noexcept -> bool
			{
				const auto subtypes = instance.origin_card_subtypes();

				switch (instance.summon_.method)
				{
					case domain::SummonMethod::NORMAL:
					case domain::SummonMethod::TRIBUTE:
					case domain::SummonMethod::FLIP:
					{
						// 可以通常召唤的怪兽都视为正规召唤
						return true;
					}
					case domain::SummonMethod::RITUAL:
					{
						return subtypes.contains(domain::CardSubtype::RITUAL);
					}
					case domain::SummonMethod::FUSION:
					{
						return subtypes.contains(domain::CardSubtype::FUSION);
					}
					case domain::SummonMethod::SYNCHRO:
					{
						return subtypes.contains(domain::CardSubtype::SYNCHRO);
					}
					case domain::SummonMethod::XYZ:
					{
						return subtypes.contains(domain::CardSubtype::XYZ);
					}
					case domain::SummonMethod::PENDULUM:
					{
						// 可以灵摆召唤的怪兽都视为正规召唤
						return true;
					}
					case domain::SummonMethod::LINK:
					{
						return subtypes.contains(domain::CardSubtype::LINK);
					}
					case domain::SummonMethod::EFFECT_SPECIAL:
					{
						// 如果是特殊召唤,则不能是融合/仪式/同调/超量/连接子类型,否则视为非正规召唤
						constexpr utility::EnumMask excludes
						{
								domain::CardSubtype::FUSION,
								domain::CardSubtype::RITUAL,
								domain::CardSubtype::SYNCHRO,
								domain::CardSubtype::XYZ,
								domain::CardSubtype::LINK,
						};
						return subtypes.intersect(excludes).empty();
					}
					case domain::SummonMethod::RULE_SPECIAL:
					{
						// 规则特招总是视为正规召唤
						return true;
					}
					default: // NOLINT(clang-diagnostic-covered-switch-default)
					{
						std::unreachable();
					}
				}
			}();
		}
	}

	CardInstance::CardInstance(const CardInstanceId id, const domain::Player owner, const dsl::CardIdentity& definition) noexcept
		: definition_{definition},
		  id_{id},
		  owner_{owner},
		  controller_{owner},
		  // placement_{},
		  // overlays_{},
		  face_up_generation_{0},
		  properly_summoned_{false},
		  token_{false},
		  move_
		  {
				  //
		  },
		  summon_
		  {
				  //
		  } {}

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

	auto CardInstance::origin_card_subtypes() const noexcept -> utility::EnumMask<domain::CardSubtype>
	{
		return definition_.get().subtypes;
	}

	auto CardInstance::origin_is_card_subtype(const domain::CardSubtype subtype) const noexcept -> bool
	{
		return origin_card_subtypes().contains(subtype);
	}

	auto CardInstance::origin_is_card_subtypes(const utility::EnumMask<domain::CardSubtype> subtypes) const noexcept -> bool
	{
		return !origin_card_subtypes().intersect(subtypes).empty();
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

	auto CardInstance::origin_has_archetype(const dsl::ArchetypeId archetype) const noexcept -> bool
	{
		return std::ranges::contains(definition_.get().archetypes, archetype);
	}

	auto CardInstance::overlays() const noexcept -> const std::vector<CardInstanceId>&
	{
		return overlays_;
	}

	auto CardInstance::face_up_generation() const noexcept -> std::uint32_t
	{
		return face_up_generation_;
	}

	auto CardInstance::properly_summoned() const noexcept -> bool
	{
		return properly_summoned_;
	}

	auto CardInstance::is_token() const noexcept -> bool
	{
		return token_;
	}

	auto CardInstance::cannot_attack(const domain::TurnId turn_id) const noexcept -> bool
	{
		return cannot_attack_turn_.has_value() && *cannot_attack_turn_ == turn_id;
	}

	auto CardInstance::has_attacked(const domain::TurnId turn_id) const noexcept -> bool
	{
		return attacked_turn_.has_value() && *attacked_turn_ == turn_id;
	}

	auto CardInstance::move_provenance() const noexcept -> const MoveProvenance&
	{
		return move_;
	}

	auto CardInstance::summon_record() const noexcept -> const SummonRecord&
	{
		return summon_;
	}
}

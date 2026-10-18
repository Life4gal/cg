#include <cg/state/card.hpp>

namespace cg::state
{
	CardInstance::CardInstance(const CardInstanceId id, const CardDefinition& definition) noexcept
		: definition_{definition},
		  id_{id},
		  owner_{domain::Player::FIRST},
		  controller_{domain::Player::FIRST},
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
}

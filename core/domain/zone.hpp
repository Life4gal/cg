#pragma once

#include <variant>

#include <core/utility/enum.hpp>
#include <core/utility/functional.hpp>

namespace cg::domain
{
	class Zone
	{
	public:
		// 各个区域的卡数量不会超过255张
		using size_type = std::uint8_t;

		// =======================================================================

		enum class MonsterPosition : size_type
		{
			MIAN_1 = 0,
			MAIN_2 = 1,
			MAIN_3 = 2,
			MAIN_4 = 3,
			MAIN_5 = 4,
		};

		enum class ExtraMonsterPosition : size_type
		{
			EXTRA_1 = 0,
			EXTRA_2 = 1,
		};

		enum class MonsterForm : size_type
		{
			FACE_UP_ATTACK,
			FACE_DOWN_ATTACK,
			FACE_UP_DEFENSE,
			FACE_DOWN_DEFENSE,
		};

		enum class SpellTrapPosition : size_type
		{
			SPELL_TRAP_1 = 0,
			SPELL_TRAP_2 = 1,
			SPELL_TRAP_3 = 2,
			SPELL_TRAP_4 = 3,
			SPELL_TRAP_5 = 4,
			FIELD_SPELL = 5,
			PENDULUM_LEFT = SPELL_TRAP_1,
			PENDULUM_RIGHT = SPELL_TRAP_5,
		};

		enum class SpellTrapForm : size_type
		{
			FACE_UP,
			FACE_DOWN,
		};

		constexpr static auto monster_main_count = static_cast<std::size_t>(5);
		constexpr static auto monster_extra_count = static_cast<std::size_t>(2);
		constexpr static auto spell_trap_count = static_cast<std::size_t>(6);

		// =======================================================================

		// 卡组
		class Deck
		{
		public:
			size_type index;

			[[nodiscard]] constexpr auto operator==(const Deck& other) const noexcept -> bool = default;
		};

		// 额外卡组
		class ExtraDeck
		{
		public:
			size_type index;

			[[nodiscard]] constexpr auto operator==(const ExtraDeck& other) const noexcept -> bool = default;
		};

		// 手牌
		class Hand
		{
		public:
			size_type index;

			[[nodiscard]] constexpr auto operator==(const Hand& other) const noexcept -> bool = default;
		};

		// 墓地
		class Graveyard
		{
		public:
			size_type index;

			[[nodiscard]] constexpr auto operator==(const Graveyard& other) const noexcept -> bool = default;
		};

		// 除外区
		class Removed
		{
		public:
			size_type index;

			[[nodiscard]] constexpr auto operator==(const Removed& other) const noexcept -> bool = default;
		};

		// 超量素材
		class Overlay
		{
		public:
			size_type index;

			[[nodiscard]] constexpr auto operator==(const Overlay& other) const noexcept -> bool = default;
		};

		// 怪兽区域
		class Monster
		{
		public:
			MonsterPosition position;
			MonsterForm form;

			[[nodiscard]] constexpr auto operator==(const Monster& other) const noexcept -> bool = default;
		};

		// 额外怪兽区
		class ExtraMonster
		{
		public:
			ExtraMonsterPosition position;
			MonsterForm form;

			[[nodiscard]] constexpr auto operator==(const ExtraMonster& other) const noexcept -> bool = default;
		};

		// 魔陷区域
		class SpellTrap
		{
		public:
			SpellTrapPosition position;
			SpellTrapForm form;

			[[nodiscard]] constexpr auto operator==(const SpellTrap& other) const noexcept -> bool = default;
		};

		// 不存在区域
		class NotExist
		{
		public:
			[[nodiscard]] constexpr auto operator==(const NotExist& other) const noexcept -> bool = default;
		};

		using zone_type = std::variant<
			// 卡组
			Deck,
			// 额外卡组
			ExtraDeck,
			// 手牌
			Hand,
			// 墓地
			Graveyard,
			// 除外区
			Removed,
			// 超量素材
			Overlay,
			// 怪兽区域
			Monster,
			// 额外怪兽区
			ExtraMonster,
			// 魔陷区域
			SpellTrap,
			// 不存在区域
			NotExist
			//
		>;

		constexpr static NotExist not_exist{};

	private:
		zone_type zone_;

	public:
		Zone() noexcept
			: zone_{not_exist} {}

		template<typename Z>
		constexpr explicit(false) Zone(Z z) noexcept
			requires std::is_constructible_v<zone_type, Z>
			: zone_{z} {}

		[[nodiscard]] constexpr auto operator==(const Zone& other) const noexcept -> bool = default;

		// 检查是否在指定区域
		template<typename Zone>
		[[nodiscard]] constexpr auto at() const noexcept -> bool
		{
			return std::holds_alternative<Zone>(zone_);
		}

		// 获取指定区域的数据
		template<typename Zone>
		[[nodiscard]] constexpr auto get() const noexcept -> auto
		{
			return std::get<Zone>(zone_);
		}

		// visit
		template<typename Visitor>
		constexpr auto visit(Visitor visitor) const noexcept -> auto //
			requires requires { std::visit(visitor, zone_); }
		{
			return std::visit(visitor, zone_);
		}

		// 是否在怪兽区域
		[[nodiscard]] constexpr auto is_monster_zone() const noexcept -> bool
		{
			return at<Monster>() || at<ExtraMonster>();
		}

		// 是否在场上区域
		[[nodiscard]] constexpr auto is_field_zone() const noexcept -> bool
		{
			return is_monster_zone() || at<SpellTrap>();
		}

	private:
		[[nodiscard]] constexpr static auto is_face_up_form(const MonsterForm form) noexcept -> bool
		{
			return form == MonsterForm::FACE_UP_ATTACK || form == MonsterForm::FACE_UP_DEFENSE;
		}

		[[nodiscard]] constexpr static auto is_face_down_form(const MonsterForm form) noexcept -> bool
		{
			return form == MonsterForm::FACE_DOWN_ATTACK || form == MonsterForm::FACE_DOWN_DEFENSE;
		}

		[[nodiscard]] constexpr static auto is_attack_form(const MonsterForm form) noexcept -> bool
		{
			return form == MonsterForm::FACE_UP_ATTACK || form == MonsterForm::FACE_DOWN_ATTACK;
		}

		[[nodiscard]] constexpr static auto is_defense_form(const MonsterForm form) noexcept -> bool
		{
			return form == MonsterForm::FACE_UP_DEFENSE || form == MonsterForm::FACE_DOWN_DEFENSE;
		}

		[[nodiscard]] constexpr auto check_form(bool (*checker)(MonsterForm form) noexcept) const noexcept -> bool
		{
			if (!is_monster_zone())
			{
				return false;
			}

			return std::visit(
				utility::Overloaded
				{
						[checker](const Monster& m) noexcept -> bool
						{
							return checker(m.form);
						},
						[checker](const ExtraMonster& em) noexcept -> bool
						{
							return checker(em.form);
						},
						[]([[maybe_unused]] const auto& unhandled) noexcept -> bool
						{
							return false;
						}
				},
				zone_
			);
		}

	public:
		// 是否(在怪兽区域且)表侧表示
		[[nodiscard]] constexpr auto is_face_up_form() const noexcept -> bool
		{
			return check_form(&Zone::is_face_up_form);
		}

		// 是否(在怪兽区域且)里侧表示
		[[nodiscard]] constexpr auto is_face_down_form() const noexcept -> bool
		{
			return check_form(&Zone::is_face_down_form);
		}

		// 是否(在怪兽区域且)攻击表示
		[[nodiscard]] constexpr auto is_attack_form() const noexcept -> bool
		{
			return check_form(&Zone::is_attack_form);
		}

		// 是否(在怪兽区域且)防御表示
		[[nodiscard]] constexpr auto is_defense_form() const noexcept -> bool
		{
			return check_form(&Zone::is_defense_form);
		}
	};

	// 选卡区域
	enum class SelectZone : std::uint16_t
	{
		NONE = 0,

		// 卡组
		DECK = 1 << 0,
		// 手牌
		HAND = 1 << 1,
		// 怪兽区域
		MONSTER = 1 << 2,
		// 额外怪兽区
		EXTRA_MONSTER = 1 << 3,
		// 魔陷区域
		SPELL_TRAP = 1 << 4,
		// 墓地
		GRAVEYARD = 1 << 5,
		// 除外区
		REMOVED = 1 << 6,
		// 额外卡组
		EXTRA_DECK = 1 << 7,
		// 超量素材
		OVERLAY = 1 << 8,
		// // 灵摆区域
		// PENDULUM = 1 << 9,
		// // 场地区域
		// FIELD_SPELL = 1 << 10,
	};

	class SelectZoneWrapper : public utility::Enum<
				SelectZone,
				// ==, !=
				utility::EnumSupport::EQUALITY,
				// &, &=, |, |=, ^, ^=, ~
				utility::EnumSupport::BIT_OPERATION
			> {};

	// 场上区域
	[[nodiscard]] constexpr auto is_field_zone(const SelectZoneWrapper zone) noexcept -> bool
	{
		constexpr static auto field = []
		{
			SelectZoneWrapper w{SelectZone::NONE};

			// 怪兽区域
			w |= SelectZone::MONSTER;
			// 额外怪兽区
			w |= SelectZone::EXTRA_MONSTER;
			// 魔陷区域
			w |= SelectZone::SPELL_TRAP;

			return w;
		}();

		return zone.any_of(field);
	}

	// 返回卡组时返回的位置
	enum class DeckPlacement : std::uint8_t
	{
		// 放在卡组顶端
		TOP,
		// 放在卡组底端
		BOTTOM,
		// 加入卡组后洗切
		SHUFFLE,
	};
}

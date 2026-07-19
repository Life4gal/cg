#pragma once

#include <vector>

#include <core/utility/flag.hpp>

#include <core/card_data.hpp>

namespace cg
{
	class Card;
	class Effect;

	// 卡片瞬时状态
	class CardState
	{
	public:
		using code_type = CardData::code_type;
		using property_value_type = std::uint32_t;

		// 无效卡密
		constexpr static code_type invalid_code = CardData::invalid_code;

		// 8位卡片密码,那些"卡名当作XXX"之类的效果会修改该值
		code_type code;
		// 第二卡密,如"鹰身女郎"同时有多个code
		code_type code2;

		// 卡片类型,那些"当作调整使用"之类的效果会修改该值
		CardType type;
		// 卡片字段,那些"卡名当作XXX"之类的效果会修改该值
		std::vector<CardArchetype> archetypes;

		// 属性,可被"DNA移植手术"之类的卡片修改
		MonsterAttribute attribute;
		// 种族,可被"DNA改造手术"之类的卡片修改
		MonsterRace race;
		// 等级,可被"等级+1/-1"/"等级视为X"之类的效果修改
		MonsterLevel level;
		// 阶级(存在修改阶级的卡片吗?)
		MonsterRank rank;
		// 左侧灵摆刻度,可被"那张卡的灵摆刻度视为X"之类的效果修改
		MonsterPendulum left_pendulum;
		// 右侧灵摆刻度,可被"那张卡的灵摆刻度视为X"之类的效果修改
		MonsterPendulum right_pendulum;
		// 链接箭头(存在修改链接箭头的卡片吗?)
		MonsterLinkMarker link_marker;

		// 攻击力(当前)
		property_value_type attack;
		// 攻击力(原始)
		property_value_type original_attack;
		// 守备力(当前)
		property_value_type defense;
		// 守备力(原始)
		property_value_type original_defense;

		// ====================================

		// 当前控制者,可被"洗脑"之类的卡片修改
		Player controller;
		// 当前位置
		CardLocation location;
		// 当前场上位置
		CardZone zone;
		// 当前表示形式
		CardForm form;

		// 当前状态改变原因(进入此状态的原因,例如被解放/被战斗破坏)
		CardReason reason;
		// 造成状态改变的玩家
		Player reason_player;
		// 造成状态改变的卡片
		Card* reason_card;
		// 造成状态改变的效果
		Effect* reason_effect;

		constexpr CardState() noexcept
			: code{invalid_code},
			  code2{invalid_code},
			  type{CardType::TOKEN},
			  attribute{MonsterAttribute::EARTH},
			  race{MonsterRace::WARRIOR},
			  level{MonsterLevel::L0},
			  rank{MonsterRank::R0},
			  left_pendulum{MonsterPendulum::P0},
			  right_pendulum{MonsterPendulum::P0},
			  link_marker{MonsterLinkMarker::TOP_LEFT},
			  attack{0},
			  original_attack{0},
			  defense{0},
			  original_defense{0},
			  controller{Player::UNKNOWN},
			  location{CardLocation::NONE},
			  zone{CardZone::NONE},
			  form{CardForm::NONE},
			  reason{CardReason::RULE},
			  reason_player{Player::UNKNOWN},
			  reason_card{nullptr},
			  reason_effect{nullptr}
		{
			//
		}

	private:
		class Helper
		{
		protected:
			friend CardState;

			std::reference_wrapper<const CardState> state_;

			constexpr explicit Helper(const CardState& state) noexcept
				: state_{state} {}
		};

	public:
		// --------------------------------------------------------------------------
		// CardLocation
		// --------------------------------------------------------------------------

		class Location : public Helper
		{
			using Helper::Helper;

		public:
			[[nodiscard]] constexpr auto in(const CardLocation required) const noexcept -> bool
			{
				return (state_.get().location & required) != CardLocation::NONE;
			}

			[[nodiscard]] constexpr auto monster() const noexcept -> bool
			{
				return in(CardLocation::MONSTER_ZONE);
			}

			[[nodiscard]] constexpr auto spell() const noexcept -> bool
			{
				return in(CardLocation::SPELL_ZONE);
			}

			[[nodiscard]] constexpr auto field() const noexcept -> bool
			{
				return in(CardLocation::FIELD);
			}

			[[nodiscard]] constexpr auto hand() const noexcept -> bool
			{
				return in(CardLocation::HAND);
			}

			[[nodiscard]] constexpr auto graveyard() const noexcept -> bool
			{
				return in(CardLocation::GRAVEYARD);
			}

			[[nodiscard]] constexpr auto removed() const noexcept -> bool
			{
				return in(CardLocation::REMOVED);
			}

			[[nodiscard]] constexpr auto overlay() const noexcept -> bool
			{
				return in(CardLocation::OVERLAY);
			}
		};

		[[nodiscard]] constexpr auto get_location() const noexcept -> Location
		{
			return Location{*this};
		}

		// --------------------------------------------------------------------------
		// CardZone
		// --------------------------------------------------------------------------

		class Zone : public Helper
		{
			using Helper::Helper;

			[[nodiscard]] constexpr auto is(auto (Location::*p)() const noexcept -> bool) const noexcept -> bool
			{
				return (state_.get().get_location().*p)();
			}

		public:
			[[nodiscard]] constexpr auto in(const CardZone required) const noexcept -> bool
			{
				return (state_.get().zone & required) != CardZone::NONE;
			}

			[[nodiscard]] constexpr auto main_monster() const noexcept -> bool
			{
				if (not is(&Location::monster))
				{
					return false;
				}

				return in(CardZone::MONSTER_MAIN_ZONE);
			}

			[[nodiscard]] constexpr auto extra_monster() const noexcept -> bool
			{
				if (not is(&Location::monster))
				{
					return false;
				}

				return in(CardZone::MONSTER_EXTRA_ZONE);
			}

			[[nodiscard]] constexpr auto monster() const noexcept -> bool
			{
				if (not is(&Location::monster))
				{
					return false;
				}

				return in(CardZone::MONSTER_ZONE);
			}

			[[nodiscard]] constexpr auto spell() const noexcept -> bool
			{
				if (not is(&Location::spell))
				{
					return false;
				}

				return in(CardZone::SPELL_ZONE);
			}

			[[nodiscard]] constexpr auto pendulum() const noexcept -> bool
			{
				if (not is(&Location::spell))
				{
					return false;
				}

				return in(CardZone::SPELL_PENDULUM_ZONE);
			}

			[[nodiscard]] constexpr auto field() const noexcept -> bool
			{
				if (not is(&Location::spell))
				{
					return false;
				}

				return in(CardZone::SPELL_FIELD_ZONE);
			}
		};

		[[nodiscard]] constexpr auto get_zone() const noexcept -> Zone
		{
			return Zone{*this};
		}

		// --------------------------------------------------------------------------
		// CardForm
		// --------------------------------------------------------------------------

		class Form : public Helper
		{
			using Helper::Helper;

		public:
			[[nodiscard]] constexpr auto is(const CardForm required) const noexcept -> bool
			{
				return (state_.get().form & required) != CardForm::NONE;
			}

			[[nodiscard]] constexpr auto face_up_attack() const noexcept -> bool
			{
				return is(CardForm::FACE_UP_ATTACK);
			}

			[[nodiscard]] constexpr auto face_down_attack() const noexcept -> bool
			{
				return is(CardForm::FACE_DOWN_ATTACK);
			}

			[[nodiscard]] constexpr auto face_up_defense() const noexcept -> bool
			{
				return is(CardForm::FACE_UP_DEFENSE);
			}

			[[nodiscard]] constexpr auto face_down_defense() const noexcept -> bool
			{
				return is(CardForm::FACE_DOWN_DEFENSE);
			}

			[[nodiscard]] constexpr auto is_face_up() const noexcept -> bool
			{
				return is(CardForm::FACE_UP);
			}

			[[nodiscard]] constexpr auto is_face_down() const noexcept -> bool
			{
				return is(CardForm::FACE_DOWN);
			}

			[[nodiscard]] constexpr auto is_attack() const noexcept -> bool
			{
				return is(CardForm::ATTACK);
			}

			[[nodiscard]] constexpr auto is_defense() const noexcept -> bool
			{
				return is(CardForm::DEFENSE);
			}
		};

		[[nodiscard]] constexpr auto get_form() const noexcept -> Form
		{
			return Form{*this};
		}
	};
}

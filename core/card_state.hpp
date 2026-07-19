#pragma once

#include <vector>

#include <core/card_data.hpp>
#include <core/card_location.hpp>

namespace cg
{
	// 玩家
	enum class Player : std::uint8_t
	{
		UNKNOWN = 0,
		PLAYER_0,
		PLAYER_1,
	};

	// 卡片状态改变原因
	enum class CardReason : std::uint32_t
	{
		RULE = 0,

		// TODO
	};

	class Card;
	class CardEffect;

	// 卡片瞬时状态
	class CardState
	{
	public:
		using code_type = CardData::code_type;

		// 无效卡密
		constexpr static code_type invalid_code = CardData::invalid_code;

		class Uninitialized {};

		class Monster
		{
		public:
			using property_value_type = CardData::Monster::property_value_type;

			// 类型,那些"当作调整使用"之类的效果会修改该值
			MonsterCategory category;

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
		};

		class Spell
		{
		public:
			// 类型
			SpellCategory category;
		};

		class Trap
		{
		public:
			// 类型
			TrapCategory category;
		};

		using data_type = std::variant<Uninitialized, Monster, Spell, Trap>;
		using archetypes_type = std::vector<CardArchetype>;

		// 8位卡片密码,那些"卡名当作XXX"之类的效果会修改该值
		code_type code;
		// 第二卡密,如"鹰身女郎"同时有多个code
		code_type code2;

		// 卡片类型+数据
		data_type data;
		// 卡片字段
		archetypes_type archetypes;

		// 当前控制者,可被"洗脑"之类的卡片修改
		Player controller;
		// 当前位置
		CardLocation location;

		// 进入此状态的原因,例如被解放/被战斗破坏
		CardReason reason;
		// 造成状态改变的玩家
		Player reason_player;
		// 造成状态改变的卡片
		Card* reason_card;
		// 造成状态改变的效果
		CardEffect* reason_effect;

		constexpr CardState() noexcept
			: code{invalid_code},
			  code2{invalid_code},
			  data{Uninitialized{}},
			  controller{Player::UNKNOWN},
			  // location{},
			  reason{CardReason::RULE},
			  reason_player{Player::UNKNOWN},
			  reason_card{nullptr},
			  reason_effect{nullptr}
		{
			//
		}
	};
}

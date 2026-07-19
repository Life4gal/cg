#pragma once

#include <variant>

namespace cg
{
	// 卡片场上表示形式
	enum class CardFieldForm : std::uint8_t
	{
		// ====================================

		// 表侧表示
		INTERNAL_FACE_UP = 1 << 0,
		// 里侧表示
		INTERNAL_FACE_DOWN = 1 << 1,
		// 攻击表示
		INTERNAL_ATTACK = 1 << 2,
		// 守备表示
		INTERNAL_DEFENSE = 1 << 3,

		// ====================================

		// 表侧攻击
		FACE_UP_ATTACK = INTERNAL_FACE_UP | INTERNAL_ATTACK,
		// 里侧攻击
		FACE_DOWN_ATTACK = INTERNAL_FACE_DOWN | INTERNAL_ATTACK,
		// 表侧守备
		FACE_UP_DEFENSE = INTERNAL_FACE_UP | INTERNAL_DEFENSE,
		// 里侧守备
		FACE_DOWN_DEFENSE = INTERNAL_FACE_DOWN | INTERNAL_DEFENSE,

		// 表侧攻击或是守备
		FACE_UP = FACE_UP_ATTACK | FACE_UP_DEFENSE,
		// 里侧攻击或是守备
		FACE_DOWN = FACE_DOWN_ATTACK | FACE_DOWN_DEFENSE,
		// 表侧或是里侧攻击
		ATTACK = FACE_UP_ATTACK | FACE_DOWN_ATTACK,
		// 表侧或是里侧守备
		DEFENSE = FACE_UP_DEFENSE | FACE_DOWN_DEFENSE,
	};

	// 卡片位置
	class CardLocation
	{
	public:
		using index_type = std::uint8_t;

		// 在主卡组
		class MainDeck
		{
		public:
			index_type index;
		};

		// 在额外卡组
		class ExtraDeck
		{
		public:
			index_type index;
		};

		// 在怪兽区域
		class MonsterZone
		{
		public:
			// -------------------------------------
			//     |5|     |6|
			// |0| |1| |2| |3| |4|
			// -------------------------------------

			enum class Index : index_type
			{
				Z0 = 0,
				Z1 = 1,
				Z2 = 2,
				Z3 = 3,
				Z4 = 4,
				Z5 = 5,
				Z6 = 6
			};

			Index index;
			CardFieldForm form;
		};

		// 在魔陷区域
		class SpellZone
		{
		public:
			// -------------------------------------
			// |5|
			//     |0| |1| |2| |3| |4|
			// -------------------------------------

			enum class Index : index_type
			{
				Z0 = 0,
				Z1 = 1,
				Z2 = 2,
				Z3 = 3,
				Z4 = 4,
				Z5 = 5,
			};

			Index index;
		};

		// 在手上
		class Hand
		{
		public:
			index_type index;
		};

		// 在墓地
		class Graveyard
		{
		public:
			index_type index;
		};

		// 在除外区
		class Removed
		{
		public:
			index_type index;
		};

		// 作为超量素材
		class Xyz
		{
		public:
			index_type index;
		};

		using location_type = std::variant<MainDeck, ExtraDeck, MonsterZone, SpellZone, Hand, Graveyard, Removed, Xyz>;

		location_type location;

		template<typename T>
			requires requires { std::holds_alternative<T>(location); }
		[[nodiscard]] constexpr auto in() const noexcept -> bool
		{
			return std::holds_alternative<T>(location);
		}
	};
}

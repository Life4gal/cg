#pragma once

#include <array>
#include <variant>

namespace cg::domain
{
	// 卡牌在场地上的位置
	// 场地魔法区域单独表示,不在此列
	enum class FieldColumn : std::uint8_t
	{
		// 列N = 区域列表下标
		C1 = 0,
		C2 = 1,
		C3 = 2,
		C4 = 3,
		C5 = 4,

		// 额外怪兽区1 = 列2
		EX1 = C2,
		// 额外怪兽区2 = 列4
		EX2 = C4,
		// 灵摆区-左 = 列1
		PENDULUM_LEFT = C1,
		// 灵摆区-右 = 列5
		PENDULUM_RIGHT = C5,
	};


	constexpr std::array field_columns
	{
			FieldColumn::C1,
			FieldColumn::C2,
			FieldColumn::C3,
			FieldColumn::C4,
			FieldColumn::C5,
	};
	constexpr auto total_field_columns = field_columns.size();

	constexpr std::array extra_columns
	{
			FieldColumn::EX1,
			FieldColumn::EX2,
	};
	constexpr auto total_extra_columns = extra_columns.size();

	// 指定列对应的对方列(同一绝对列)
	[[nodiscard]] constexpr auto opponent_column(const FieldColumn column) noexcept -> FieldColumn
	{
		const auto c = std::to_underlying(column);
		const auto opponent_c = total_field_columns - 1 - c;

		return static_cast<FieldColumn>(opponent_c);
	}

	// 卡牌表示形式
	enum class Position : std::uint8_t
	{
		// 表侧+攻击表示
		FACE_UP_ATTACK,
		// 表侧+守备表示
		FACE_UP_DEFENSE,
		// 里侧+攻击表示 -- 不存在
		// FACE_DOWN_ATTACK,
		// 里侧+守备表示
		FACE_DOWN_DEFENSE,
	};

	// ==============================================================
	// 卡牌所在区域
	// ==============================================================

	enum class Zone : std::uint8_t
	{
		// 不存在区域
		NOT_EXIST,
		// 卡组
		DECK,
		// 额外卡组
		EXTRA_DECK,
		// 手牌
		HAND,
		// 墓地
		GRAVEYARD,
		// 除外区
		BANISHED,
		// 怪兽区
		MONSTER,
		// 额外怪兽区
		EXTRA_MONSTER,
		// 魔陷区
		SPELL_TRAP,
		// 场地魔法区
		FIELD_SPELL,
	};

	// ==============================================================
	// 卡牌所在位置
	// ==============================================================

	// 不存在区域
	class NotExistLocation
	{
	public:
		[[nodiscard]] constexpr auto operator==(const NotExistLocation& other) const noexcept -> bool = default;
	};

	// 卡组
	class DeckLocation
	{
	public:
		// 卡数量不会超过255张
		using size_type = std::uint8_t;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const DeckLocation& other) const noexcept -> bool = default;
	};

	// 额外卡组
	class ExtraDeckLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const ExtraDeckLocation& other) const noexcept -> bool = default;
	};

	// 手牌
	class HandLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const HandLocation& other) const noexcept -> bool = default;
	};

	// 墓地
	class GraveyardLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const GraveyardLocation& other) const noexcept -> bool = default;
	};

	// 除外区
	class BanishedLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const BanishedLocation& other) const noexcept -> bool = default;
	};

	// 怪兽区
	class MonsterLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		// 所在列
		FieldColumn column;

		[[nodiscard]] constexpr auto operator==(const MonsterLocation& other) const noexcept -> bool = default;
	};

	// 额外怪兽区
	class ExtraMonsterLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		// 所在列
		// 必须是EX列
		FieldColumn column;

		[[nodiscard]] constexpr auto operator==(const ExtraMonsterLocation& other) const noexcept -> bool = default;
	};

	// 魔陷区
	class SpellTrapLocation
	{
	public:
		using size_type = DeckLocation::size_type;

		// 所在列
		FieldColumn column;

		[[nodiscard]] constexpr auto operator==(const SpellTrapLocation& other) const noexcept -> bool = default;
	};

	// 场地魔法区
	class FieldSpellLocation
	{
	public:
		[[nodiscard]] constexpr auto operator==(const FieldSpellLocation& other) const noexcept -> bool = default;
	};

	class Location : public std::variant<
				// 不存在区域
				NotExistLocation,
				// 卡组
				DeckLocation,
				// 额外卡组
				ExtraDeckLocation,
				// 手牌
				HandLocation,
				// 墓地
				GraveyardLocation,
				// 除外区
				BanishedLocation,
				// 怪兽区
				MonsterLocation,
				// 额外怪兽区
				ExtraMonsterLocation,
				// 魔陷区
				SpellTrapLocation,
				// 场地魔法区
				FieldSpellLocation
				//
			> {};

	class FieldLocation : public std::variant<
				// 怪兽区
				MonsterLocation,
				// 额外怪兽区
				ExtraMonsterLocation,
				// 魔陷区
				SpellTrapLocation,
				// 场地魔法区
				FieldSpellLocation
				//
			> {};

	class AnyMonsterLocation : public std::variant<
				// 怪兽区
				MonsterLocation,
				// 额外怪兽区
				ExtraMonsterLocation
				//
			> {};

	// ==============================================================
	// 卡牌当前形态
	// ==============================================================

	// 不存在区域
	class NotExistPlacement
	{
	public:
		[[nodiscard]] constexpr auto operator==(const NotExistPlacement& other) const noexcept -> bool = default;
	};

	// 卡组
	class DeckPlacement
	{
	public:
		using size_type = DeckLocation::size_type;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const DeckPlacement& other) const noexcept -> bool = default;
	};

	// 额外卡组
	class ExtraDeckPlacement
	{
	public:
		using size_type = DeckPlacement::size_type;

		size_type index;
		// 只有表侧/里侧,无视攻击/守备
		Position position;

		[[nodiscard]] constexpr auto operator==(const ExtraDeckPlacement& other) const noexcept -> bool = default;
	};

	// 手牌
	class HandPlacement
	{
	public:
		using size_type = DeckPlacement::size_type;

		size_type index;
		// 只有表侧/里侧,无视攻击/守备
		Position position;

		[[nodiscard]] constexpr auto operator==(const HandPlacement& other) const noexcept -> bool = default;
	};

	// 墓地
	class GraveyardPlacement
	{
	public:
		using size_type = DeckPlacement::size_type;

		size_type index;

		[[nodiscard]] constexpr auto operator==(const GraveyardPlacement& other) const noexcept -> bool = default;
	};

	// 除外区
	class BanishedPlacement
	{
	public:
		using size_type = DeckPlacement::size_type;

		size_type index;
		// 只有表侧/里侧,无视攻击/守备
		Position position;

		[[nodiscard]] constexpr auto operator==(const BanishedPlacement& other) const noexcept -> bool = default;
	};

	// 怪兽区
	class MonsterPlacement
	{
	public:
		FieldColumn column;
		Position position;

		[[nodiscard]] constexpr auto operator==(const MonsterPlacement& other) const noexcept -> bool = default;
	};

	// 额外怪兽区
	class ExtraMonsterPlacement
	{
	public:
		using size_type = DeckPlacement::size_type;

		FieldColumn column;
		// 连接怪兽只能是表侧+攻击表示
		Position position;

		[[nodiscard]] constexpr auto operator==(const ExtraMonsterPlacement& other) const noexcept -> bool = default;
	};

	// 魔陷区
	class SpellTrapPlacement
	{
	public:
		FieldColumn column;
		// 只有表侧/里侧,无视攻击/守备
		Position position;

		[[nodiscard]] constexpr auto operator==(const SpellTrapPlacement& other) const noexcept -> bool = default;
	};

	// 场地魔法区
	class FieldSpellPlacement
	{
	public:
		// 只有表侧/里侧,无视攻击/守备
		Position position;

		[[nodiscard]] constexpr auto operator==(const FieldSpellPlacement& other) const noexcept -> bool = default;
	};

	class Placement : public std::variant<
				// 不存在区域
				NotExistPlacement,
				// 卡组
				DeckPlacement,
				// 额外卡组
				ExtraDeckPlacement,
				// 手牌
				HandPlacement,
				// 墓地
				GraveyardPlacement,
				// 除外区
				BanishedPlacement,
				// 怪兽区域
				MonsterPlacement,
				// 额外怪兽区
				ExtraMonsterPlacement,
				// 魔陷区域
				SpellTrapPlacement,
				// 场地魔法区
				FieldSpellPlacement
				//
			> {};

	class FieldPlacement : public std::variant<
				// 怪兽区
				MonsterPlacement,
				// 额外怪兽区
				ExtraMonsterPlacement,
				// 魔陷区
				SpellTrapPlacement,
				// 场地魔法区
				FieldSpellPlacement
				//
			> {};

	class AnyMonsterPlacement : public std::variant<
				// 怪兽区
				MonsterPlacement,
				// 额外怪兽区
				ExtraMonsterPlacement
				//
			> {};
}

#pragma once

#include <array>
#include <variant>
#include <optional>

#include <cg/utility/functional.hpp>

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

	class FieldInfo
	{
	public:
		FieldColumn column;
		Position position;
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

	[[nodiscard]] constexpr auto index_of(const Location& location) noexcept -> std::optional<DeckLocation::size_type>
	{
		return std::visit(
			utility::Overloaded
			{
					[]<typename L>(const L& l) noexcept -> std::optional<DeckLocation::size_type> //
						requires(
							std::is_same_v<L, DeckLocation> ||
							std::is_same_v<L, ExtraDeckLocation> ||
							std::is_same_v<L, HandLocation> ||
							std::is_same_v<L, GraveyardLocation> ||
							std::is_same_v<L, BanishedLocation>
						)
					{
						return l.index;
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> std::optional<DeckLocation::size_type>
					{
						return std::nullopt;
					}
			},
			location
		);
	}

	[[nodiscard]] constexpr auto column_of(const Location& location) noexcept -> std::optional<FieldColumn>
	{
		return std::visit(
			utility::Overloaded
			{
					[]<typename L>(const L& l) noexcept -> std::optional<FieldColumn> //
						requires(
							std::is_same_v<L, MonsterLocation> ||
							std::is_same_v<L, ExtraMonsterLocation> ||
							std::is_same_v<L, SpellTrapLocation>
						)
					{
						return l.column;
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> std::optional<FieldColumn>
					{
						return std::nullopt;
					}
			},
			location
		);
	}

	[[nodiscard]] constexpr auto column_of(const FieldLocation& location) noexcept -> std::optional<FieldColumn>
	{
		return std::visit(
			utility::Overloaded
			{
					[]<typename L>(const L& l) noexcept -> std::optional<FieldColumn> //
						requires(
							std::is_same_v<L, MonsterLocation> ||
							std::is_same_v<L, ExtraMonsterLocation> ||
							std::is_same_v<L, SpellTrapLocation>
						)
					{
						return l.column;
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> std::optional<FieldColumn>
					{
						return std::nullopt;
					}
			},
			location
		);
	}

	[[nodiscard]] constexpr auto column_of(const AnyMonsterLocation& location) noexcept -> FieldColumn
	{
		return std::visit(
			[](const auto& l) noexcept -> FieldColumn
			{
				return l.column;
			},
			location
		);
	}

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

	[[nodiscard]] constexpr auto index_of(const Placement& placement) noexcept -> std::optional<DeckPlacement::size_type>
	{
		return std::visit(
			utility::Overloaded
			{
					[]<typename P>(const P& p) noexcept -> std::optional<DeckPlacement::size_type> //
						requires(
							std::is_same_v<P, DeckPlacement> ||
							std::is_same_v<P, ExtraDeckPlacement> ||
							std::is_same_v<P, HandPlacement> ||
							std::is_same_v<P, GraveyardPlacement> ||
							std::is_same_v<P, BanishedPlacement>
						)
					{
						return p.index;
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> std::optional<DeckPlacement::size_type>
					{
						return std::nullopt;
					}
			},
			placement
		);
	}

	[[nodiscard]] constexpr auto column_of(const Placement& placement) noexcept -> std::optional<FieldColumn>
	{
		return std::visit(
			utility::Overloaded
			{
					[]<typename P>(const P& p) noexcept -> std::optional<FieldColumn> //
						requires(
							std::is_same_v<P, MonsterPlacement> ||
							std::is_same_v<P, ExtraMonsterPlacement> ||
							std::is_same_v<P, SpellTrapPlacement>
						)
					{
						return p.column;
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> std::optional<FieldColumn>
					{
						return std::nullopt;
					}
			},
			placement
		);
	}

	[[nodiscard]] constexpr auto column_of(const FieldPlacement& placement) noexcept -> std::optional<FieldColumn>
	{
		return std::visit(
			utility::Overloaded
			{
					[]<typename P>(const P& p) noexcept -> std::optional<FieldColumn> //
						requires(
							std::is_same_v<P, MonsterPlacement> ||
							std::is_same_v<P, ExtraMonsterPlacement> ||
							std::is_same_v<P, SpellTrapPlacement>
						)
					{
						return p.column;
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> std::optional<FieldColumn>
					{
						return std::nullopt;
					}
			},
			placement
		);
	}

	[[nodiscard]] constexpr auto column_of(const AnyMonsterPlacement& placement) noexcept -> FieldColumn
	{
		return std::visit(
			[](const auto& p) noexcept -> FieldColumn
			{
				return p.column;
			},
			placement
		);
	}

	[[nodiscard]] constexpr auto field_info_of(const Placement& placement) noexcept -> std::optional<FieldInfo>
	{
		return std::visit(
			utility::Overloaded
			{
					[]<typename P>(const P& p) noexcept -> std::optional<FieldInfo> //
						requires(
							std::is_same_v<P, MonsterPlacement> ||
							std::is_same_v<P, ExtraMonsterPlacement> ||
							std::is_same_v<P, SpellTrapPlacement>
						)
					{
						return FieldInfo{.column = p.column, .position = p.position};
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> std::optional<FieldInfo>
					{
						return std::nullopt;
					}
			},
			placement
		);
	}

	[[nodiscard]] constexpr auto field_info_of(const FieldPlacement& placement) noexcept -> std::optional<FieldInfo>
	{
		return std::visit(
			utility::Overloaded
			{
					[]<typename P>(const P& p) noexcept -> std::optional<FieldInfo> //
						requires(
							std::is_same_v<P, MonsterPlacement> ||
							std::is_same_v<P, ExtraMonsterPlacement> ||
							std::is_same_v<P, SpellTrapPlacement>
						)
					{
						return FieldInfo{.column = p.column, .position = p.position};
					},
					[]([[maybe_unused]] const auto& unhandled) noexcept -> std::optional<FieldInfo>
					{
						return std::nullopt;
					}
			},
			placement
		);
	}

	[[nodiscard]] constexpr auto field_info_of(const AnyMonsterPlacement& placement) noexcept -> FieldInfo
	{
		return std::visit(
			[](const auto& p) noexcept -> FieldInfo
			{
				return {.column = p.column, .position = p.position};
			},
			placement
		);
	}

	[[nodiscard]] constexpr auto location_of(const Placement& placement) noexcept -> Location
	{
		return std::visit(
			utility::Overloaded
			{
					[&]([[maybe_unused]] const DeckPlacement& p) noexcept -> Location
					{
						return {DeckLocation{.index = p.index}};
					},
					[&]([[maybe_unused]] const ExtraDeckPlacement& p) noexcept -> Location
					{
						return {ExtraDeckLocation{.index = p.index}};
					},
					[&]([[maybe_unused]] const HandPlacement& p) noexcept -> Location
					{
						return {HandLocation{.index = p.index}};
					},
					[&]([[maybe_unused]] const GraveyardPlacement& p) noexcept -> Location
					{
						return {GraveyardLocation{.index = p.index}};
					},
					[&]([[maybe_unused]] const BanishedPlacement& p) noexcept -> Location
					{
						return {BanishedLocation{.index = p.index}};
					},
					[&](const MonsterPlacement& p) noexcept -> Location
					{
						return {MonsterLocation{.column = p.column}};
					},
					[&](const ExtraMonsterPlacement& p) noexcept -> Location
					{
						return {ExtraMonsterLocation{.column = p.column}};
					},
					[&](const SpellTrapPlacement& p) noexcept -> Location
					{
						return {SpellTrapLocation{.column = p.column}};
					},
					[&]([[maybe_unused]] const FieldSpellPlacement& p) noexcept -> Location
					{
						return {FieldSpellLocation{}};
					},
					[](const NotExistPlacement&) noexcept -> Location
					{
						return {NotExistLocation{}};
					},
			},
			placement
		);
	}

	[[nodiscard]] constexpr auto location_of(const FieldPlacement& placement) noexcept -> Location
	{
		return std::visit(
			utility::Overloaded
			{
					[&](const MonsterPlacement& p) noexcept -> Location
					{
						return {MonsterLocation{.column = p.column}};
					},
					[&](const ExtraMonsterPlacement& p) noexcept -> Location
					{
						return {ExtraMonsterLocation{.column = p.column}};
					},
					[&](const SpellTrapPlacement& p) noexcept -> Location
					{
						return {SpellTrapLocation{.column = p.column}};
					},
					[&]([[maybe_unused]] const FieldSpellPlacement& p) noexcept -> Location
					{
						return {FieldSpellLocation{}};
					},
			},
			placement
		);
	}

	[[nodiscard]] constexpr auto location_of(const AnyMonsterPlacement& placement) noexcept -> Location
	{
		return std::visit(
			utility::Overloaded
			{
					[&](const MonsterPlacement& p) noexcept -> Location
					{
						return {MonsterLocation{.column = p.column}};
					},
					[&](const ExtraMonsterPlacement& p) noexcept -> Location
					{
						return {ExtraMonsterLocation{.column = p.column}};
					},
			},
			placement
		);
	}
}

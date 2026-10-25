#pragma once

#include <array>
#include <variant>
#include <optional>

#include <cg/utility/variant.hpp>

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

	constexpr std::array pendulum_columns
	{
			FieldColumn::PENDULUM_LEFT,
			FieldColumn::PENDULUM_RIGHT,
	};
	constexpr auto total_pendulum_columns = pendulum_columns.size();

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
	// "袋"区位置(DECK/EXTRA_DECK/HAND/GRAVEYARD/BANISHED)
	// ==============================================================

	enum class BagPlacement : std::uint8_t
	{
		TOP,
		BOTTOM,
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
			>
	{
	public:
		using variant::variant;
	};

	class AnyBagLocation : public std::variant<
				// 卡组
				DeckLocation,
				// 额外卡组
				ExtraDeckLocation,
				// 手牌
				HandLocation,
				// 墓地
				GraveyardLocation,
				// 除外区
				BanishedLocation
				//
			>
	{
	public:
		using variant::variant;
	};

	class AnyFieldLocation : public std::variant<
				// 怪兽区
				MonsterLocation,
				// 额外怪兽区
				ExtraMonsterLocation,
				// 魔陷区
				SpellTrapLocation,
				// 场地魔法区
				FieldSpellLocation
				//
			>
	{
	public:
		using variant::variant;
	};

	class AnyMonsterLocation : public std::variant<
				// 怪兽区
				MonsterLocation,
				// 额外怪兽区
				ExtraMonsterLocation
				//
			>
	{
	public:
		using variant::variant;
	};

	// ===========================================================================
	// LOCATION ==> ZONE
	// ===========================================================================

	[[nodiscard]] constexpr auto zone_of(const Location& location) noexcept -> Zone
	{
		const auto index = location.index();
		return static_cast<Zone>(index);
	}

	[[nodiscard]] constexpr auto zone_of(const AnyBagLocation& location) noexcept -> Zone
	{
		constexpr auto base = utility::variant_index_v<DeckLocation, Location>;
		const auto index = location.index();
		return static_cast<Zone>(base + index);
	}

	[[nodiscard]] constexpr auto zone_of(const AnyFieldLocation& location) noexcept -> Zone
	{
		constexpr auto base = utility::variant_index_v<MonsterLocation, Location>;
		const auto index = location.index();
		return static_cast<Zone>(base + index);
	}

	[[nodiscard]] constexpr auto zone_of(const AnyMonsterLocation& location) noexcept -> Zone
	{
		constexpr auto base = utility::variant_index_v<MonsterLocation, Location>;
		const auto index = location.index();
		return static_cast<Zone>(base + index);
	}

	// ===========================================================================
	// LOCATION ==> INDEX
	// ===========================================================================

	template<typename L>
		requires(std::is_same_v<L, Location> || std::is_same_v<L, AnyBagLocation> || std::is_same_v<L, AnyFieldLocation> || std::is_same_v<L, AnyMonsterLocation>)
	[[nodiscard]] constexpr auto index_of(const L& location) noexcept -> std::optional<DeckLocation::size_type>
	{
		return std::visit(
			[]<typename T>(const T& l) noexcept -> std::optional<DeckLocation::size_type>
			{
				// DeckLocation/ExtraDeckLocation/HandLocation/GraveyardLocation/BanishedLocation
				if constexpr (requires { l.index; })
				{
					return l.index;
				}
				else
				{
					return std::nullopt;
				}
			},
			location
		);
	}

	// ===========================================================================
	// LOCATION ==> COLUMN
	// ===========================================================================

	template<typename L>
		requires(std::is_same_v<L, Location> || std::is_same_v<L, AnyBagLocation> || std::is_same_v<L, AnyFieldLocation> || std::is_same_v<L, AnyMonsterLocation>)
	[[nodiscard]] constexpr auto column_of(const L& location) noexcept -> std::conditional_t<std::is_same_v<L, Location>, std::optional<FieldColumn>, FieldColumn>
	{
		return std::visit(
			[]<typename T>(const T& l) noexcept -> std::optional<FieldColumn>
			{
				// MonsterLocation/ExtraMonsterLocation/SpellTrapLocation
				if constexpr (requires { l.column; })
				{
					return l.column;
				}
				else
				{
					return std::nullopt;
				}
			},
			location
		);
	}

	// ===========================================================================
	// LOCATION ==> LOCATION
	// ===========================================================================

	template<typename L>
		requires(std::is_same_v<L, AnyBagLocation> || std::is_same_v<L, AnyFieldLocation> || std::is_same_v<L, AnyMonsterLocation>)
	[[nodiscard]] constexpr auto location_of(const L& location) noexcept -> Location
	{
		return std::visit(
			[]<typename T>(const T& l) noexcept -> Location
			{
				return {l};
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
			>
	{
	public:
		using variant::variant;
	};

	class AnyBagPlacement : public std::variant<
				// 卡组
				DeckPlacement,
				// 额外卡组
				ExtraDeckPlacement,
				// 手牌
				HandPlacement,
				// 墓地
				GraveyardPlacement,
				// 除外区
				BanishedPlacement
				//
			>
	{
	public:
		using variant::variant;
	};

	class AnyFieldPlacement : public std::variant<
				// 怪兽区域
				MonsterPlacement,
				// 额外怪兽区
				ExtraMonsterPlacement,
				// 魔陷区域
				SpellTrapPlacement,
				// 场地魔法区
				FieldSpellPlacement
				//
			>
	{
	public:
		using variant::variant;
	};

	class AnyMonsterPlacement : public std::variant<
				// 怪兽区
				MonsterPlacement,
				// 额外怪兽区
				ExtraMonsterPlacement
				//
			>
	{
	public:
		using variant::variant;
	};

	// ===========================================================================
	// PLACEMENT ==> ZONE
	// ===========================================================================

	[[nodiscard]] constexpr auto zone_of(const Placement& placement) noexcept -> Zone
	{
		const auto index = placement.index();
		return static_cast<Zone>(index);
	}

	[[nodiscard]] constexpr auto zone_of(const AnyBagPlacement& placement) noexcept -> Zone
	{
		constexpr auto base = utility::variant_index_v<DeckPlacement, Placement>;
		const auto index = placement.index();
		return static_cast<Zone>(base + index);
	}

	[[nodiscard]] constexpr auto zone_of(const AnyFieldPlacement& placement) noexcept -> Zone
	{
		constexpr auto base = utility::variant_index_v<MonsterPlacement, Placement>;
		const auto index = placement.index();
		return static_cast<Zone>(base + index);
	}

	[[nodiscard]] constexpr auto zone_of(const AnyMonsterPlacement& placement) noexcept -> Zone
	{
		constexpr auto base = utility::variant_index_v<MonsterPlacement, Placement>;
		const auto index = placement.index();
		return static_cast<Zone>(base + index);
	}

	// ===========================================================================
	// PLACEMENT ==> INDEX
	// ===========================================================================

	template<typename P>
		requires(std::is_same_v<P, Placement> || std::is_same_v<P, AnyBagPlacement> || std::is_same_v<P, AnyFieldPlacement> || std::is_same_v<P, AnyMonsterPlacement>)
	[[nodiscard]] constexpr auto index_of(const P& placement) noexcept -> std::optional<DeckPlacement::size_type>
	{
		return std::visit(
			[]<typename T>(const T& p) noexcept -> std::optional<DeckPlacement::size_type>
			{
				// DeckPlacement/ExtraDeckPlacement/HandPlacement/GraveyardPlacement/BanishedPlacement
				if constexpr (requires { p.index; })
				{
					return p.index;
				}
				else
				{
					return std::nullopt;
				}
			},
			placement
		);
	}

	// ===========================================================================
	// PLACEMENT ==> COLUMN
	// ===========================================================================

	template<typename P>
		requires(std::is_same_v<P, Placement> || std::is_same_v<P, AnyBagPlacement> || std::is_same_v<P, AnyFieldPlacement> || std::is_same_v<P, AnyMonsterPlacement>)
	[[nodiscard]] constexpr auto column_of(const P& placement) noexcept -> std::optional<FieldColumn>
	{
		return std::visit(
			[]<typename T>(const T& p) noexcept -> std::optional<FieldColumn>
			{
				// MonsterPlacement/ExtraMonsterPlacement/SpellTrapPlacement
				if constexpr (requires { p.column; })
				{
					return p.column;
				}
				else
				{
					return std::nullopt;
				}
			},
			placement
		);
	}

	// ===========================================================================
	// PLACEMENT ==> POSITION
	// ===========================================================================

	template<typename P>
		requires(std::is_same_v<P, Placement> || std::is_same_v<P, AnyBagPlacement> || std::is_same_v<P, AnyFieldPlacement> || std::is_same_v<P, AnyMonsterPlacement>)
	[[nodiscard]] constexpr auto position_of(const P& placement) noexcept -> std::optional<Position>
	{
		return std::visit(
			[]<typename T>(const T& p) noexcept -> std::optional<Position>
			{
				// MonsterPlacement/ExtraMonsterPlacement/SpellTrapPlacement/FieldSpellPlacement
				if constexpr (requires { p.position; })
				{
					return p.position;
				}
				else
				{
					return std::nullopt;
				}
			},
			placement
		);
	}

	// ===========================================================================
	// PLACEMENT ==> LOCATION
	// ===========================================================================

	template<typename P>
		requires(std::is_same_v<P, Placement> || std::is_same_v<P, AnyBagPlacement> || std::is_same_v<P, AnyFieldPlacement> || std::is_same_v<P, AnyMonsterPlacement>)
	[[nodiscard]] constexpr auto location_of(const P& placement) noexcept -> Location
	{
		return std::visit(
			[]<typename T>(const T& p) noexcept -> Location
			{
				constexpr auto index = utility::variant_index_v<T, Placement>;
				// 假定Placement和Location对应结构体的位置一样!
				using location_type = utility::variant_alternative_t<index, Location>;

				// DeckPlacement/ExtraDeckPlacement/HandPlacement/GraveyardPlacement/BanishedPlacement
				if constexpr (requires { p.index; })
				{
					return location_type{.index = p.index};
				}
				// MonsterPlacement/ExtraMonsterPlacement/SpellTrapPlacement
				else if constexpr (requires { p.column; })
				{
					return location_type{.column = p.column};
				}
				else if constexpr (std::is_same_v<T, FieldSpellPlacement>)
				{
					return location_type{};
				}
				else
				{
					static_assert(index == utility::variant_index_v<NotExistPlacement, Placement>);
					return NotExistLocation{};
				}
			},
			placement
		);
	}

	[[nodiscard]] constexpr auto direct_location_of(const Placement& placement) noexcept -> Location
	{
		return location_of(placement);
	}

	[[nodiscard]] constexpr auto direct_location_of(const AnyBagPlacement& placement) noexcept -> AnyBagLocation
	{
		return std::visit(
			[]<typename T>(const T& p) noexcept -> AnyBagLocation
			{
				constexpr auto index = utility::variant_index_v<T, AnyBagPlacement>;
				// 假定AnyBagPlacement和AnyBagLocation对应结构体的位置一样!
				using location_type = utility::variant_alternative_t<index, AnyBagLocation>;

				return location_type{.index = p.index};
			},
			placement
		);
	}

	[[nodiscard]] constexpr auto direct_location_of(const AnyFieldPlacement& placement) noexcept -> AnyFieldLocation
	{
		return std::visit(
			[]<typename T>(const T& p) noexcept -> AnyFieldLocation
			{
				constexpr auto index = utility::variant_index_v<T, AnyFieldPlacement>;
				// 假定AnyFieldPlacement和AnyFieldLocation对应结构体的位置一样!
				using location_type = utility::variant_alternative_t<index, AnyFieldLocation>;

				// MonsterPlacement/ExtraMonsterPlacement/SpellTrapPlacement
				if constexpr (requires { p.column; })
				{
					return location_type{.column = p.column};
				}
				else if constexpr (std::is_same_v<T, FieldSpellPlacement>)
				{
					return location_type{};
				}
				else
				{
					std::unreachable();
				}
			},
			placement
		);
	}

	[[nodiscard]] constexpr auto direct_location_of(const AnyMonsterPlacement& placement) noexcept -> AnyMonsterLocation
	{
		return std::visit(
			[]<typename T>(const T& p) noexcept -> AnyMonsterLocation
			{
				constexpr auto index = utility::variant_index_v<T, AnyMonsterPlacement>;
				// 假定AnyMonsterPlacement和AnyMonsterLocation对应结构体的位置一样!
				using location_type = utility::variant_alternative_t<index, AnyMonsterLocation>;

				return location_type{.column = p.column};
			},
			placement
		);
	}

	// ===========================================================================
	// PLACEMENT ==> PLACEMENT
	// ===========================================================================

	template<typename P>
		requires(std::is_same_v<P, AnyBagPlacement> || std::is_same_v<P, AnyFieldPlacement> || std::is_same_v<P, AnyMonsterPlacement>)
	[[nodiscard]] constexpr auto placement_of(const P& placement) noexcept -> Placement
	{
		return std::visit(
			[]<typename T>(const T& p) noexcept -> Placement
			{
				return {p};
			},
			placement
		);
	}

	// ===========================================================================
	// PLACEMENT ==> FIELD INFO
	// ===========================================================================

	template<typename P>
		requires(std::is_same_v<P, Placement> || std::is_same_v<P, AnyBagPlacement> || std::is_same_v<P, AnyFieldPlacement> || std::is_same_v<P, AnyMonsterPlacement>)
	[[nodiscard]] constexpr auto field_info_of(const P& placement) noexcept -> std::conditional_t<
		std::is_same_v<P, Placement> || std::is_same_v<P, AnyBagPlacement> || std::is_same_v<P, AnyFieldPlacement>,
		std::optional<FieldInfo>,
		FieldInfo
	>
	{
		return std::visit(
			[]<typename T>(const T& p) noexcept ->
		std::conditional_t<
			std::is_same_v<P, Placement> || std::is_same_v<P, AnyBagPlacement> || std::is_same_v<P, AnyFieldPlacement>,
			std::optional<FieldInfo>,
			FieldInfo
		>
			{
				// MonsterPlacement/ExtraMonsterPlacement/SpellTrapPlacement
				if constexpr (requires { p.column; p.position; })
				{
					return FieldInfo{.column = p.column, .position = p.position};
				}
				else
				{
					return std::nullopt;
				}
			},
			placement
		);
	}
}

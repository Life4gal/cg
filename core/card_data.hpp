#pragma once

#include <core/defs.hpp>

namespace cg
{
	// 卡片数据
	// 用于与数据库交互
	class CardData final
	{
	public:
		using code_type = std::uint32_t;
		using property_value_type = std::uint32_t;

		// 无效卡密
		constexpr static code_type invalid_code = 0;
		// 一张卡最多有多少个字段
		constexpr static std::size_t max_archetype_count = 16;

		// 8位卡片密码(如青眼白龙是89631139)
		code_type code;
		// 别名(视为另一张卡,用于不同卡图版本)
		code_type alias_code;
		// 规则用替代密码
		code_type rule_code;

		// 卡片类型
		CardType type;
		// 卡片字段
		CardArchetype archetypes[max_archetype_count];

		// 属性
		MonsterAttribute attribute;
		// 种族
		MonsterRace race;
		// 等级
		MonsterLevel level;
		// 阶级
		MonsterRank rank;
		// 左侧灵摆刻度
		MonsterPendulum left_pendulum;
		// 右侧灵摆刻度
		MonsterPendulum right_pendulum;
		// 链接箭头
		MonsterLinkMarker link_marker;

		// 攻击力
		property_value_type attack;
		// 守备力
		property_value_type defense;

		// TODO:
		// private ctor + static load_from_database() -> CardData

		constexpr CardData() noexcept
			: code{invalid_code},
			  alias_code{invalid_code},
			  rule_code{invalid_code},
			  type{CardType::TOKEN},
			  archetypes{},
			  attribute{MonsterAttribute::EARTH},
			  race{MonsterRace::WARRIOR},
			  level{MonsterLevel::L0},
			  rank{MonsterRank::R0},
			  left_pendulum{MonsterPendulum::P0},
			  right_pendulum{MonsterPendulum::P0},
			  link_marker{MonsterLinkMarker::TOP_LEFT},
			  attack{0},
			  defense{0} {}

		// 获取卡密
		[[nodiscard]] constexpr auto get_code() const noexcept -> code_type
		{
			if (alias_code != invalid_code)
			{
				return alias_code;
			}

			return code;
		}

		// 获取规则卡密
		[[nodiscard]] constexpr auto get_rule_code() const noexcept -> code_type
		{
			if (rule_code != invalid_code)
			{
				return rule_code;
			}

			return get_code();
		}

		// 检查是否有所需字段
		[[nodiscard]] constexpr auto has_archetype(const CardArchetype required) const noexcept -> bool
		{
			for (const auto a: archetypes)
			{
				if (a == InvalidArchetype)
				{
					return false;
				}

				if (a.match(required))
				{
					return true;
				}
			}

			return false;
		}
	};
}

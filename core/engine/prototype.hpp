#pragma once

#include <array>

#include <core/domain/card.hpp>
#include <core/domain/attribute.hpp>
#include <core/domain/race.hpp>
#include <core/domain/numeric.hpp>

#include <core/engine/series_registry.hpp>

namespace cg::engine
{
	using card_code_type = std::uint32_t;

	class Prototype
	{
	public:
		constexpr static card_code_type origin_code = 0;
		constexpr static std::size_t max_series_count = 4;
		constexpr static series_code_type placeholder = 0;

		// 卡密(卡片左下角的 8 位编号)
		card_code_type code;
		// 规范卡密: 异画卡/同名卡指向的原卡(origin_code == 原卡)
		card_code_type canonical_code;
		// 规则替换卡密(origin_code == 无)
		card_code_type rule_code;

		// 字段
		std::array<series_code_type, max_series_count> series;

		// 卡牌类型
		domain::CardTypeWrapper card_type;

		// ==== 怪兽卡 ====

		// 属性
		domain::AttributeWrapper attribute;
		// 种族
		domain::Race race;

		// 等级/阶级/链接箭头
		union
		{
			domain::LevelWrapper level;
			domain::Rank rank;
			domain::LinkMarkerWrapper link_marker;
		};

		// 攻击力
		domain::attack_value_type attack;
		// 防御力
		domain::defense_value_type defense;
		// 灵摆刻度(如果有)
		domain::PendulumScale left_pendulum;
		domain::PendulumScale right_pendulum;

		// ==== 魔法卡 ====

		// ==== 陷阱卡 ====

		// ---------------------------------------------------------------

		// 记载卡密(用于同名判定: 有canonical_code时为其值,否则为code)
		[[nodiscard]] constexpr auto printed_code() const noexcept -> card_code_type
		{
			if (canonical_code != origin_code)
			{
				return canonical_code;
			}

			return code;
		}

		// 决斗中实际使用的卡密(规则替换优先)
		[[nodiscard]] constexpr auto duel_code() const noexcept -> card_code_type
		{
			if (rule_code != origin_code)
			{
				return rule_code;
			}

			return printed_code();
		}

		// 字段匹配
		[[nodiscard]] constexpr auto match_series(const series_code_type pattern) const noexcept -> bool
		{
			for (const auto candidate: series)
			{
				if (candidate == placeholder)
				{
					break;
				}

				if (candidate == pattern)
				{
					return true;
				}
			}

			return false;
		}
	};
}

#pragma once

#include <array>

#include <core/domain/card.hpp>
#include <core/domain/attribute.hpp>
#include <core/domain/race.hpp>
#include <core/domain/numeric.hpp>

namespace cg::engine
{
	class Prototype
	{
	public:
		constexpr static auto origin_code = static_cast<domain::CardCode>(0);
		constexpr static auto invalid_code = static_cast<domain::CardCode>(-1);

		constexpr static auto max_series_count = static_cast<std::size_t>(4);
		constexpr static auto invalid_series = static_cast<domain::SeriesCode>(-1);

	private:
		[[nodiscard]] constexpr static auto make_series() noexcept -> std::array<domain::SeriesCode, max_series_count>
		{
			std::array<domain::SeriesCode, max_series_count> s{};
			s.fill(invalid_series);
			return s;
		}

	public:
		// 卡密(卡片左下角的 8 位编号)
		domain::CardCode code = invalid_code;
		// 规范卡密: 异画卡/同名卡指向的原卡(origin_code == 原卡)
		domain::CardCode canonical_code = origin_code;
		// 规则替换卡密(origin_code == 无)
		domain::CardCode rule_code = origin_code;

		// 字段
		std::array<domain::SeriesCode, max_series_count> series = make_series();

		// 卡牌类型
		domain::CardTypeWrapper card_type = {domain::CardType::NONE};

		// ==== 怪兽卡 ====

		// 属性
		domain::AttributeWrapper attribute = {domain::Attribute::EARTH};
		// 种族
		domain::Race race = domain::Race::WARRIOR;

		// 等级/阶级/链接箭头
		union
		{
			domain::LevelWrapper level = {domain::Level::L1};
			domain::Rank rank;
			domain::LinkMarkerWrapper link_marker;
		};

		// 攻击力
		domain::attack_value_type attack = 0;
		// 防御力
		domain::defense_value_type defense = 0;
		// 灵摆刻度(如果有)
		domain::PendulumScale left_pendulum = domain::PendulumScale::PS0;
		domain::PendulumScale right_pendulum = domain::PendulumScale::PS0;

		// ==== 魔法卡 ====

		// ==== 陷阱卡 ====

		// ---------------------------------------------------------------

		// 记载卡密(用于同名判定: 有canonical_code时为其值,否则为code)
		[[nodiscard]] constexpr auto printed_code() const noexcept -> domain::CardCode
		{
			if (canonical_code != origin_code)
			{
				return canonical_code;
			}

			return code;
		}

		// 决斗中实际使用的卡密(规则替换优先)
		[[nodiscard]] constexpr auto duel_code() const noexcept -> domain::CardCode
		{
			if (rule_code != origin_code)
			{
				return rule_code;
			}

			return printed_code();
		}

		// 字段匹配
		[[nodiscard]] constexpr auto match_series(const domain::SeriesCode pattern) const noexcept -> bool
		{
			for (const auto candidate: series)
			{
				if (candidate == invalid_series)
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

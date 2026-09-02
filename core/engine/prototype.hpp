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
		constexpr static auto max_series_count = static_cast<std::size_t>(4);

		using series_type = std::array<domain::SeriesCode, max_series_count>;

	private:
		[[nodiscard]] constexpr static auto make_series() noexcept -> series_type
		{
			std::array<domain::SeriesCode, max_series_count> s{};
			s.fill(domain::SeriesCode::INVALID);
			return s;
		}

		// 卡密(卡片左下角的 8 位编号)
		domain::CardCode code_;
		// 规范卡密: 异画卡/同名卡指向的原卡(origin_code == 原卡)
		domain::CardCode canonical_code_;
		// 规则替换卡密(origin_code == 无)
		domain::CardCode rule_code_;

		// 字段
		series_type series_;

		// 卡牌类型
		domain::CardTypeWrapper card_type_;

		// ==== 怪兽卡 ====

		// 属性
		domain::AttributeWrapper attribute_;
		// 种族
		domain::Race race_;

		// 等级/阶级/链接箭头
		union
		{
			domain::LevelWrapper level_;
			domain::Rank rank_;
			domain::LinkMarkerWrapper link_marker_;
		};

		// 攻击力
		domain::attack_defense_value_type attack_;
		// 防御力
		domain::attack_defense_value_type defense_;
		// 灵摆刻度(如果有)
		domain::PendulumScale left_pendulum_;
		domain::PendulumScale right_pendulum_;

		// ==== 魔法卡 ====

		// ==== 陷阱卡 ====

		// ---------------------------------------------------------------

		// 记载卡密(用于同名判定: 有canonical_code时为其值,否则为code)
		[[nodiscard]] auto printed_code() const noexcept -> domain::CardCode;

		// 决斗中实际使用的卡密(规则替换优先)
		[[nodiscard]] auto duel_code() const noexcept -> domain::CardCode;

	public:
		explicit Prototype(domain::CardCode code) noexcept;

		auto set_canonical_code(domain::CardCode code) noexcept -> void;
		auto set_rule_code(domain::CardCode code) noexcept -> void;
		auto set_series(series_type series) noexcept -> void;
		auto set_card_type(domain::CardType card_type) noexcept -> void;
		auto set_card_type(domain::CardTypeWrapper card_type) noexcept -> void;
		auto set_attribute(domain::Attribute attribute) noexcept -> void;
		auto set_attribute(domain::AttributeWrapper attribute) noexcept -> void;
		auto set_race(domain::Race race) noexcept -> void;
		auto set_level(domain::Level level) noexcept -> void;
		auto set_level(domain::LevelWrapper level) noexcept -> void;
		auto set_rank(domain::Rank rank) noexcept -> void;
		auto set_link_marker(domain::LinkMarker link_marker) noexcept -> void;
		auto set_link_marker(domain::LinkMarkerWrapper link_marker) noexcept -> void;
		auto set_attack(domain::attack_defense_value_type attack) noexcept -> void;
		auto set_defense(domain::attack_defense_value_type defense) noexcept -> void;
		auto set_left_pendulum(domain::PendulumScale pendulum) noexcept -> void;
		auto set_right_pendulum(domain::PendulumScale pendulum) noexcept -> void;

		[[nodiscard]] auto code() const noexcept -> domain::CardCode;
		[[nodiscard]] auto code_rule() const noexcept -> domain::CardCode;
		[[nodiscard]] auto series() const noexcept -> std::array<domain::SeriesCode, max_series_count>;
		[[nodiscard]] auto card_type() const noexcept -> domain::CardTypeWrapper;
		[[nodiscard]] auto attribute() const noexcept -> domain::AttributeWrapper;
		[[nodiscard]] auto race() const noexcept -> domain::Race;
		[[nodiscard]] auto level() const noexcept -> domain::LevelWrapper;
		[[nodiscard]] auto rank() const noexcept -> domain::Rank;
		[[nodiscard]] auto link_marker() const noexcept -> domain::LinkMarkerWrapper;
		[[nodiscard]] auto attack() const noexcept -> domain::attack_defense_value_type;
		[[nodiscard]] auto defense() const noexcept -> domain::attack_defense_value_type;
		[[nodiscard]] auto left_pendulum() const noexcept -> domain::PendulumScale;
		[[nodiscard]] auto right_pendulum() const noexcept -> domain::PendulumScale;

		// 字段包含
		[[nodiscard]] auto has_series(domain::SeriesCode pattern) const noexcept -> bool;
		// 卡类型匹配
		[[nodiscard]] auto is_card_type(domain::CardTypeWrapper expected_card_type) const noexcept -> bool;
		// 卡类型包含
		[[nodiscard]] auto has_card_type(domain::CardTypeWrapper expected_card_type) const noexcept -> bool;
	};
}

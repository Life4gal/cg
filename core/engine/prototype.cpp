#include <core/engine/prototype.hpp>

namespace cg::engine
{
	auto Prototype::printed_code() const noexcept -> domain::CardCode
	{
		if (canonical_code_ != domain::CardCode::ORIGIN)
		{
			return canonical_code_;
		}

		return code_;
	}

	auto Prototype::duel_code() const noexcept -> domain::CardCode
	{
		if (rule_code_ != domain::CardCode::ORIGIN)
		{
			return rule_code_;
		}

		return printed_code();
	}

	Prototype::Prototype(const domain::CardCode code) noexcept
		: code_{code},
		  canonical_code_{domain::CardCode::ORIGIN},
		  rule_code_{domain::CardCode::ORIGIN},
		  series_{make_series()},
		  card_type_{domain::CardType::NONE},
		  attribute_{domain::Attribute::EARTH},
		  race_{domain::Race::WARRIOR},
		  level_{domain::Level::L1},
		  attack_{0},
		  defense_{0},
		  left_pendulum_{domain::PendulumScale::PS0},
		  right_pendulum_{domain::PendulumScale::PS0}
	{
		//
	}

	auto Prototype::set_canonical_code(const domain::CardCode code) noexcept -> void
	{
		canonical_code_ = code;
	}

	auto Prototype::set_rule_code(const domain::CardCode code) noexcept -> void
	{
		rule_code_ = code;
	}

	auto Prototype::set_series(const series_type series) noexcept -> void
	{
		series_ = series;
	}

	auto Prototype::set_card_type(const domain::CardType card_type) noexcept -> void
	{
		set_card_type(domain::CardTypeWrapper{card_type});
	}

	auto Prototype::set_card_type(const domain::CardTypeWrapper card_type) noexcept -> void
	{
		card_type_ = card_type;
	}

	auto Prototype::set_attribute(const domain::Attribute attribute) noexcept -> void
	{
		set_attribute(domain::AttributeWrapper{attribute});
	}

	auto Prototype::set_attribute(const domain::AttributeWrapper attribute) noexcept -> void
	{
		attribute_ = attribute;
	}

	auto Prototype::set_race(const domain::Race race) noexcept -> void
	{
		race_ = race;
	}

	auto Prototype::set_level(const domain::Level level) noexcept -> void
	{
		set_level(domain::LevelWrapper{level});
	}

	auto Prototype::set_level(const domain::LevelWrapper level) noexcept -> void
	{
		level_ = level;
	}

	auto Prototype::set_rank(const domain::Rank rank) noexcept -> void
	{
		rank_ = rank;
	}

	auto Prototype::set_link_marker(const domain::LinkMarker link_marker) noexcept -> void
	{
		set_link_marker(domain::LinkMarkerWrapper{link_marker});
	}

	auto Prototype::set_link_marker(const domain::LinkMarkerWrapper link_marker) noexcept -> void
	{
		link_marker_ = link_marker;
	}

	auto Prototype::set_attack(const domain::attack_defense_value_type attack) noexcept -> void
	{
		attack_ = attack;
	}

	auto Prototype::set_defense(const domain::attack_defense_value_type defense) noexcept -> void
	{
		defense_ = defense;
	}

	auto Prototype::set_left_pendulum(const domain::PendulumScale pendulum) noexcept -> void
	{
		left_pendulum_ = pendulum;
	}

	auto Prototype::set_right_pendulum(const domain::PendulumScale pendulum) noexcept -> void
	{
		right_pendulum_ = pendulum;
	}

	auto Prototype::code() const noexcept -> domain::CardCode
	{
		return code_;
	}

	auto Prototype::code_rule() const noexcept -> domain::CardCode
	{
		return duel_code();
	}

	auto Prototype::series() const noexcept -> std::array<domain::SeriesCode, max_series_count>
	{
		return series_;
	}

	auto Prototype::card_type() const noexcept -> domain::CardTypeWrapper
	{
		return card_type_;
	}

	auto Prototype::attribute() const noexcept -> domain::AttributeWrapper
	{
		return attribute_;
	}

	auto Prototype::race() const noexcept -> domain::Race
	{
		return race_;
	}

	auto Prototype::level() const noexcept -> domain::LevelWrapper
	{
		return level_;
	}

	auto Prototype::rank() const noexcept -> domain::Rank
	{
		return rank_;
	}

	auto Prototype::link_marker() const noexcept -> domain::LinkMarkerWrapper
	{
		return link_marker_;
	}

	auto Prototype::attack() const noexcept -> domain::attack_defense_value_type
	{
		return attack_;
	}

	auto Prototype::defense() const noexcept -> domain::attack_defense_value_type
	{
		return defense_;
	}

	auto Prototype::left_pendulum() const noexcept -> domain::PendulumScale
	{
		return left_pendulum_;
	}

	auto Prototype::right_pendulum() const noexcept -> domain::PendulumScale
	{
		return right_pendulum_;
	}

	auto Prototype::has_series(const domain::SeriesCode pattern) const noexcept -> bool
	{
		for (const auto candidate: series_)
		{
			if (candidate == domain::SeriesCode::INVALID)
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

	auto Prototype::is_card_type(const domain::CardTypeWrapper expected_card_type) const noexcept -> bool
	{
		return card_type_.all_of(expected_card_type);
	}

	auto Prototype::has_card_type(const domain::CardTypeWrapper expected_card_type) const noexcept -> bool
	{
		return card_type_.any_of(expected_card_type);
	}
}

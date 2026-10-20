#pragma once

#include <optional>
#include <vector>

#include <cg/utility/enum.hpp>

#include <cg/domain/card.hpp>

#include <cg/dsl/ids.hpp>

namespace cg::dsl
{
	// 怪兽固有
	class MonsterIdentity
	{
	public:
		// 属性
		domain::Attribute attribute;
		// 种族
		domain::Race race;
		// 等级 / 阶级
		domain::LevelRank level;
		// 原攻击力
		domain::AttackDefense attack;
		// 原守备力
		domain::AttackDefense defense;

		[[nodiscard]] constexpr auto operator==(const MonsterIdentity& other) const -> bool = default;
	};

	// 灵摆刻度
	class PendulumIdentity
	{
	public:
		domain::PendulumScale left;
		domain::PendulumScale right;

		[[nodiscard]] constexpr auto operator==(const PendulumIdentity& other) const -> bool = default;
	};

	// 连接箭头
	class LinkMarkerIdentity
	{
	public:
		utility::EnumMask<domain::LinkMarker> markers;

		[[nodiscard]] constexpr auto operator==(const LinkMarkerIdentity& other) const -> bool = default;
	};

	// 任意卡牌
	class CardIdentity
	{
	public:
		// 卡密
		// TODO: 目前的设计是卡牌脚本文件名即卡密,这意味着解析脚本内容时我们不知道卡牌的卡密,这个值必须由外部设置
		domain::CardCode code;
		// 主类型: MONSTER | SPELL | TRAP
		domain::CardType kind;
		// 子类型
		utility::EnumMask<domain::CardSubtype> subtypes;
		// 怪兽固有
		std::optional<MonsterIdentity> monster;
		// 灵摆刻度
		std::optional<PendulumIdentity> pendulum;
		// 连接
		std::optional<LinkMarkerIdentity> link_marker;
		// 系列/字段
		std::vector<ArchetypeId> archetypes;
		// 追加卡密
		std::vector<domain::CardCode> code_list;

		[[nodiscard]] constexpr auto operator==(const CardIdentity& other) const -> bool = default;
	};
}

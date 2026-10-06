#pragma once

#include <map>
#include <vector>

#include <engine/state/card.hpp>
#include <engine/state/zone.hpp>

namespace cg::engine
{
	class GameState
	{
	public:
		using card_definitions_type = std::map<CardCode, CardDefinition>;
		using card_instances_type = std::vector<CardInstance>;

		// 游戏中所有卡牌的定义
		card_definitions_type card_definitions;
		// 游戏中创建的所有卡牌实例
		card_instances_type card_instances;
		// 游戏场地
		Playground playground;
		//
	};

	// 指定卡密所属卡牌是否存在定义
	[[nodiscard]] auto has_definition(const GameState& state, CardCode code) noexcept -> bool;

	// 获取指定卡密所属卡牌的定义
	[[nodiscard]] auto find_definition(const GameState& state, CardCode code) noexcept -> const CardDefinition*;

	// 指定实例ID所属实例是否存在
	[[nodiscard]] auto has_instance(const GameState& state, CardInstanceId card) noexcept -> bool;

	// 获取实例ID所属实例
	[[nodiscard]] auto find_instance(const GameState& state, CardInstanceId card) noexcept -> const CardInstance*;
}

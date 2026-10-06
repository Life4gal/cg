#pragma once

#include <span>

#include <engine/summon/spec.hpp>
#include <engine/state/id.hpp>

namespace cg::engine
{
	class GameState;

	class SummonPlacement : public std::variant<
				// 召唤到主要怪兽区
				domain::MonsterLocation,
				// 召唤到额外怪兽区
				domain::ExtraMonsterLocation
			> {};

	class SummonRequest
	{
	public:
		// 召唤者
		domain::Player player;
		// 控制者(召唤到对方场上,控制者即为对方)
		domain::Player controller;
		// 召唤方式
		domain::SummonMethod method;
		// 召唤素材来源
		domain::SummonMaterialSource material_source;

		// 召唤的怪兽实例
		CardInstanceId instance;
		// 召唤使用的素材
		std::vector<CardInstanceId> materials;

		// 召唤落点
		SummonPlacement placement;
	};

	enum class SummonError : std::uint8_t
	{
		NONE,
		// 目标不存在
		NOT_FOUND,
		// 目标不是怪兽
		NOT_MONSTER,
		// 
	};

	// 召唤校验:
	//  - NORMAL:
	//   SummonRequest::instance: 位于手牌
	//   SummonRequest::materials: 空
	//  - ADVANCE:
	//   SummonRequest::instance: 位于手牌
	//   SummonRequest::materials: 取决于instance等级,[0~4]=错误,[5~6]=1,[7~12]=2
	//  - RITUAL:
	//   SummonRequest::instance: 
	//   SummonRequest::materials:
	//  - FUSION:
	//   SummonRequest::instance:
	//   SummonRequest::materials:
	//  - SYNCHRO:
	//   SummonRequest::instance:
	//   SummonRequest::materials:
	//  - XYZ:
	//   SummonRequest::instance:
	//   SummonRequest::materials:
	//  - PENDULUM:
	//   SummonRequest::instance:
	//   SummonRequest::materials:
	//  - LINK:
	//   SummonRequest::instance:
	//   SummonRequest::materials:
	//  - SPECIAL:
	//   SummonRequest::instance:
	//   SummonRequest::materials: 
	//
	auto validate_summon(const GameState& state, const SummonRequest& request) noexcept -> SummonError;

	// ==============================================================
	// NORMAL
	// ==============================================================

	//

	// ==============================================================
	// ADVANCE
	// ==============================================================

	//

	// ==============================================================
	// RITUAL
	// ==============================================================

	[[nodiscard]] auto can_be_ritual_material(const GameState& state, CardInstanceId target, CardInstanceId material) noexcept -> bool;

	// ==============================================================
	// FUSION
	// ==============================================================

	[[nodiscard]] auto can_fusion_summon(const GameState& state, const SummonRequest& request) noexcept -> bool;

	[[nodiscard]] auto can_be_fusion_material(const GameState& state, CardInstanceId target, CardInstanceId material) noexcept -> bool;

	// ==============================================================
	// SYNCHRO
	// ==============================================================

	[[nodiscard]] auto can_synchro_summon(const GameState& state, const SummonRequest& request) noexcept -> bool;

	[[nodiscard]] auto can_be_synchro_material(const GameState& state, CardInstanceId target, CardInstanceId material) noexcept -> bool;

	// ==============================================================
	// XYZ
	// ==============================================================

	[[nodiscard]] auto can_xyz_summon(const GameState& state, const SummonRequest& request) noexcept -> bool;

	[[nodiscard]] auto can_be_xyz_material(const GameState& state, CardInstanceId target, CardInstanceId material) noexcept -> bool;

	// ==============================================================
	// PENDULUM
	// ==============================================================

	[[nodiscard]] auto can_pendulum_summon(const GameState& state, domain::Player player, std::span<CardInstanceId> candidates) noexcept -> SummonError;

	// ==============================================================
	// LINK
	// ==============================================================

	[[nodiscard]] auto can_link_summon(const GameState& state, const SummonRequest& request) noexcept -> bool;

	[[nodiscard]] auto can_be_link_material(const GameState& state, CardInstanceId target, CardInstanceId material) noexcept -> bool;

	// ==============================================================
	// SPECIAL
	// ==============================================================

	//
}

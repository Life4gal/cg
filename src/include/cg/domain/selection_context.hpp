#pragma once

#include <cstdint>

namespace cg::domain
{
	// 选择行为上下文
	enum class SelectionContext : std::uint8_t
	{
		// 正在进行通常召唤
		NORMAL_SUMMON,
		// 正在进行上级召唤
		ADVANCE_SUMMON,
		// 正在选择上级召唤祭品
		SELECT_TRIBUTE,
		// 正在选择COST
		SELECT_COST,
		// 正在选择效果对象
		SELECT_EFFECT_TARGET,
		// 正在选择攻击目标
		SELECT_ATTACK_TARGET,
		// 正在选择怪兽落点
		SELECT_MONSTER_LOCATION,
		// 正在选择怪兽表示形式
		SELECT_MONSTER_POSITION,
		// 正在选择魔陷落点
		SELECT_SPELL_TRAP_LOCATION,
	};
}

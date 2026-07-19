#pragma once

#include <core/card_state.hpp>

namespace cg
{
	class Duel;

	// 卡片
	class Card
	{
	public:
		using duel_type = std::reference_wrapper<Duel>;
		using id_type = std::uint32_t;

		// -- 所属决斗 --

		duel_type duel;

		// -- 卡片数据 --

		CardData data;

		// -- 身份标识 --

		// 实例ID
		id_type instance_id;
		// 场地ID(反复进出场地的卡片不视为同一张卡)
		id_type field_id;


		// -- 状态 --

		// 当前状态
		CardState current;
		// 上一步骤状态
		CardState previous;
		// 临时状态(操作进行的中间状态)
		CardState temp;

		explicit Card(Duel& duel) noexcept;
	};
}

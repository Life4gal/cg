#pragma once

#include <core/state.hpp>

namespace cg::core
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

		Data data;

		// -- 身份标识 --

		// 实例ID,也用于排序
		id_type instance_id;
		// 场地ID(反复进出场地的卡片不视为同一张卡)
		id_type field_id;


		// -- 状态 --

		// 当前状态
		State current;
		// 上一步骤状态
		State previous;
		// 临时状态(操作进行的中间状态)
		State temp;

		explicit Card(Duel& duel) noexcept;
	};
}

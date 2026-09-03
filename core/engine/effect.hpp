#pragma once

#include <core/domain/id.hpp>

#include <core/engine/card_reference.hpp>
#include <core/engine/effect_reference.hpp>

namespace cg::engine
{
	class Duel;

	class Effect
	{
	public:
		//

	private:
		// Duel实例引用
		std::reference_wrapper<Duel> duel_;
		// 效果实例ID,由Duel生成
		domain::EffectInstanceId instance_id_;
		// 效果所有者
		CardReference owner_;

	public:
		Effect(Duel& duel, domain::EffectInstanceId instance_id, Card& owner) noexcept;

		[[nodiscard]] auto instance_id() const noexcept -> domain::EffectInstanceId;
	};
}

#include <cg/dsl/effect.hpp>

#include <cg/utility/functional.hpp>

namespace cg::dsl
{
	auto speed_of(const domain::CardType type, const utility::EnumMask<domain::CardSubtype> subtypes, const ActivationKind& activation) noexcept -> domain::EffectSpeed
	{
		return std::visit(
			utility::Overloaded
			{
					[&]([[maybe_unused]] const CardActivation& kind) noexcept -> domain::EffectSpeed
					{
						switch (type)
						{
							case domain::CardType::MONSTER:
							{
								// 怪兽不使用卡自身的"发动"
								// 怪兽效果见Ignition/Trigger/Quick
								return domain::EffectSpeed::NONE;
							}
							case domain::CardType::SPELL:
							{
								if (subtypes.contains(domain::CardSubtype::QUICK_PLAY))
								{
									return domain::EffectSpeed::QUICK;
								}

								return domain::EffectSpeed::NORMAL;
							}
							case domain::CardType::TRAP:
							{
								if (subtypes.contains(domain::CardSubtype::COUNTER))
								{
									return domain::EffectSpeed::COUNTER;
								}

								return domain::EffectSpeed::QUICK;
							}
							default: // NOLINT(clang-diagnostic-covered-switch-default)
							{
								std::unreachable();
							}
						}
					},
					[]([[maybe_unused]] const Ignition& kind) noexcept -> domain::EffectSpeed
					{
						return domain::EffectSpeed::NORMAL;
					},
					[]([[maybe_unused]] const Trigger& kind) noexcept -> domain::EffectSpeed
					{
						return domain::EffectSpeed::NORMAL;
					},
					[]([[maybe_unused]] const Quick& kind) noexcept -> domain::EffectSpeed
					{
						return domain::EffectSpeed::QUICK;
					},
			},
			activation
		);
	}
}

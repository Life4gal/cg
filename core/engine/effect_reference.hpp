#pragma once

#include <functional>

namespace cg::engine
{
	class Effect;

	class EffectReference : public std::reference_wrapper<Effect>
	{
	public:
		using reference_wrapper::reference_wrapper;
	};

	class EffectOptional
	{
	public:
		// todo: 一个更安全的可选保有Effect实例的包装类,避免危险的裸指针

	private:
		Effect* effect_;

	public:
		constexpr EffectOptional() noexcept
			: effect_{nullptr} {}

		constexpr explicit EffectOptional(Effect* effect) noexcept
			: effect_{effect} {}

		[[nodiscard]] constexpr auto operator==(const EffectOptional& other) const noexcept -> bool
		{
			return effect_ == other.effect_;
		}

		// todo
	};
}

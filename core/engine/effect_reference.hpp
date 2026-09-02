#pragma once

#include <functional>

namespace cg::engine
{
	class Effect;

	class EffectReference : public std::reference_wrapper<Effect>
	{
	public:
		using reference_wrapper::reference_wrapper;

		[[nodiscard]] constexpr auto operator==(const EffectReference& other) const noexcept -> bool
		{
			// 必须引用同一个实例
			return &get() == &other.get();
		}
	};

	class EffectOptional
	{
	public:
		// todo: 一个更安全的可选保有Effect实例的包装类,避免危险的裸指针

	private:
		Effect* effect_;

	public:
		constexpr EffectOptional() noexcept
			: EffectOptional{nullptr} {}

		constexpr explicit(false) EffectOptional(std::nullptr_t) noexcept
			: effect_{nullptr} {}

		constexpr explicit EffectOptional(Effect* effect) noexcept
			: effect_{effect} {}

		constexpr explicit EffectOptional(const EffectReference& reference) noexcept
			: effect_{&reference.get()} {}

		[[nodiscard]] constexpr auto operator*() const noexcept -> Effect&
		{
			return *effect_;
		}

		// 不传播const
		// [[nodiscard]] constexpr auto operator*() const noexcept -> const Effect&
		// {
		// 	return *effect_;
		// }

		[[nodiscard]] constexpr auto operator->() const noexcept -> Effect*
		{
			return effect_;
		}

		// 不传播const
		// [[nodiscard]] constexpr auto operator->() const noexcept -> const Effect*
		// {
		// 	return effect_;
		// }

		[[nodiscard]] constexpr explicit operator bool() const noexcept
		{
			return effect_ != nullptr;
		}

		[[nodiscard]] constexpr auto operator==(const Effect* other) const noexcept -> bool
		{
			return effect_ == other;
		}

		[[nodiscard]] constexpr auto operator==(const EffectOptional& other) const noexcept -> bool
		{
			return *this == other.effect_;
		}

		[[nodiscard]] constexpr auto operator==(const EffectReference& other) const noexcept -> bool
		{
			return effect_ != nullptr && effect_ == &other.get();
		}

		[[nodiscard]] friend constexpr auto operator==(const EffectReference& lhs, const EffectOptional& rhs) noexcept -> bool
		{
			return rhs == lhs;
		}
	};
}

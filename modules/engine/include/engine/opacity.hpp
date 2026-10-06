#pragma once

#include <cstdint>

namespace cg::engine
{
	template<typename Tag, typename ValueType = std::uint32_t, ValueType Invalid = ValueType{0}>
	class Opacity
	{
	public:
		using tag = Tag;

		using value_type = ValueType;

		constexpr static value_type invalid_id = Invalid;

	private:
		value_type handler_;

	public:
		constexpr Opacity() noexcept
			: handler_{invalid_id} {}

		constexpr explicit Opacity(const value_type id) noexcept
			: handler_{id} {}

		[[nodiscard]] constexpr auto valid() const noexcept -> bool
		{
			return handler_ != invalid_id;
		}

		[[nodiscard]] constexpr auto raw() const noexcept -> value_type
		{
			return handler_;
		}

		friend constexpr auto operator<=>(const Opacity& lhs, const Opacity& rhs) noexcept -> auto = default;
		friend constexpr auto operator==(const Opacity& lhs, const Opacity& rhs) noexcept -> bool = default;
	};
}

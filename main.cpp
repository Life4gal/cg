#include <print>

#include <core/utility/enum.hpp>

enum class T : std::uint8_t
{
	T1 = 1,
	T2 = 2,
	T3 = 3,
};

using te = cg::utility::Enum<T, cg::utility::EnumSupport::EQUALITY, cg::utility::EnumSupport::COMPARISON, cg::utility::EnumSupport::ARITHMETIC, cg::utility::EnumSupport::BIT_OPERATION>;

static_assert(te::supported<cg::utility::EnumSupport::EQUALITY>());
static_assert(te::supported<cg::utility::EnumSupport::COMPARISON>());
static_assert(te::supported<cg::utility::EnumSupport::ARITHMETIC>());
static_assert(te::supported<cg::utility::EnumSupport::BIT_OPERATION>());

auto main() noexcept -> int
{
	constexpr te e1{T::T1};
	constexpr te e2{T::T2};

	auto e3 = e1 + e2;
	e3 += e1;

	auto e4 = e3 - e2;
	e4 -= e1;

	auto e5 = ~e4;
	e5 ^= e1;

	auto e6 = e1 | e2;
	e6 &= e1;

	return static_cast<int>(e3);
}

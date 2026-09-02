#pragma once

#include <core/engine/collection.hpp>

namespace cg::engine
{
	class TargetInfo
	{
	public:
		using size_type = std::uint32_t;

	private:
		// 本卡指定为对象的卡
		Group card_targets_;
		// 以本卡为对象的卡
		Group owner_targets_;

		auto set_owner_target(Card& card) noexcept -> bool;
		auto cancel_owner_target(const Card& card) noexcept -> bool;

	public:
		TargetInfo() noexcept;

		auto set_target(Card& target, Card& owner) noexcept -> bool;
		auto cancel_target(Card& target, const Card& owner) noexcept -> bool;

		[[nodiscard]] auto card_targets() const noexcept -> View;
		[[nodiscard]] auto owner_targets() const noexcept -> View;

		[[nodiscard]] auto has_target() const noexcept -> bool;
		[[nodiscard]] auto has_target(const Card& card) const noexcept -> bool;
		[[nodiscard]] auto target_count() const noexcept -> size_type;
	};
}

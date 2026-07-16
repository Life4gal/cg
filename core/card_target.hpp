#pragma once

#include <memory>
#include <vector>

namespace cg
{
	class Card;

	// 发动效果对象
	class TargetCard
	{
	public:
		using card_type = std::shared_ptr<Card>;

	private:
		card_type card_;
		// todo: 其他信息(例如位置信息)

	public:
		explicit TargetCard(card_type card) noexcept;

		TargetCard(const TargetCard&) noexcept = delete;
		auto operator=(const TargetCard&) noexcept -> TargetCard& = delete;

		TargetCard(TargetCard&&) noexcept;
		auto operator=(TargetCard&&) noexcept -> TargetCard&;

		~TargetCard() noexcept;

		// todo: 其他接口
	};

	// 发动效果对象
	class TargetCards
	{
	public:
		using targets_type = std::vector<TargetCard>;

		using size_type = targets_type::size_type;

	private:
		targets_type targets_;

	public:
		TargetCards() noexcept;

		auto reserve(size_type capacity) noexcept -> void;

		auto add(TargetCard&& target) noexcept -> void;

		[[nodiscard]] auto size() const noexcept -> size_type;
	};
}

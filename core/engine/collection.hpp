#pragma once

#include <set>
#include <span>
#include <ranges>
#include <algorithm>
#include <vector>

#include <core/engine/card_reference.hpp>
#include <core/engine/card_order.hpp>

namespace cg::engine
{
	// 卡牌集合
	// 无序(相对于插入顺序)
	class Group
	{
	public:
		using container_type = std::set<CardReference, CardOrder>;

		using iterator = container_type::iterator;
		using const_iterator = container_type::const_iterator;
		using size_type = container_type::size_type;

	private:
		container_type cards_;

		constexpr explicit Group(container_type cards) noexcept
			: cards_{std::move(cards)} {}

	public:
		constexpr Group() noexcept = default;

		// -------------------------------------------------
		// 容器大小
		// -------------------------------------------------

		[[nodiscard]] constexpr auto empty() const noexcept -> bool
		{
			return cards_.empty();
		}

		[[nodiscard]] constexpr auto size() const noexcept -> size_type
		{
			return cards_.size();
		}

		// -------------------------------------------------
		// 遍历容器
		// -------------------------------------------------

		[[nodiscard]] constexpr auto begin() noexcept -> iterator
		{
			return cards_.begin();
		}

		[[nodiscard]] constexpr auto begin() const noexcept -> const_iterator
		{
			return cards_.begin();
		}

		[[nodiscard]] constexpr auto end() noexcept -> iterator
		{
			return cards_.end();
		}

		[[nodiscard]] constexpr auto end() const noexcept -> const_iterator
		{
			return cards_.end();
		}

		[[nodiscard]] constexpr auto contains(const Card& card) const noexcept -> bool
		{
			return cards_.contains(const_cast<Card&>(card));
		}

		// -------------------------------------------------
		// 修改容器
		// -------------------------------------------------

		constexpr auto reserve(const size_type capacity) noexcept -> void
		{
			std::ignore = this;
			std::ignore = capacity;
		}

		constexpr auto insert(Card& card) noexcept -> bool
		{
			return cards_.insert(card).second;
		}

		template<typename Range>
		constexpr auto insert(Range&& range) noexcept -> auto //
			requires requires { cards_.insert_range(std::forward<Range>(range)); }
		{
			return cards_.insert_range(std::forward<Range>(range));
		}

		constexpr auto erase(const Card& card) noexcept -> bool
		{
			return cards_.erase(const_cast<Card&>(card)) != 0;
		}

		constexpr auto clear() noexcept -> void
		{
			cards_.clear();
		}

		// -------------------------------------------------
		// 子视图
		// -------------------------------------------------

		// 获取前N张卡片的子视图
		[[nodiscard]] constexpr auto select(const size_type count) const noexcept -> Group
		{
			auto result = cards_ | std::views::take(count) | std::ranges::to<container_type>();

			return Group{std::move(result)};
		}

		// -------------------------------------------------
		// 过滤
		// -------------------------------------------------

		// 获取所有满足条件的卡片的子视图
		template<typename Predicate>
			requires std::predicate<Predicate, const Card&>
		[[nodiscard]] constexpr auto filter(Predicate predicate) const noexcept -> Group
		{
			auto result = cards_ | std::views::filter(predicate) | std::ranges::to<container_type>();

			return Group{std::move(result)};
		}

		// 获取所有满足条件的卡片的数量
		template<typename Predicate>
			requires std::predicate<Predicate, const Card&>
		[[nodiscard]] constexpr auto filter_count(Predicate predicate) const noexcept -> size_type
		{
			return std::ranges::count_if(cards_, predicate);
		}

		// 检查是否有满足条件的卡片
		template<typename Predicate>
			requires std::predicate<Predicate, const Card&>
		[[nodiscard]] constexpr auto filter_exists(Predicate predicate) const noexcept -> bool
		{
			return cards_ | std::ranges::any_of(predicate);
		}

		// 获取第一张满足条件的卡片
		template<typename Predicate>
			requires std::predicate<Predicate, const Card&>
		[[nodiscard]] constexpr auto filter_first(Predicate predicate) const noexcept -> const Card*
		{
			const auto it = cards_ | std::ranges::find_if(predicate);

			if (it == std::ranges::end(cards_))
			{
				return nullptr;
			}

			return &*it;
		}
	};

	// 卡牌序列
	// 有序(相对于插入顺序),但是不保证没有重复
	class Sequence
	{
	public:
		using container_type = std::vector<CardReference>;

		using iterator = container_type::iterator;
		using const_iterator = container_type::const_iterator;
		using size_type = container_type::size_type;

	private:
		container_type cards_;

		constexpr explicit Sequence(container_type cards) noexcept
			: cards_{std::move(cards)} {}

	public:
		constexpr Sequence() noexcept = default;

		// -------------------------------------------------
		// 容器大小
		// -------------------------------------------------

		[[nodiscard]] constexpr auto empty() const noexcept -> bool
		{
			return cards_.empty();
		}

		[[nodiscard]] constexpr auto size() const noexcept -> size_type
		{
			return cards_.size();
		}

		// -------------------------------------------------
		// 遍历容器
		// -------------------------------------------------

		[[nodiscard]] constexpr auto begin() noexcept -> iterator
		{
			return cards_.begin();
		}

		[[nodiscard]] constexpr auto begin() const noexcept -> const_iterator
		{
			return cards_.begin();
		}

		[[nodiscard]] constexpr auto end() noexcept -> iterator
		{
			return cards_.end();
		}

		[[nodiscard]] constexpr auto end() const noexcept -> const_iterator
		{
			return cards_.end();
		}

		[[nodiscard]] constexpr auto contains(const Card& card) const noexcept -> bool
		{
			return std::ranges::contains(
				cards_,
				&card,
				[](const CardReference& r) noexcept -> const Card*
				{
					return &r.get();
				}
			);
		}

		// -------------------------------------------------
		// 修改容器
		// -------------------------------------------------

		constexpr auto reserve(const size_type capacity) noexcept -> void
		{
			cards_.reserve(capacity);
		}

		constexpr auto insert(Card& card) noexcept -> bool
		{
			cards_.emplace_back(card);
			return true;
		}

		template<typename Range>
		constexpr auto insert(Range&& range) noexcept -> auto //
			requires requires { cards_.insert_range(std::forward<Range>(range)); }
		{
			return cards_.insert_range(std::forward<Range>(range));
		}

		constexpr auto erase(const Card& card) noexcept -> bool
		{
			const auto it = std::ranges::find(
				cards_,
				&card,
				[](const CardReference& r) noexcept -> const Card*
				{
					return &r.get();
				}
			);
			if (it != cards_.end())
			{
				cards_.erase(it);
				return true;
			}

			return false;
		}

		constexpr auto clear() noexcept -> void
		{
			cards_.clear();
		}

		// -------------------------------------------------
		// 子视图
		// -------------------------------------------------

		// 获取前N张卡片的子视图
		[[nodiscard]] constexpr auto select(const size_type count) const noexcept -> Sequence
		{
			auto result = cards_ | std::views::take(count) | std::ranges::to<container_type>();

			return Sequence{std::move(result)};
		}

		// -------------------------------------------------
		// 过滤
		// -------------------------------------------------

		// 获取所有满足条件的卡片的子视图
		template<typename Predicate>
			requires std::predicate<Predicate, const Card&>
		[[nodiscard]] constexpr auto filter(Predicate predicate) const noexcept -> Sequence
		{
			auto result = cards_ | std::views::filter(predicate) | std::ranges::to<container_type>();

			return Sequence{std::move(result)};
		}

		// 获取所有满足条件的卡片的数量
		template<typename Predicate>
			requires std::predicate<Predicate, const Card&>
		[[nodiscard]] constexpr auto filter_count(Predicate predicate) const noexcept -> size_type
		{
			return std::ranges::count_if(cards_, predicate);
		}

		// 检查是否有满足条件的卡片
		template<typename Predicate>
			requires std::predicate<Predicate, const Card&>
		[[nodiscard]] constexpr auto filter_exists(Predicate predicate) const noexcept -> bool
		{
			return cards_ | std::ranges::any_of(predicate);
		}

		// 获取第一张满足条件的卡片
		template<typename Predicate>
			requires std::predicate<Predicate, const Card&>
		[[nodiscard]] constexpr auto filter_first(Predicate predicate) const noexcept -> const Card*
		{
			const auto it = cards_ | std::ranges::find_if(predicate);

			if (it == std::ranges::end(cards_))
			{
				return nullptr;
			}

			return &*it;
		}
	};

	class View
	{
	public:
		using container_type = std::vector<CardReference>;

		using iterator = container_type::iterator;
		using const_iterator = container_type::const_iterator;
		using size_type = container_type::size_type;

	private:
		container_type cards_;

	public:
		constexpr explicit View(const Group& group) noexcept
			: cards_{group.begin(), group.end()} {}

		constexpr explicit View(const Sequence& sequence) noexcept
			: cards_{sequence.begin(), sequence.end()} {}

		// -------------------------------------------------
		// 容器大小
		// -------------------------------------------------

		[[nodiscard]] constexpr auto empty() const noexcept -> bool
		{
			return cards_.empty();
		}

		[[nodiscard]] constexpr auto size() const noexcept -> size_type
		{
			return cards_.size();
		}

		// -------------------------------------------------
		// 遍历容器
		// -------------------------------------------------

		[[nodiscard]] constexpr auto begin() noexcept -> iterator
		{
			return cards_.begin();
		}

		[[nodiscard]] constexpr auto begin() const noexcept -> const_iterator
		{
			return cards_.begin();
		}

		[[nodiscard]] constexpr auto end() noexcept -> iterator
		{
			return cards_.end();
		}

		[[nodiscard]] constexpr auto end() const noexcept -> const_iterator
		{
			return cards_.end();
		}
	};
}

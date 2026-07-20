#pragma once

#include <algorithm>
#include <set>
#include <ranges>

namespace cg::core
{
	class Card;

	// 卡片视图
	class View
	{
	public:
		class Sorter
		{
		public:
			[[nodiscard]] static auto operator()(const Card& lhs, const Card& rhs) noexcept -> bool;
		};

		using card_type = std::reference_wrapper<const Card>;
		using view_type = std::set<card_type, Sorter>;

		using iterator = view_type::iterator;
		using const_iterator = view_type::const_iterator;
		using size_type = view_type::size_type;

	private:
		view_type view_;

		template<typename Self>
		[[nodiscard]] constexpr auto view(this Self&& self) noexcept -> decltype(auto)
		{
			return std::forward<Self>(self).view_;
		}

	public:
		constexpr View() noexcept = default;

		constexpr explicit View(view_type view) noexcept
			: view_{std::move(view)} {}

		// -------------------------------------------------
		// 容器大小
		// -------------------------------------------------

		[[nodiscard]] constexpr auto empty() const noexcept -> bool
		{
			return view_.empty();
		}

		[[nodiscard]] constexpr auto size() const noexcept -> size_type
		{
			return view_.size();
		}

		// -------------------------------------------------
		// 遍历容器
		// -------------------------------------------------

		template<typename Self>
		[[nodiscard]] constexpr auto begin(this Self&& self) noexcept -> auto
		{
			return this->view(std::forward<Self>(self)).begin();
		}

		template<typename Self>
		[[nodiscard]] constexpr auto end(this Self&& self) noexcept -> auto
		{
			return this->view(std::forward<Self>(self)).end();
		}

		[[nodiscard]] constexpr auto contains(const Card& card) const noexcept -> bool
		{
			return view_.contains(card);
		}

		// -------------------------------------------------
		// 修改容器
		// -------------------------------------------------

		constexpr auto reserve(const size_type capacity) noexcept -> void
		{
			std::ignore = this;
			std::ignore = capacity;
		}

		constexpr auto insert(const Card& card) noexcept -> auto
		{
			return view_.insert(card);
		}

		template<typename Range>
		constexpr auto insert(Range&& range) noexcept -> auto //
			requires requires { view_.insert_range(std::forward<Range>(range)); }
		{
			return view_.insert_range(std::forward<Range>(range));
		}

		constexpr auto erase(const Card& card) noexcept -> auto
		{
			return view_.erase(card);
		}

		constexpr auto clear() noexcept -> void
		{
			view_.clear();
		}

		// -------------------------------------------------
		// 子视图
		// -------------------------------------------------

		// 获取前N张卡片的子视图
		[[nodiscard]] constexpr auto select(const size_type count) const noexcept -> View
		{
			auto result = view_ | std::views::take(count) | std::ranges::to<view_type>();

			return View{std::move(result)};
		}

		// -------------------------------------------------
		// 过滤
		// -------------------------------------------------

		// 获取所有满足条件的卡片的子视图
		template<typename Predicate>
			requires std::predicate<Predicate, const Card&>
		[[nodiscard]] constexpr auto filter(Predicate predicate) const noexcept -> View
		{
			auto result = view_ | std::views::filter(predicate) | std::ranges::to<view_type>();

			return View{std::move(result)};
		}

		// 获取所有满足条件的卡片的数量
		template<typename Predicate>
			requires std::predicate<Predicate, const Card&>
		[[nodiscard]] constexpr auto filter_count(Predicate predicate) const noexcept -> size_type
		{
			return std::ranges::count_if(view_, predicate);
		}

		// 检查是否有满足条件的卡片
		template<typename Predicate>
			requires std::predicate<Predicate, const Card&>
		[[nodiscard]] constexpr auto filter_exists(Predicate predicate) const noexcept -> bool
		{
			return view_ | std::ranges::any_of(predicate);
		}

		// 获取第一张满足条件的卡片
		template<typename Predicate>
			requires std::predicate<Predicate, const Card&>
		[[nodiscard]] constexpr auto filter_first(Predicate predicate) const noexcept -> const Card*
		{
			const auto it = view_ | std::ranges::find_if(predicate);

			if (it == std::ranges::end(view_))
			{
				return nullptr;
			}

			return &*it;
		}
	};
}

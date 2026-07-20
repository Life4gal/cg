#pragma once

#include <random>
#include <unordered_set>

#include <core/bridge.hpp>
#include <core/field.hpp>

namespace cg::core
{
	class Duel
	{
	public:
		using id_type = Card::id_type;

		using random_type = std::mt19937;

		template<typename T>
		class Hasher
		{
		public:
			using is_transparent = int;

			[[nodiscard]] static auto operator()(const std::unique_ptr<T>& object) noexcept -> std::size_t
			{
				// return std::hash<std::unique_ptr<T>>::operator()(object);
				return Hasher::operator()(object.get());
			}

			[[nodiscard]] static auto operator()(const T* object) noexcept -> std::size_t
			{
				return std::hash<T>::operator()(object);
			}
		};

	private:
		// 随机数生成器
		random_type random_;

		// 桥(脚本引擎)
		std::unique_ptr<Bridge> bridge_;

		// 决斗场地
		Field field_;

		// 决斗用到的所有卡片
		std::unordered_set<std::unique_ptr<Card>, Hasher<Card>, std::equal_to<>> cards_;
		// 决斗用到的所有卡片视图
		std::unordered_set<std::unique_ptr<View>, Hasher<View>, std::equal_to<>> views_;
		// 决斗用到的所有卡片效果
		std::unordered_set<std::unique_ptr<Effect>, Hasher<Effect>, std::equal_to<>> effects_;

		// 卡片实例id生成器
		id_type instance_id_;

		[[nodiscard]] auto make_instance_id() noexcept -> id_type;

	public:
		Duel() noexcept;
		explicit Duel(random_type::result_type seed) noexcept;
		explicit Duel(const std::seed_seq& seed) noexcept;

		auto initialize(std::unique_ptr<Bridge> bridge) noexcept -> bool;
		auto initialize(Bridge* bridge) noexcept -> bool;

		[[nodiscard]] auto create_card(Data::code_type code) noexcept -> Card&;
		auto destroy_card(Card& card) noexcept -> bool;

	private:
		[[nodiscard]] auto create_view(std::unique_ptr<View> view) noexcept -> View&;

	public:
		[[nodiscard]] auto create_view() noexcept -> View&;
		[[nodiscard]] auto create_view(const Card& card) noexcept -> View&;
		[[nodiscard]] auto create_view(View::view_type cards) noexcept -> View&;
		auto destroy_view(View& view) noexcept -> bool;

		[[nodiscard]] auto create_effect() noexcept -> Effect&;
		auto destroy_effect(Effect& effect) noexcept -> bool;
	};
}

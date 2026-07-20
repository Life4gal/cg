#include <core/duel.hpp>

namespace cg::core
{
	auto Duel::make_instance_id() noexcept -> id_type
	{
		const auto id = instance_id_;
		instance_id_ += 1;
		return id;
	}

	Duel::Duel() noexcept
		: Duel{std::random_device{}()}
	{
		//
	}

	Duel::Duel(const random_type::result_type seed) noexcept
		: random_{seed},
		  instance_id_{1}
	{
		//
	}

	Duel::Duel(const std::seed_seq& seed) noexcept
		: random_{seed},
		  instance_id_{1}
	{
		//
	}

	auto Duel::initialize(std::unique_ptr<Bridge> bridge) noexcept -> bool
	{
		bridge_ = std::move(bridge);

		// TODO

		return true;
	}

	auto Duel::initialize(Bridge* bridge) noexcept -> bool
	{
		auto p = std::unique_ptr<Bridge>(bridge);

		return initialize(std::move(p));
	}

	auto Duel::create_card(Data::code_type code) noexcept -> Card&
	{
		auto& card = cards_.emplace(std::make_unique<Card>(*this)).first.operator*();
		card->instance_id = make_instance_id();

		// TODO: read card data

		// 注册该卡片
		bridge_->register_card(*card);

		return *card;
	}

	auto Duel::destroy_card(Card& card) noexcept -> bool
	{
		bool result = true;

		// todo: 从场地移除该卡片

		// 从脚本引擎中移除该卡片
		result &= bridge_->unregister_card(card);

		// 从实例中移除该卡片
		result &= cards_.erase(&card);

		return result;
	}

	auto Duel::create_view(std::unique_ptr<View> view) noexcept -> View&
	{
		auto& v = views_.emplace(std::move(view)).first.operator*();

		// 注册该视图
		bridge_->register_view(*v);

		return *v;
	}

	auto Duel::create_view() noexcept -> View&
	{
		return create_view(std::make_unique<View>());
	}

	auto Duel::create_view(const Card& card) noexcept -> View&
	{
		View::view_type view{};
		view.insert(card);

		return create_view(std::move(view));
	}

	auto Duel::create_view(View::view_type cards) noexcept -> View&
	{
		return create_view(std::make_unique<View>(std::move(cards)));
	}

	auto Duel::destroy_view(View& view) noexcept -> bool
	{
		bool result = true;

		// 从脚本引擎中移除该视图
		result &= bridge_->unregister_view(view);

		// 从实例中移除该视图
		result &= views_.erase(&view);

		return result;
	}

	auto Duel::create_effect() noexcept -> Effect&
	{
		auto& effect = effects_.emplace(std::make_unique<Effect>(*this)).first.operator*();

		// 注册该效果
		bridge_->register_effect(*effect);

		return *effect;
	}

	auto Duel::destroy_effect(Effect& effect) noexcept -> bool
	{
		bool result = true;

		// 从脚本引擎中移除该效果
		result &= bridge_->unregister_effect(effect);

		// 从实例中移除该效果
		result &= effects_.erase(&effect);

		return result;
	}
}

#include <core/engine/duel.hpp>

namespace
{
	using namespace cg;

	constexpr auto card_instance_id_begin = static_cast<domain::CardInstanceId>(0);
	constexpr auto card_instance_step = static_cast<domain::CardInstanceId>(1);
	constexpr auto effect_instance_id_begin = static_cast<domain::EffectInstanceId>(0);
	constexpr auto effect_instance_id_step = static_cast<domain::EffectInstanceId>(1);
}

namespace cg::engine
{
	Duel::Duel(const random_type::result_type seed) noexcept
		: random_{seed},
		  api_{*this},
		  host_{nullptr},
		  // prototype_registry_{},
		  // series_registry_{},
		  field_{*this},
		  // cards_{},
		  // effects_{},
		  card_instance_id_generator_{card_instance_id_begin},
		  effect_instance_id_generator_{effect_instance_id_begin}
	{
		//
	}


	Duel::Duel() noexcept
		: Duel{std::random_device{}()} {}

	auto Duel::bind_host(std::unique_ptr<script::Host> host) noexcept -> void
	{
		host_ = std::move(host);
		host_->bind_api(api_);
	}

	auto Duel::start() noexcept -> void
	{
		auto field_playground = field_.playground();
		auto field_turn = field_.turn();

		// 洗牌
		field_playground.shuffle_deck(domain::Player::FIRST);
		field_playground.shuffle_deck(domain::Player::SECOND);

		// 抽牌
		field_playground.start_draw(domain::Player::FIRST);
		field_playground.start_draw(domain::Player::SECOND);

		// 开始回合
		field_turn.new_turn(domain::Player::FIRST);

		// 限制第一回合不能攻击
		field_turn.set_can_battle(false);

		// todo: 广播决斗开始
	}

	auto Duel::random() const noexcept -> random_type&
	{
		return random_;
	}

	auto Duel::register_prototype(const domain::CardCode code) noexcept -> bool
	{
		if (host_ == nullptr)
		{
			return false;
		}

		const auto prototype = host_->load_prototype(code);
		if (!prototype.has_value())
		{
			return false;
		}

		prototype_registry_.register_prototype(*prototype);
		return true;
	}

	auto Duel::set_player_info(
		const domain::Player player,
		const domain::life_point_type life_point,
		const domain::zone_sequence_type start_hand,
		const domain::zone_sequence_type draw_count
	) noexcept -> void
	{
		auto playground = field_.playground();

		playground.set_life_point(player, life_point);
		playground.set_start_hand(player, start_hand);
		playground.set_draw_count(player, draw_count);
	}
}

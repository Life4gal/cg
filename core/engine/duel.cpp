#include <core/engine/duel.hpp>

namespace
{
	using namespace cg;

	constexpr auto card_instance_id_begin = static_cast<domain::CardInstanceId>(0);
	constexpr auto card_instance_id_step = static_cast<domain::CardInstanceId>(1);
	constexpr auto effect_instance_id_begin = static_cast<domain::EffectInstanceId>(0);
	constexpr auto effect_instance_id_step = static_cast<domain::EffectInstanceId>(1);
}

namespace cg::engine
{
	auto Duel::register_prototype(const domain::CardCode code) noexcept -> const Prototype*
	{
		if (host_ == nullptr)
		{
			return nullptr;
		}

		const auto prototype = host_->load_prototype(code);
		if (!prototype.has_value())
		{
			return nullptr;
		}

		return &prototype_registry_.register_prototype(*prototype);
	}

	auto Duel::make_card_instance_id() noexcept -> domain::CardInstanceId
	{
		const auto id = card_instance_id_generator_;
		card_instance_id_generator_ += card_instance_id_step;
		return id;
	}

	auto Duel::make_effect_instance_id() noexcept -> domain::EffectInstanceId
	{
		const auto id = effect_instance_id_generator_;
		effect_instance_id_generator_ += effect_instance_id_step;
		return id;
	}

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

	Duel::Duel(Duel&&) noexcept = default;

	auto Duel::operator=(Duel&&) noexcept -> Duel& = default;

	Duel::~Duel() noexcept = default;

	auto Duel::bind_host(std::unique_ptr<script::Host> host) noexcept -> void
	{
		host_ = std::move(host);
		host_->bind_api(api_);
	}

	auto Duel::set_player_info(
		const domain::Player player,
		const domain::life_point_type life_point,
		const Field::size_type start_hand,
		const Field::size_type draw_count
	) noexcept -> void
	{
		auto playground = field_.playground();

		playground.set_life_point(player, life_point);
		playground.set_start_hand(player, start_hand);
		playground.set_draw_count(player, draw_count);
	}

	auto Duel::register_card(const domain::CardCode code, const domain::Player player, const domain::Zone zone) noexcept -> bool
	{
		// 检查原型是否已注册
		const auto* prototype = prototype_registry_.find_prototype(code);
		if (prototype == nullptr)
		{
			// 未注册则进行注册
			prototype = register_prototype(code);

			if (prototype == nullptr)
			{
				// 原型注册失败
				// todo: 怎么办?
				static Prototype fallback{domain::CardCode::INVALID};
				prototype = &fallback;
			}
		}

		// 创建卡牌实例
		auto& card = cards_.emplace_back(*this, make_card_instance_id(), player, *prototype);

		// 移动到指定位置
		auto field_playground = field_.playground();
		return field_playground.move_card(card, player, zone, domain::Reason::RULE);
	}

	auto Duel::start() noexcept -> void
	{
		auto field_playground = field_.playground();
		auto field_turn = field_.turn();

		// 洗牌
		field_playground.shuffle(domain::Player::FIRST, domain::AutoZone::DECK, random_);
		field_playground.shuffle(domain::Player::SECOND, domain::AutoZone::DECK, random_);

		// 抽牌
		field_playground.start_draw(domain::Player::FIRST);
		field_playground.start_draw(domain::Player::SECOND);

		// 开始回合
		field_turn.new_turn(domain::Player::FIRST);

		// 限制第一回合不能攻击
		field_turn.set_can_battle(false);

		// todo: 广播决斗开始
	}
}

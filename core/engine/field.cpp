#include <core/engine/field.hpp>

#include <core/engine/card.hpp>
#include <core/engine/duel.hpp>

namespace cg::engine
{
	auto Field::duel() const noexcept -> Duel&
	{
		return duel_.get();
	}

	Field::PlaygroundHandler::PlaygroundHandler(Field& field) noexcept
		: field_{field} {}

	auto Field::PlaygroundHandler::playground() noexcept -> Playground&
	{
		return field_.get().playground_;
	}

	auto Field::PlaygroundHandler::playground() const noexcept -> const Playground&
	{
		return field_.get().playground_;
	}

	auto Field::PlaygroundHandler::set_life_point(const domain::Player player, const domain::life_point_type life_point) noexcept -> void
	{
		playground().set_life_point(player, life_point);
	}

	auto Field::PlaygroundHandler::set_start_hand(const domain::Player player, const size_type start_hand) noexcept -> void
	{
		playground().set_start_hand(player, start_hand);
	}

	auto Field::PlaygroundHandler::set_draw_count(const domain::Player player, const size_type draw_count) noexcept -> void
	{
		playground().set_draw_count(player, draw_count);
	}

	auto Field::PlaygroundHandler::life_point(const domain::Player player) const noexcept -> domain::life_point_type
	{
		return playground().life_point(player);
	}

	auto Field::PlaygroundHandler::start_hand(const domain::Player player) const noexcept -> size_type
	{
		return playground().start_hand(player);
	}

	auto Field::PlaygroundHandler::draw_count(const domain::Player player) const noexcept -> size_type
	{
		return playground().draw_count(player);
	}

	auto Field::PlaygroundHandler::affordable(const domain::Player player, const domain::life_point_type delta) const noexcept -> bool
	{
		return playground().affordable(player, delta);
	}

	auto Field::PlaygroundHandler::afford(const domain::Player player, const domain::life_point_type delta) noexcept -> void
	{
		playground().afford(player, delta);

		if (life_point(player) <= 0)
		{
			// 生命值不足
			// todo: 决斗结束
		}
	}

	auto Field::PlaygroundHandler::start_draw(const domain::Player player) noexcept -> void
	{
		const auto draws = playground().start_draw(player);
		// 应该不需要考虑起手牌不够的情况吧? :)
		std::ignore = draws;
	}

	auto Field::PlaygroundHandler::draw(const domain::Player player, const size_type count) noexcept -> void
	{
		const auto draws = playground().draw(player, count);

		// todo: 广播抽到的卡
		// 例如 霸王龙Z-ARC[13331639] 的灵摆效果可以破坏刚刚抽到的所有卡
		std::ignore = draws;
	}

	auto Field::PlaygroundHandler::add(const domain::Player player, Card& card, const domain::AutoZone zone) noexcept -> void
	{
		playground().add(player, card, zone);
	}

	auto Field::PlaygroundHandler::shuffle(const domain::Player player, const domain::AutoZone zone, utility::Random& random) noexcept -> void
	{
		playground().shuffle(player, zone, random);

		// todo: 广播洗牌
	}

	auto Field::PlaygroundHandler::reverse(const domain::Player player, const domain::AutoZone zone) noexcept -> void
	{
		playground().reverse(player, zone);

		// todo: 广播反转
	}

	auto Field::PlaygroundHandler::select(const domain::Player player, const domain::Zone zone) const noexcept -> CardOptional
	{
		return playground().select(player, zone);
	}

	auto Field::PlaygroundHandler::occupied(const domain::Player player, const domain::Zone zone) const noexcept -> bool
	{
		return playground().occupied(player, zone);
	}

	auto Field::PlaygroundHandler::count(const domain::Player player, const domain::Zone zone) const noexcept -> size_type
	{
		return playground().count(player, zone);
	}

	auto Field::PlaygroundHandler::count(const domain::Player player, const domain::AutoZone zone) const noexcept -> size_type
	{
		return playground().count(player, zone);
	}

	auto Field::PlaygroundHandler::count(const domain::Player player, const domain::FieldZone zone) const noexcept -> size_type
	{
		return playground().count(player, zone);
	}

	auto Field::PlaygroundHandler::free(const domain::Player player, const domain::Zone zone) const noexcept -> std::vector<Playground::size_type>
	{
		return playground().free(player, zone);
	}

	auto Field::PlaygroundHandler::free(const domain::Player player, const domain::FieldZone zone) const noexcept -> std::vector<Playground::size_type>
	{
		return playground().free(player, zone);
	}

	auto Field::PlaygroundHandler::move_card(const CardReference card, const domain::Player player, const domain::Zone zone, const domain::Reason reason) noexcept -> bool
	{
		std::ignore = card;
		std::ignore = player;
		std::ignore = zone;
		std::ignore = reason;

		return true;
	}

	Field::TossInfoHandler::TossInfoHandler(Field& field) noexcept
		: field_{field} {}

	auto Field::TossInfoHandler::toss_info() noexcept -> TossInfo&
	{
		return field_.get().toss_;
	}

	auto Field::TossInfoHandler::toss_info() const noexcept -> const TossInfo&
	{
		return field_.get().toss_;
	}

	auto Field::TossInfoHandler::toss_dice(const domain::Player player, const TossInfo::size_type count, utility::Random& random) noexcept -> void
	{
		auto& ti = toss_info();

		// 掷骰子
		ti.toss_dice(count, random);

		// todo: 效果替换?
		ti.dice_results[0] = domain::DiceValue::ONE;

		// 广播掷骰子
		field_.get().game_events().emit(domain::GameEvent::TossDice{.player = player, .results = ti.dice_results});
	}

	auto Field::TossInfoHandler::toss_coin(const domain::Player player, const TossInfo::size_type count, utility::Random& random) noexcept -> void
	{
		auto& ti = toss_info();

		// 掷硬币
		ti.toss_coin(count, random);

		// todo: 效果替换?
		ti.coin_results[0] = domain::CoinSide::HEADS;

		// 广播掷硬币
		field_.get().game_events().emit(domain::GameEvent::TossCoin{.player = player, .results = ti.coin_results});
	}

	Field::TurnHandler::TurnHandler(Field& field) noexcept
		: field_{field} {}

	auto Field::TurnHandler::turn_info() noexcept -> TurnInfo&
	{
		return field_.get().turn_;
	}

	auto Field::TurnHandler::turn_info() const noexcept -> const TurnInfo&
	{
		return field_.get().turn_;
	}

	auto Field::TurnHandler::set_turn_id(const domain::TurnId id) noexcept -> void
	{
		auto& ti = turn_info();

		ti.id = id;
	}

	auto Field::TurnHandler::set_phase(const domain::Phase phase) noexcept -> void
	{
		auto& ti = turn_info();

		ti.phase = phase;
	}

	auto Field::TurnHandler::set_turn_phase(const domain::TurnPhase turn_phase) noexcept -> void
	{
		auto& ti = turn_info();

		ti.phase.turn = turn_phase;
	}

	auto Field::TurnHandler::set_battle_step(const domain::BattleStep battle_step) noexcept -> void
	{
		auto& ti = turn_info();

		ti.phase.battle = battle_step;
	}

	auto Field::TurnHandler::set_damage_step(const domain::DamageStep damage_step) noexcept -> void
	{
		auto& ti = turn_info();

		ti.phase.damage = damage_step;
	}

	auto Field::TurnHandler::set_player(const domain::Player player) noexcept -> void
	{
		auto& ti = turn_info();

		ti.player = player;
	}

	auto Field::TurnHandler::set_can_battle(const bool can_battle) noexcept -> void
	{
		auto& ti = turn_info();

		ti.can_battle = can_battle;
	}

	auto Field::TurnHandler::set_can_normal_summon(const bool can_normal_summon) noexcept -> void
	{
		auto& ti = turn_info();

		ti.can_normal_summon = can_normal_summon;
	}

	auto Field::TurnHandler::set_can_special_summon(const bool can_special_summon) noexcept -> void
	{
		auto& ti = turn_info();

		ti.can_special_summon = can_special_summon;
	}

	auto Field::TurnHandler::turn_id() const noexcept -> domain::TurnId
	{
		auto& ti = turn_info();

		return ti.id;
	}

	auto Field::TurnHandler::phase() const noexcept -> domain::Phase
	{
		auto& ti = turn_info();

		return ti.phase;
	}

	auto Field::TurnHandler::turn_phase() const noexcept -> domain::TurnPhase
	{
		auto& ti = turn_info();

		return ti.phase.turn;
	}

	auto Field::TurnHandler::battle_step() const noexcept -> domain::BattleStep
	{
		auto& ti = turn_info();

		return ti.phase.battle;
	}

	auto Field::TurnHandler::damage_step() const noexcept -> domain::DamageStep
	{
		auto& ti = turn_info();

		return ti.phase.damage;
	}

	auto Field::TurnHandler::can_battle() const noexcept -> bool
	{
		auto& ti = turn_info();

		return ti.can_battle;
	}

	auto Field::TurnHandler::can_normal_summon() const noexcept -> bool
	{
		auto& ti = turn_info();

		return ti.can_normal_summon;
	}

	auto Field::TurnHandler::can_special_summon() const noexcept -> bool
	{
		auto& ti = turn_info();

		return ti.can_special_summon;
	}

	auto Field::TurnHandler::is_turn_id(const domain::TurnId expected_id) const noexcept -> bool
	{
		return turn_id() == expected_id;
	}

	auto Field::TurnHandler::is_phase(const domain::Phase expected_phase) const noexcept -> bool
	{
		return phase() == expected_phase;
	}

	auto Field::TurnHandler::is_turn_phase(const domain::TurnPhase expected_turn_phase) const noexcept -> bool
	{
		return turn_phase() == expected_turn_phase;
	}

	auto Field::TurnHandler::is_battle_step(const domain::BattleStep expected_battle_step) const noexcept -> bool
	{
		return battle_step() == expected_battle_step;
	}

	auto Field::TurnHandler::is_damage_step(const domain::DamageStep expected_damage_step) const noexcept -> bool
	{
		return damage_step() == expected_damage_step;
	}

	auto Field::TurnHandler::new_turn(const domain::Player player) noexcept -> void
	{
		turn_info().on_turn_end(domain::opponent_of(player));
		turn_info().on_turn_begin(player);

		// todo: 广播抽卡阶段开始
	}

	auto Field::TurnHandler::advance_phase(const domain::TurnPhase turn_phase) noexcept -> void
	{
		turn_info().advance_phase(turn_phase);

		// todo: 广播指定阶段开始
	}

	Field::EventHandler::EventHandler(Field& field) noexcept
		: field_{field} {}

	auto Field::EventHandler::events() noexcept -> Events&
	{
		return field_.get().events_;
	}

	auto Field::EventHandler::events() const noexcept -> const Events&
	{
		return field_.get().events_;
	}

	auto Field::EventHandler::raise(const Event& event) noexcept -> void
	{
		events().raise(event);
	}

	auto Field::EventHandler::triggered(const domain::EventType type) const noexcept -> bool
	{
		return events().triggered(type);
	}

	Field::GameEventHandler::GameEventHandler(Field& field) noexcept
		: field_{field} {}

	auto Field::GameEventHandler::events() noexcept -> GameEvents&
	{
		return field_.get().game_events_;
	}

	auto Field::GameEventHandler::events() const noexcept -> const GameEvents&
	{
		return field_.get().game_events_;
	}

	auto Field::GameEventHandler::emit(domain::GameEvent event) noexcept -> void
	{
		events().emit(std::move(event));
	}

	Field::ChainHandler::ChainHandler(Field& field) noexcept
		: field_{field} {}

	auto Field::ChainHandler::chain() noexcept -> ChainLink&
	{
		return field_.get().chain_;
	}

	auto Field::ChainHandler::chain() const noexcept -> const ChainLink&
	{
		return field_.get().chain_;
	}

	Field::Field(Duel& duel) noexcept
		: duel_{duel}
	// playground_{},
	// toss_{},
	// turn_{},
	// events_{},
	// game_events_{}
	{
		//
	}

	auto Field::playground() noexcept -> PlaygroundHandler
	{
		return PlaygroundHandler{*this};
	}

	auto Field::random() noexcept -> TossInfoHandler
	{
		return TossInfoHandler{*this};
	}

	auto Field::turn() noexcept -> TurnHandler
	{
		return TurnHandler{*this};
	}

	auto Field::events() noexcept -> EventHandler
	{
		return EventHandler{*this};
	}

	auto Field::game_events() noexcept -> GameEventHandler
	{
		return GameEventHandler{*this};
	}
}

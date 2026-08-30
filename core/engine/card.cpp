#include <core/engine/card.hpp>

namespace cg::engine
{
	Card::StateHandler::StateHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::StateHandler::state() noexcept -> CardState&
	{
		return card_.get().state_;
	}

	auto Card::StateHandler::state() const noexcept -> const CardState&
	{
		return card_.get().state_;
	}

	auto Card::StateHandler::set_controller(const domain::Player controller) noexcept -> void
	{
		state().controller = controller;
	}

	auto Card::StateHandler::set_zone(const domain::Zone zone) noexcept -> void
	{
		state().zone = zone;
	}

	auto Card::StateHandler::set_form(const domain::FieldZoneForm form) noexcept -> void
	{
		state().form = form;
	}

	auto Card::StateHandler::set_reason_player(const domain::Player player) noexcept -> void
	{
		state().reason_player = player;
	}

	auto Card::StateHandler::set_reason_card(const CardOptional card) noexcept -> void
	{
		state().reason_card = card;
	}

	auto Card::StateHandler::set_reason_effect(const EffectOptional effect) noexcept -> void
	{
		state().reason_effect = effect;
	}

	auto Card::StateHandler::set_reason(const domain::ReasonWrapper reason) noexcept -> void
	{
		state().reason = reason;
	}

	auto Card::StateHandler::controller() const noexcept -> domain::Player
	{
		return state().controller;
	}

	auto Card::StateHandler::zone() const noexcept -> domain::Zone
	{
		return state().zone;
	}

	auto Card::StateHandler::form() const noexcept -> domain::FieldZoneForm
	{
		return state().form;
	}

	auto Card::StateHandler::reason_player() const noexcept -> domain::Player
	{
		return state().reason_player;
	}

	auto Card::StateHandler::reason_card() const noexcept -> CardOptional
	{
		return state().reason_card;
	}

	auto Card::StateHandler::reason_effect() const noexcept -> EffectOptional
	{
		return state().reason_effect;
	}

	auto Card::StateHandler::reason() const noexcept -> domain::ReasonWrapper
	{
		return state().reason;
	}

	auto Card::StateHandler::is_controller(const domain::Player expected_player) const noexcept -> bool
	{
		return controller() == expected_player;
	}

	auto Card::StateHandler::is_zone(const domain::Zone expected_zone) const noexcept -> bool
	{
		return zone() == expected_zone;
	}

	auto Card::StateHandler::is_form(const domain::FieldZoneForm expected_form) const noexcept -> bool
	{
		return form() == expected_form;
	}

	auto Card::StateHandler::is_reason_player(const domain::Player expected_player) const noexcept -> bool
	{
		return reason_player() == expected_player;
	}

	auto Card::StateHandler::is_reason_card(const CardOptional expected_card) const noexcept -> bool
	{
		return reason_card() == expected_card;
	}

	auto Card::StateHandler::is_reason_effect(const EffectOptional expected_effect) const noexcept -> bool
	{
		return reason_effect() == expected_effect;
	}

	auto Card::StateHandler::is_reason(const domain::ReasonWrapper expected_reason) const noexcept -> bool
	{
		return reason().all_of(expected_reason);
	}

	auto Card::StateHandler::has_reason(const domain::ReasonWrapper expected_reason) const noexcept -> bool
	{
		return reason().any_of(expected_reason);
	}

	auto Card::StateHandler::is_field_zone() const noexcept -> bool
	{
		return domain::is_field_zone(zone());
	}

	auto Card::StateHandler::is_face_up_form() const noexcept -> bool
	{
		return domain::is_face_up_form(form());
	}

	auto Card::StateHandler::is_face_down_form() const noexcept -> bool
	{
		return domain::is_face_down_form(form());
	}

	auto Card::StateHandler::is_attack_form() const noexcept -> bool
	{
		return domain::is_attack_form(form());
	}

	auto Card::StateHandler::is_defense_form() const noexcept -> bool
	{
		return domain::is_defense_form(form());
	}

	Card::SummonInfoHandler::SummonInfoHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::SummonInfoHandler::summon_info() noexcept -> SummonInfo&
	{
		return card_.get().summon_info_;
	}

	auto Card::SummonInfoHandler::summon_info() const noexcept -> const SummonInfo&
	{
		return card_.get().summon_info_;
	}

	auto Card::SummonInfoHandler::set_kind(const domain::SummonKind kind) noexcept -> void
	{
		summon_info().kind = kind;
	}

	auto Card::SummonInfoHandler::set_from_zone(const domain::Zone from_zone) noexcept -> void
	{
		summon_info().from_zone = from_zone;
	}

	auto Card::SummonInfoHandler::set_player(const domain::Player player) noexcept -> void
	{
		summon_info().player = player;
	}

	auto Card::SummonInfoHandler::set_turn_index(const turn_index_type turn_index) noexcept -> void
	{
		summon_info().turn_index = turn_index;
	}

	auto Card::SummonInfoHandler::set_materials(Group group) noexcept -> void
	{
		summon_info().materials = std::move(group);
	}

	auto Card::SummonInfoHandler::kind() const noexcept -> domain::SummonKind
	{
		return summon_info().kind;
	}

	auto Card::SummonInfoHandler::from_zone() const noexcept -> domain::Zone
	{
		return summon_info().from_zone;
	}

	auto Card::SummonInfoHandler::player() const noexcept -> domain::Player
	{
		return summon_info().player;
	}

	auto Card::SummonInfoHandler::turn_index() const noexcept -> turn_index_type
	{
		return summon_info().turn_index;
	}

	auto Card::SummonInfoHandler::materials() const noexcept -> GroupView
	{
		return GroupView{summon_info().materials};
	}

	auto Card::SummonInfoHandler::is_kind(const domain::SummonKind expected_kind) const noexcept -> bool
	{
		return kind() == expected_kind;
	}

	auto Card::SummonInfoHandler::is_from_zone(const domain::Zone expected_from_zone) const noexcept -> bool
	{
		return from_zone() == expected_from_zone;
	}

	auto Card::SummonInfoHandler::is_player(const domain::Player expected_player) const noexcept -> bool
	{
		return player() == expected_player;
	}

	auto Card::SummonInfoHandler::is_turn_index(const turn_index_type expected_turn_index) const noexcept -> bool
	{
		return turn_index() == expected_turn_index;
	}

	auto Card::SummonInfoHandler::has_material(const Card& card) const noexcept -> bool
	{
		return summon_info().materials.contains(card);
	}

	auto Card::SummonInfoHandler::material_count() const noexcept -> std::size_t
	{
		return summon_info().materials.size();
	}

	Card::BattleInfoHandler::BattleInfoHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::BattleInfoHandler::battle_info() noexcept -> BattleInfo&
	{
		return card_.get().battle_info_;
	}

	auto Card::BattleInfoHandler::battle_info() const noexcept -> const BattleInfo&
	{
		return card_.get().battle_info_;
	}

	auto Card::BattleInfoHandler::record_attack_announced() noexcept -> void
	{
		auto& bi = battle_info();

		bi.attack_announced_count += 1;
		// todo: 获取当前回合号
		bi.attack_turn_index = 0;
	}

	auto Card::BattleInfoHandler::record_attack_canceled() noexcept -> void
	{
		auto& bi = battle_info();

		// todo: 获取当前回合号
		bi.attack_canceled_turn_index = 0;
	}

	auto Card::BattleInfoHandler::record_attacked_card(Card& card) noexcept -> void
	{
		auto& bi = battle_info();

		bi.attacked_cards.insert(card);
		bi.battled_cards.insert(card);
		bi.attacked_count += 1;
	}

	auto Card::BattleInfoHandler::attacked_cards() const noexcept -> GroupView
	{
		return GroupView{battle_info().attacked_cards};
	}

	auto Card::BattleInfoHandler::attacked_count() const noexcept -> std::size_t
	{
		return battle_info().attacked_count;
	}

	auto Card::BattleInfoHandler::attack_announced_count() const noexcept -> std::size_t
	{
		return battle_info().attack_announced_count;
	}

	auto Card::BattleInfoHandler::attack_turn_index() const noexcept -> turn_index_type
	{
		return battle_info().attack_turn_index;
	}

	auto Card::BattleInfoHandler::attack_canceled_turn_index() const noexcept -> turn_index_type
	{
		return battle_info().attack_canceled_turn_index;
	}

	auto Card::BattleInfoHandler::is_attacked_this_turn() const noexcept -> bool
	{
		// todo: 获取当前回合号
		return attack_turn_index() == 0;
	}

	auto Card::BattleInfoHandler::is_attack_canceled_this_turn() const noexcept -> bool
	{
		// todo: 获取当前回合号
		return attack_canceled_turn_index() == 0;
	}

	Card::TargetInfoHandler::TargetInfoHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::TargetInfoHandler::target_info() noexcept -> TargetInfo&
	{
		return card_.get().target_info_;
	}

	auto Card::TargetInfoHandler::target_info() const noexcept -> const TargetInfo&
	{
		return card_.get().target_info_;
	}

	auto Card::TargetInfoHandler::set_owner_target(Card& card) noexcept -> bool
	{
		return target_info().owner_targets.insert(card);
	}

	auto Card::TargetInfoHandler::cancel_owner_target(const Card& card) noexcept -> bool
	{
		return target_info().owner_targets.erase(card);
	}

	auto Card::TargetInfoHandler::set_target(Card& card) noexcept -> bool
	{
		if (target_info().card_targets.insert(card))
		{
			// 将本卡加入目标卡的"以本卡为对象的卡"列表中
			card.target_info().set_owner_target(card_);

			// TODO: 发出事件,广播该卡被设为目标

			return true;
		}

		return false;
	}

	auto Card::TargetInfoHandler::cancel_target(Card& card) noexcept -> bool
	{
		if (target_info().card_targets.erase(card))
		{
			// 将本卡移出目标卡的"以本卡为对象的卡"列表中
			card.target_info().cancel_owner_target(card_);

			return true;
		}

		return false;
	}

	auto Card::TargetInfoHandler::card_targets() const noexcept -> GroupView
	{
		return GroupView{target_info().card_targets};
	}

	auto Card::TargetInfoHandler::owner_targets() const noexcept -> GroupView
	{
		return GroupView{target_info().owner_targets};
	}

	auto Card::TargetInfoHandler::has_target() const noexcept -> bool
	{
		return !target_info().card_targets.empty();
	}

	auto Card::TargetInfoHandler::has_target(const Card& card) const noexcept -> bool
	{
		return target_info().card_targets.contains(card);
	}

	auto Card::TargetInfoHandler::target_count() const noexcept -> std::size_t
	{
		return target_info().card_targets.size();
	}

	auto Card::instance_id() const noexcept -> card_instance_id_type
	{
		return instance_id_;
	}

	auto Card::state() noexcept -> StateHandler
	{
		return StateHandler{*this};
	}

	auto Card::summon_info() noexcept -> SummonInfoHandler
	{
		return SummonInfoHandler{*this};
	}

	auto Card::battle_info() noexcept -> BattleInfoHandler
	{
		return BattleInfoHandler{*this};
	}

	auto Card::target_info() noexcept -> TargetInfoHandler
	{
		return TargetInfoHandler{*this};
	}
}

#include <core/engine/card.hpp>

namespace cg::engine
{
	Card::PrototypeHandler::PrototypeHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::PrototypeHandler::prototype() const noexcept -> const Prototype&
	{
		return card_.get().prototype_.get();
	}

	auto Card::PrototypeHandler::code() const noexcept -> domain::CardCode
	{
		return prototype().code();
	}

	auto Card::PrototypeHandler::code_rule() const noexcept -> domain::CardCode
	{
		return prototype().code_rule();
	}

	auto Card::PrototypeHandler::series() const noexcept -> std::array<domain::SeriesCode, Prototype::max_series_count>
	{
		return prototype().series();
	}

	auto Card::PrototypeHandler::card_type() const noexcept -> domain::CardTypeWrapper
	{
		return prototype().card_type();
	}

	auto Card::PrototypeHandler::attribute() const noexcept -> domain::AttributeWrapper
	{
		return prototype().attribute();
	}

	auto Card::PrototypeHandler::race() const noexcept -> domain::Race
	{
		return prototype().race();
	}

	auto Card::PrototypeHandler::level() const noexcept -> domain::LevelWrapper
	{
		return prototype().level();
	}

	auto Card::PrototypeHandler::rank() const noexcept -> domain::Rank
	{
		return prototype().rank();
	}

	auto Card::PrototypeHandler::link_marker() const noexcept -> domain::LinkMarkerWrapper
	{
		return prototype().link_marker();
	}

	auto Card::PrototypeHandler::attack() const noexcept -> domain::attack_defense_value_type
	{
		return prototype().attack();
	}

	auto Card::PrototypeHandler::defense() const noexcept -> domain::attack_defense_value_type
	{
		return prototype().defense();
	}

	auto Card::PrototypeHandler::left_pendulum() const noexcept -> domain::PendulumScale
	{
		return prototype().left_pendulum();
	}

	auto Card::PrototypeHandler::right_pendulum() const noexcept -> domain::PendulumScale
	{
		return prototype().right_pendulum();
	}

	auto Card::PrototypeHandler::is_card_type(const domain::CardTypeWrapper expected_card_type) const noexcept -> bool
	{
		return card_type().all_of(expected_card_type);
	}

	auto Card::PrototypeHandler::has_card_type(const domain::CardTypeWrapper expected_card_type) const noexcept -> bool
	{
		return card_type().any_of(expected_card_type);
	}

	Card::PropertyHandler::PropertyHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::PropertyHandler::prototype() const noexcept -> PrototypeHandler
	{
		return card_.get().prototype();
	}

	auto Card::PropertyHandler::card_type() const noexcept -> domain::CardTypeWrapper
	{
		const auto prototype = card_.get().prototype();
		auto type = prototype.card_type();

		// todo: 应用效果
		type |= domain::CardType::TOKEN_MONSTER;

		return type;
	}

	auto Card::PropertyHandler::attribute() const noexcept -> domain::AttributeWrapper
	{
		const auto prototype = card_.get().prototype();
		auto attribute = prototype.attribute();

		// todo: 应用效果
		attribute |= domain::Attribute::DIVINE;

		return attribute;
	}

	auto Card::PropertyHandler::race() const noexcept -> domain::Race
	{
		const auto prototype = card_.get().prototype();
		auto race = prototype.race();

		// todo: 应用效果
		if (race != domain::Race::WARRIOR)
		{
			race = domain::Race::WARRIOR;
		}

		return race;
	}

	auto Card::PropertyHandler::level() const noexcept -> domain::LevelWrapper
	{
		// 超量和连接卡没有等级
		constexpr auto invalid_card_type = []
		{
			domain::CardTypeWrapper w{domain::CardType::NONE};

			// 超量
			w |= domain::CardType::XYZ;
			// 连接
			w |= domain::CardType::LINK;

			return w;
		}();

		// todo: 如何处理?
		if (has_card_type(invalid_card_type))
		{
			return {domain::Level{0}};
		}

		auto level = prototype().level();

		// todo: 应用效果
		level += domain::Level::L1;

		return level;
	}

	auto Card::PropertyHandler::rank() const noexcept -> domain::Rank
	{
		// 只有超量卡有阶级

		// todo: 如何处理?
		if (!has_card_type({domain::CardType::XYZ}))
		{
			return domain::Rank::R1;
		}

		auto rank = prototype().rank();

		// todo: 应用效果
		if (rank != domain::Rank::R1)
		{
			rank = domain::Rank::R1;
		}

		return rank;
	}

	auto Card::PropertyHandler::link_marker() const noexcept -> domain::LinkMarkerWrapper
	{
		// 只有连接卡有连接箭头

		// todo: 如何处理?
		if (!has_card_type({domain::CardType::LINK}))
		{
			return {domain::LinkMarker::TOP};
		}

		auto link_marker = prototype().link_marker();

		// todo: 应用效果
		link_marker |= domain::LinkMarker::TOP;

		return link_marker;
	}

	auto Card::PropertyHandler::attack() const noexcept -> domain::attack_defense_value_type
	{
		auto attack = prototype().attack();

		// todo: 应用效果
		attack += 9990;

		return attack;
	}

	auto Card::PropertyHandler::defense() const noexcept -> domain::attack_defense_value_type
	{
		auto defense = prototype().defense();

		// todo: 应用效果
		defense += 9990;

		return defense;
	}

	auto Card::PropertyHandler::left_pendulum() const noexcept -> domain::PendulumScale
	{
		// 只有灵摆怪兽有灵摆刻度

		// todo: 如何处理?
		if (!has_card_type({domain::CardType::PENDULUM}))
		{
			return domain::PendulumScale::PS0;
		}

		auto left_pendulum = prototype().left_pendulum();

		// todo: 应用效果
		if (left_pendulum != domain::PendulumScale::PS0)
		{
			left_pendulum = domain::PendulumScale::PS0;
		}

		return left_pendulum;
	}

	auto Card::PropertyHandler::right_pendulum() const noexcept -> domain::PendulumScale
	{
		// 只有灵摆怪兽有灵摆刻度

		// todo: 如何处理?
		if (!has_card_type({domain::CardType::PENDULUM}))
		{
			return domain::PendulumScale::PS0;
		}

		auto right_pendulum = prototype().right_pendulum();

		// todo: 应用效果
		if (right_pendulum != domain::PendulumScale::PS0)
		{
			right_pendulum = domain::PendulumScale::PS0;
		}

		return right_pendulum;
	}

	auto Card::PropertyHandler::is_card_type(const domain::CardTypeWrapper expected_card_type) const noexcept -> bool
	{
		return prototype().is_card_type(expected_card_type);
	}

	auto Card::PropertyHandler::has_card_type(const domain::CardTypeWrapper expected_card_type) const noexcept -> bool
	{
		return prototype().has_card_type(expected_card_type);
	}

	Card::StateHandler::StateHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::StateHandler::state() noexcept -> State&
	{
		return card_.get().state_;
	}

	auto Card::StateHandler::state() const noexcept -> const State&
	{
		return card_.get().state_;
	}

	auto Card::StateHandler::set_controller(const domain::Player controller) noexcept -> void
	{
		state().set_controller(controller);
	}

	auto Card::StateHandler::set_zone(const domain::Zone zone) noexcept -> void
	{
		state().set_zone(zone);
	}

	auto Card::StateHandler::set_reason_player(const domain::Player reason_player) noexcept -> void
	{
		state().set_reason_player(reason_player);
	}

	auto Card::StateHandler::set_reason_card(const CardOptional reason_card) noexcept -> void
	{
		state().set_reason_card(reason_card);
	}

	auto Card::StateHandler::set_reason_effect(const EffectOptional reason_effect) noexcept -> void
	{
		state().set_reason_effect(reason_effect);
	}

	auto Card::StateHandler::set_reason(const domain::ReasonWrapper reason) noexcept -> void
	{
		state().set_reason(reason);
	}

	auto Card::StateHandler::controller() const noexcept -> domain::Player
	{
		return state().controller();
	}

	auto Card::StateHandler::zone() const noexcept -> domain::Zone
	{
		return state().zone();
	}

	auto Card::StateHandler::reason_player() const noexcept -> domain::Player
	{
		return state().reason_player();
	}

	auto Card::StateHandler::reason_card() const noexcept -> CardOptional
	{
		return state().reason_card();
	}

	auto Card::StateHandler::reason_effect() const noexcept -> EffectOptional
	{
		return state().reason_effect();
	}

	auto Card::StateHandler::reason() const noexcept -> domain::ReasonWrapper
	{
		return state().reason();
	}

	auto Card::StateHandler::is_controller(const domain::Player expected_player) const noexcept -> bool
	{
		return state().is_controller(expected_player);
	}

	auto Card::StateHandler::is_zone(const domain::Zone expected_zone) const noexcept -> bool
	{
		return state().is_zone(expected_zone);
	}

	auto Card::StateHandler::is_reason_player(const domain::Player expected_player) const noexcept -> bool
	{
		return state().is_reason_player(expected_player);
	}

	auto Card::StateHandler::is_reason_card(const CardOptional expected_card) const noexcept -> bool
	{
		return state().is_reason_card(expected_card);
	}

	auto Card::StateHandler::is_reason_effect(const EffectOptional expected_effect) const noexcept -> bool
	{
		return state().is_reason_effect(expected_effect);
	}

	auto Card::StateHandler::is_reason(const domain::ReasonWrapper expected_reason) const noexcept -> bool
	{
		return state().is_reason(expected_reason);
	}

	auto Card::StateHandler::has_reason(const domain::ReasonWrapper expected_reason) const noexcept -> bool
	{
		return state().has_reason(expected_reason);
	}

	auto Card::StateHandler::is_field_zone() const noexcept -> bool
	{
		return state().is_field_zone();
	}

	auto Card::StateHandler::is_face_up_form() const noexcept -> bool
	{
		return state().is_face_up_form();
	}

	auto Card::StateHandler::is_face_down_form() const noexcept -> bool
	{
		return state().is_face_down_form();
	}

	auto Card::StateHandler::is_attack_form() const noexcept -> bool
	{
		return state().is_attack_form();
	}

	auto Card::StateHandler::is_defense_form() const noexcept -> bool
	{
		return state().is_defense_form();
	}

	Card::SummonInfoHandler::SummonInfoHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::SummonInfoHandler::summon_info() noexcept -> SummonInfo&
	{
		return card_.get().summon_;
	}

	auto Card::SummonInfoHandler::summon_info() const noexcept -> const SummonInfo&
	{
		return card_.get().summon_;
	}

	auto Card::SummonInfoHandler::set_kind(const domain::SummonKind kind) noexcept -> void
	{
		summon_info().set_kind(kind);
	}

	auto Card::SummonInfoHandler::set_from_zone(const domain::Zone from_zone) noexcept -> void
	{
		summon_info().set_from_zone(from_zone);
	}

	auto Card::SummonInfoHandler::set_player(const domain::Player player) noexcept -> void
	{
		summon_info().set_player(player);
	}

	auto Card::SummonInfoHandler::set_turn_id(const domain::TurnId turn_id) noexcept -> void
	{
		summon_info().set_turn_id(turn_id);
	}

	auto Card::SummonInfoHandler::set_materials(Group materials) noexcept -> void
	{
		summon_info().set_materials(std::move(materials));
	}

	auto Card::SummonInfoHandler::kind() const noexcept -> domain::SummonKind
	{
		return summon_info().kind();
	}

	auto Card::SummonInfoHandler::from_zone() const noexcept -> domain::Zone
	{
		return summon_info().from_zone();
	}

	auto Card::SummonInfoHandler::player() const noexcept -> domain::Player
	{
		return summon_info().player();
	}

	auto Card::SummonInfoHandler::turn_id() const noexcept -> domain::TurnId
	{
		return summon_info().turn_id();
	}

	auto Card::SummonInfoHandler::materials() const noexcept -> View
	{
		return summon_info().materials();
	}

	auto Card::SummonInfoHandler::is_kind(const domain::SummonKind expected_kind) const noexcept -> bool
	{
		return summon_info().is_kind(expected_kind);
	}

	auto Card::SummonInfoHandler::is_from_zone(const domain::Zone expected_from_zone) const noexcept -> bool
	{
		return summon_info().is_from_zone(expected_from_zone);
	}

	auto Card::SummonInfoHandler::is_player(const domain::Player expected_player) const noexcept -> bool
	{
		return summon_info().is_player(expected_player);
	}

	auto Card::SummonInfoHandler::is_turn_id(const domain::TurnId expected_turn_id) const noexcept -> bool
	{
		return summon_info().is_turn_id(expected_turn_id);
	}

	auto Card::SummonInfoHandler::has_material(const Card& card) const noexcept -> bool
	{
		return summon_info().has_material(card);
	}

	auto Card::SummonInfoHandler::material_count() const noexcept -> std::size_t
	{
		return summon_info().material_count();
	}

	Card::BattleInfoHandler::BattleInfoHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::BattleInfoHandler::battle_info() noexcept -> BattleInfo&
	{
		return card_.get().battle_;
	}

	auto Card::BattleInfoHandler::battle_info() const noexcept -> const BattleInfo&
	{
		return card_.get().battle_;
	}

	auto Card::BattleInfoHandler::record_attack_announced() noexcept -> void
	{
		battle_info().record_attack_announced();
	}

	auto Card::BattleInfoHandler::record_attack_canceled() noexcept -> void
	{
		battle_info().record_attack_canceled();
	}

	auto Card::BattleInfoHandler::record_attacked_card(Card& card) noexcept -> void
	{
		battle_info().record_attacked_card(card);
	}

	auto Card::BattleInfoHandler::attacked_cards() const noexcept -> View
	{
		return battle_info().attacked_cards();
	}

	auto Card::BattleInfoHandler::attacked_count() const noexcept -> BattleInfo::size_type
	{
		return battle_info().attacked_count();
	}

	auto Card::BattleInfoHandler::attack_announced_count() const noexcept -> BattleInfo::size_type
	{
		return battle_info().attack_announced_count();
	}

	auto Card::BattleInfoHandler::attack_turn_id() const noexcept -> domain::TurnId
	{
		return battle_info().attack_turn_id();
	}

	auto Card::BattleInfoHandler::attack_canceled_turn_id() const noexcept -> domain::TurnId
	{
		return battle_info().attack_canceled_turn_id();
	}

	auto Card::BattleInfoHandler::is_attacked_this_turn() const noexcept -> bool
	{
		return battle_info().is_attacked_this_turn();
	}

	auto Card::BattleInfoHandler::is_attack_canceled_this_turn() const noexcept -> bool
	{
		return battle_info().is_attack_canceled_this_turn();
	}

	Card::TargetInfoHandler::TargetInfoHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::TargetInfoHandler::target_info() noexcept -> TargetInfo&
	{
		return card_.get().target_;
	}

	auto Card::TargetInfoHandler::target_info() const noexcept -> const TargetInfo&
	{
		return card_.get().target_;
	}

	auto Card::TargetInfoHandler::set_target(Card& target) noexcept -> bool
	{
		return target_info().set_target(target, card_);
	}

	auto Card::TargetInfoHandler::cancel_target(Card& target) noexcept -> bool
	{
		return target_info().cancel_target(target, card_);
	}

	auto Card::TargetInfoHandler::card_targets() const noexcept -> View
	{
		return target_info().card_targets();
	}

	auto Card::TargetInfoHandler::owner_targets() const noexcept -> View
	{
		return target_info().owner_targets();
	}

	auto Card::TargetInfoHandler::has_target() const noexcept -> bool
	{
		return target_info().has_target();
	}

	auto Card::TargetInfoHandler::has_target(const Card& card) const noexcept -> bool
	{
		return target_info().has_target(card);
	}

	auto Card::TargetInfoHandler::target_count() const noexcept -> TargetInfo::size_type
	{
		return target_info().target_count();
	}

	Card::XyzInfoHandler::XyzInfoHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::XyzInfoHandler::xyz_info() noexcept -> XyzInfo&
	{
		return card_.get().xyz_;
	}

	auto Card::XyzInfoHandler::xyz_info() const noexcept -> const XyzInfo&
	{
		return card_.get().xyz_;
	}

	// ReSharper disable once CppMemberFunctionMayBeConst
	auto Card::XyzInfoHandler::add_overlay(Card& material) noexcept -> bool
	{
		return XyzInfo::add_overlay(material, card_);
	}

	// ReSharper disable once CppMemberFunctionMayBeConst
	auto Card::XyzInfoHandler::remove_overlay(Card& material) noexcept -> bool
	{
		return XyzInfo::remove_overlay(material, card_);
	}

	auto Card::XyzInfoHandler::materials() const noexcept -> View
	{
		return xyz_info().materials();
	}

	auto Card::XyzInfoHandler::overlay_target() const noexcept -> CardOptional
	{
		return xyz_info().overlay_target();
	}

	auto Card::XyzInfoHandler::has_material() const noexcept -> bool
	{
		return xyz_info().has_material();
	}

	auto Card::XyzInfoHandler::has_material(const Card& material) const noexcept -> bool
	{
		return xyz_info().has_material(material);
	}

	auto Card::XyzInfoHandler::material_count() const noexcept -> XyzInfo::size_type
	{
		return xyz_info().material_count();
	}

	Card::EquipInfoHandler::EquipInfoHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::EquipInfoHandler::equip_info() noexcept -> EquipInfo&
	{
		return card_.get().equip_;
	}

	auto Card::EquipInfoHandler::equip_info() const noexcept -> const EquipInfo&
	{
		return card_.get().equip_;
	}

	// ReSharper disable once CppMemberFunctionMayBeConst
	auto Card::EquipInfoHandler::add_equip(Card& equip) noexcept -> bool
	{
		return EquipInfo::add_equip(equip, card_);
	}

	// ReSharper disable once CppMemberFunctionMayBeConst
	auto Card::EquipInfoHandler::remove_equip(Card& equip) noexcept -> bool
	{
		return EquipInfo::remove_equip(equip, card_);
	}

	auto Card::EquipInfoHandler::can_equip() const noexcept -> bool
	{
		return EquipInfo::can_equip(card_);
	}

	auto Card::EquipInfoHandler::equips() const noexcept -> View
	{
		return equip_info().equips();
	}

	auto Card::EquipInfoHandler::owner() const noexcept -> CardOptional
	{
		return equip_info().owner();
	}

	auto Card::EquipInfoHandler::has_equip() const noexcept -> bool
	{
		return equip_info().has_equip();
	}

	auto Card::EquipInfoHandler::has_equip(const Card& equip) const noexcept -> bool
	{
		return equip_info().has_equip(equip);
	}

	auto Card::EquipInfoHandler::equip_count() const noexcept -> std::size_t
	{
		return equip_info().equip_count();
	}

	Card::Card(Duel& duel, const domain::CardInstanceId instance_id, const domain::Player owner, const Prototype& prototype) noexcept
		: duel_{duel},
		  instance_id_{instance_id},
		  owner_{owner},
		  prototype_{prototype}
	// state_{},
	// summon_{},
	// battle_{},
	// target_{},
	// xyz_{},
	// equip_{}
	{
		//
	}

	auto Card::instance_id() const noexcept -> domain::CardInstanceId
	{
		return instance_id_;
	}

	auto Card::prototype() noexcept -> PrototypeHandler
	{
		return PrototypeHandler{*this};
	}

	auto Card::property() noexcept -> PropertyHandler
	{
		return PropertyHandler{*this};
	}

	auto Card::state() noexcept -> StateHandler
	{
		return StateHandler{*this};
	}

	auto Card::summon() noexcept -> SummonInfoHandler
	{
		return SummonInfoHandler{*this};
	}

	auto Card::battle() noexcept -> BattleInfoHandler
	{
		return BattleInfoHandler{*this};
	}

	auto Card::target() noexcept -> TargetInfoHandler
	{
		return TargetInfoHandler{*this};
	}

	auto Card::xyz() noexcept -> XyzInfoHandler
	{
		return XyzInfoHandler{*this};
	}

	auto Card::equip() noexcept -> EquipInfoHandler
	{
		return EquipInfoHandler{*this};
	}
}

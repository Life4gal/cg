#include <core/engine/card.hpp>

namespace cg::engine
{
	Card::PrototypeHandler::PrototypeHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::PrototypeHandler::prototype_data() const noexcept -> const Prototype&
	{
		return card_.get().prototype_.get();
	}

	auto Card::PrototypeHandler::code() const noexcept -> card_code_type
	{
		return prototype_data().code;
	}

	auto Card::PrototypeHandler::code_rule() const noexcept -> card_code_type
	{
		return prototype_data().duel_code();
	}

	auto Card::PrototypeHandler::series() const noexcept -> std::array<series_code_type, Prototype::max_series_count>
	{
		return prototype_data().series;
	}

	auto Card::PrototypeHandler::card_type() const noexcept -> domain::CardTypeWrapper
	{
		return prototype_data().card_type;
	}

	auto Card::PrototypeHandler::attribute() const noexcept -> domain::AttributeWrapper
	{
		return prototype_data().attribute;
	}

	auto Card::PrototypeHandler::race() const noexcept -> domain::Race
	{
		return prototype_data().race;
	}

	auto Card::PrototypeHandler::level() const noexcept -> domain::LevelWrapper
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

		return prototype_data().level;
	}

	auto Card::PrototypeHandler::rank() const noexcept -> domain::Rank
	{
		// 只有超量卡有阶级

		// todo: 如何处理?
		if (!has_card_type({domain::CardType::XYZ}))
		{
			return domain::Rank::R1;
		}

		return prototype_data().rank;
	}

	auto Card::PrototypeHandler::link_marker() const noexcept -> domain::LinkMarkerWrapper
	{
		// 只有连接卡有连接箭头

		// todo: 如何处理?
		if (!has_card_type({domain::CardType::LINK}))
		{
			return {domain::LinkMarker::TOP};
		}

		return prototype_data().link_marker;
	}

	auto Card::PrototypeHandler::attack() const noexcept -> domain::attack_value_type
	{
		return prototype_data().attack;
	}

	auto Card::PrototypeHandler::defense() const noexcept -> domain::defense_value_type
	{
		return prototype_data().defense;
	}

	auto Card::PrototypeHandler::left_pendulum() const noexcept -> domain::PendulumScale
	{
		// 只有灵摆怪兽有灵摆刻度

		// todo: 如何处理?
		if (!has_card_type({domain::CardType::PENDULUM}))
		{
			return domain::PendulumScale::PS0;
		}

		return prototype_data().left_pendulum;
	}

	auto Card::PrototypeHandler::right_pendulum() const noexcept -> domain::PendulumScale
	{
		// 只有灵摆怪兽有灵摆刻度

		// todo: 如何处理?
		if (!has_card_type({domain::CardType::PENDULUM}))
		{
			return domain::PendulumScale::PS0;
		}

		return prototype_data().right_pendulum;
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
		const auto prototype = card_.get().prototype();
		auto level = prototype.level();

		// todo: 应用效果
		level += domain::Level::L1;

		return level;
	}

	auto Card::PropertyHandler::rank() const noexcept -> domain::Rank
	{
		const auto prototype = card_.get().prototype();
		auto rank = prototype.rank();

		// todo: 应用效果
		if (rank != domain::Rank::R1)
		{
			rank = domain::Rank::R1;
		}

		return rank;
	}

	auto Card::PropertyHandler::link_marker() const noexcept -> domain::LinkMarkerWrapper
	{
		const auto prototype = card_.get().prototype();
		auto link_marker = prototype.link_marker();

		// todo: 应用效果
		link_marker |= domain::LinkMarker::TOP;

		return link_marker;
	}

	auto Card::PropertyHandler::attack() const noexcept -> domain::attack_value_type
	{
		const auto prototype = card_.get().prototype();
		auto attack = prototype.attack();

		// todo: 应用效果
		attack += 9990;

		return attack;
	}

	auto Card::PropertyHandler::defense() const noexcept -> domain::defense_value_type
	{
		const auto prototype = card_.get().prototype();
		auto defense = prototype.defense();

		// todo: 应用效果
		defense += 9990;

		return defense;
	}

	auto Card::PropertyHandler::left_pendulum() const noexcept -> domain::PendulumScale
	{
		const auto prototype = card_.get().prototype();
		auto left_pendulum = prototype.left_pendulum();

		// todo: 应用效果
		if (left_pendulum != domain::PendulumScale::PS0)
		{
			left_pendulum = domain::PendulumScale::PS0;
		}

		return left_pendulum;
	}

	auto Card::PropertyHandler::right_pendulum() const noexcept -> domain::PendulumScale
	{
		const auto prototype = card_.get().prototype();
		auto right_pendulum = prototype.right_pendulum();

		// todo: 应用效果
		if (right_pendulum != domain::PendulumScale::PS0)
		{
			right_pendulum = domain::PendulumScale::PS0;
		}

		return right_pendulum;
	}

	auto Card::PropertyHandler::is_card_type(const domain::CardTypeWrapper expected_card_type) const noexcept -> bool
	{
		return card_type().all_of(expected_card_type);
	}

	auto Card::PropertyHandler::has_card_type(const domain::CardTypeWrapper expected_card_type) const noexcept -> bool
	{
		return card_type().any_of(expected_card_type);
	}

	Card::StateHandler::StateHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::StateHandler::state_data() noexcept -> CardState&
	{
		return card_.get().state_;
	}

	auto Card::StateHandler::state_data() const noexcept -> const CardState&
	{
		return card_.get().state_;
	}

	auto Card::StateHandler::set_controller(const domain::Player controller) noexcept -> void
	{
		state_data().controller = controller;
	}

	auto Card::StateHandler::set_zone(const domain::Zone zone) noexcept -> void
	{
		state_data().zone = zone;
	}

	auto Card::StateHandler::set_zone_index(const domain::zone_sequence_type zone_index) noexcept -> void
	{
		state_data().zone_index = zone_index;
	}

	auto Card::StateHandler::set_field_zone(const domain::FieldZoneSequence field_zone) noexcept -> void
	{
		state_data().field_zone = field_zone;
	}

	auto Card::StateHandler::set_form(const domain::FieldZoneForm form) noexcept -> void
	{
		state_data().form = form;
	}

	auto Card::StateHandler::set_reason_player(const domain::Player player) noexcept -> void
	{
		state_data().reason_player = player;
	}

	auto Card::StateHandler::set_reason_card(const CardOptional card) noexcept -> void
	{
		state_data().reason_card = card;
	}

	auto Card::StateHandler::set_reason_effect(const EffectOptional effect) noexcept -> void
	{
		state_data().reason_effect = effect;
	}

	auto Card::StateHandler::set_reason(const domain::ReasonWrapper reason) noexcept -> void
	{
		state_data().reason = reason;
	}

	auto Card::StateHandler::controller() const noexcept -> domain::Player
	{
		return state_data().controller;
	}

	auto Card::StateHandler::zone() const noexcept -> domain::Zone
	{
		return state_data().zone;
	}

	auto Card::StateHandler::zone_index() const noexcept -> domain::zone_sequence_type
	{
		return state_data().zone_index;
	}

	auto Card::StateHandler::field_zone() const noexcept -> domain::FieldZoneSequence
	{
		return state_data().field_zone;
	}

	auto Card::StateHandler::form() const noexcept -> domain::FieldZoneForm
	{
		return state_data().form;
	}

	auto Card::StateHandler::reason_player() const noexcept -> domain::Player
	{
		return state_data().reason_player;
	}

	auto Card::StateHandler::reason_card() const noexcept -> CardOptional
	{
		return state_data().reason_card;
	}

	auto Card::StateHandler::reason_effect() const noexcept -> EffectOptional
	{
		return state_data().reason_effect;
	}

	auto Card::StateHandler::reason() const noexcept -> domain::ReasonWrapper
	{
		return state_data().reason;
	}

	auto Card::StateHandler::is_controller(const domain::Player expected_player) const noexcept -> bool
	{
		return controller() == expected_player;
	}

	auto Card::StateHandler::is_zone(const domain::Zone expected_zone) const noexcept -> bool
	{
		return zone() == expected_zone;
	}

	auto Card::StateHandler::is_field_zone(const domain::FieldZoneSequence expected_field_zone) const noexcept -> bool
	{
		return field_zone() == expected_field_zone;
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

	Card::SummonHandler::SummonHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::SummonHandler::summon_data() noexcept -> Summon&
	{
		return card_.get().summon_;
	}

	auto Card::SummonHandler::summon_data() const noexcept -> const Summon&
	{
		return card_.get().summon_;
	}

	auto Card::SummonHandler::set_kind(const domain::SummonKind kind) noexcept -> void
	{
		summon_data().kind = kind;
	}

	auto Card::SummonHandler::set_from_zone(const domain::Zone from_zone) noexcept -> void
	{
		summon_data().from_zone = from_zone;
	}

	auto Card::SummonHandler::set_player(const domain::Player player) noexcept -> void
	{
		summon_data().player = player;
	}

	auto Card::SummonHandler::set_turn_index(const turn_index_type turn_index) noexcept -> void
	{
		summon_data().turn_index = turn_index;
	}

	auto Card::SummonHandler::set_materials(Group group) noexcept -> void
	{
		summon_data().materials = std::move(group);
	}

	auto Card::SummonHandler::kind() const noexcept -> domain::SummonKind
	{
		return summon_data().kind;
	}

	auto Card::SummonHandler::from_zone() const noexcept -> domain::Zone
	{
		return summon_data().from_zone;
	}

	auto Card::SummonHandler::player() const noexcept -> domain::Player
	{
		return summon_data().player;
	}

	auto Card::SummonHandler::turn_index() const noexcept -> turn_index_type
	{
		return summon_data().turn_index;
	}

	auto Card::SummonHandler::materials() const noexcept -> View
	{
		return View{summon_data().materials};
	}

	auto Card::SummonHandler::is_kind(const domain::SummonKind expected_kind) const noexcept -> bool
	{
		return kind() == expected_kind;
	}

	auto Card::SummonHandler::is_from_zone(const domain::Zone expected_from_zone) const noexcept -> bool
	{
		return from_zone() == expected_from_zone;
	}

	auto Card::SummonHandler::is_player(const domain::Player expected_player) const noexcept -> bool
	{
		return player() == expected_player;
	}

	auto Card::SummonHandler::is_turn_index(const turn_index_type expected_turn_index) const noexcept -> bool
	{
		return turn_index() == expected_turn_index;
	}

	auto Card::SummonHandler::has_material(const Card& card) const noexcept -> bool
	{
		return summon_data().materials.contains(card);
	}

	auto Card::SummonHandler::material_count() const noexcept -> std::size_t
	{
		return summon_data().materials.size();
	}

	Card::BattleHandler::BattleHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::BattleHandler::battle_data() noexcept -> Battle&
	{
		return card_.get().battle_;
	}

	auto Card::BattleHandler::battle_data() const noexcept -> const Battle&
	{
		return card_.get().battle_;
	}

	auto Card::BattleHandler::record_attack_announced() noexcept -> void
	{
		auto& bi = battle_data();

		bi.attack_announced_count += 1;
		// todo: 获取当前回合号
		bi.attack_turn_index = 0;
	}

	auto Card::BattleHandler::record_attack_canceled() noexcept -> void
	{
		auto& bi = battle_data();

		// todo: 获取当前回合号
		bi.attack_canceled_turn_index = 0;
	}

	auto Card::BattleHandler::record_attacked_card(Card& card) noexcept -> void
	{
		auto& b = battle_data();

		b.attacked_cards.insert(card);
		b.battled_cards.insert(card);
		b.attacked_count += 1;
	}

	auto Card::BattleHandler::attacked_cards() const noexcept -> View
	{
		return View{battle_data().attacked_cards};
	}

	auto Card::BattleHandler::attacked_count() const noexcept -> std::size_t
	{
		return battle_data().attacked_count;
	}

	auto Card::BattleHandler::attack_announced_count() const noexcept -> std::size_t
	{
		return battle_data().attack_announced_count;
	}

	auto Card::BattleHandler::attack_turn_index() const noexcept -> turn_index_type
	{
		return battle_data().attack_turn_index;
	}

	auto Card::BattleHandler::attack_canceled_turn_index() const noexcept -> turn_index_type
	{
		return battle_data().attack_canceled_turn_index;
	}

	auto Card::BattleHandler::is_attacked_this_turn() const noexcept -> bool
	{
		// todo: 获取当前回合号
		return attack_turn_index() == 0;
	}

	auto Card::BattleHandler::is_attack_canceled_this_turn() const noexcept -> bool
	{
		// todo: 获取当前回合号
		return attack_canceled_turn_index() == 0;
	}

	Card::TargetHandler::TargetHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::TargetHandler::target_data() noexcept -> Target&
	{
		return card_.get().target_;
	}

	auto Card::TargetHandler::target_data() const noexcept -> const Target&
	{
		return card_.get().target_;
	}

	auto Card::TargetHandler::set_owner_target(Card& card) noexcept -> bool
	{
		return target_data().owner_targets.insert(card);
	}

	auto Card::TargetHandler::cancel_owner_target(const Card& card) noexcept -> bool
	{
		return target_data().owner_targets.erase(card);
	}

	auto Card::TargetHandler::set_target(Card& card) noexcept -> bool
	{
		if (target_data().card_targets.insert(card))
		{
			// 将本卡加入目标卡的"以本卡为对象的卡"列表中
			card.target().set_owner_target(card_);

			// TODO: 发出事件,广播该卡被设为目标

			return true;
		}

		return false;
	}

	auto Card::TargetHandler::cancel_target(Card& card) noexcept -> bool
	{
		if (target_data().card_targets.erase(card))
		{
			// 将本卡移出目标卡的"以本卡为对象的卡"列表中
			card.target().cancel_owner_target(card_);

			return true;
		}

		return false;
	}

	auto Card::TargetHandler::card_targets() const noexcept -> View
	{
		return View{target_data().card_targets};
	}

	auto Card::TargetHandler::owner_targets() const noexcept -> View
	{
		return View{target_data().owner_targets};
	}

	auto Card::TargetHandler::has_target() const noexcept -> bool
	{
		return !target_data().card_targets.empty();
	}

	auto Card::TargetHandler::has_target(const Card& card) const noexcept -> bool
	{
		return target_data().card_targets.contains(card);
	}

	auto Card::TargetHandler::target_count() const noexcept -> std::size_t
	{
		return target_data().card_targets.size();
	}

	Card::XyzHandler::XyzHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::XyzHandler::xyz_data() noexcept -> Xyz&
	{
		return card_.get().xyz_;
	}

	auto Card::XyzHandler::xyz_data() const noexcept -> const Xyz&
	{
		return card_.get().xyz_;
	}

	auto Card::XyzHandler::add_overlay(Card& material) noexcept -> bool
	{
		// 检查是否已叠放
		if (material.xyz().overlay_target() == &card_.get())
		{
			return true;
		}

		// 如果已经是别的卡的超量素材,将其从原宿主移除
		if (const auto target = material.xyz().overlay_target();
			target != nullptr)
		{
			auto target_xyz = target.get()->xyz();
			target_xyz.remove_overlay(material);
		}
		else
		{
			// 如果不是别的卡的超量素材,将其从场上移除(作为超量素材附加到当前卡上)
			// todo: 将其从场上移除
		}

		// 本卡当前已有素材数量作为新素材的序列位置
		const auto zone_index = material_count();

		// 将本卡作为超量素材的目标
		material.xyz().xyz_data().overlay_target = CardOptional{&card_.get()};
		// 将超量素材添加到超量素材列表
		xyz_data().materials.push_back(material);

		// 设置超量素材状态
		auto material_state = material.state();
		// 控制者设置为本卡的控制者
		material_state.set_controller(card_.get().state().controller());
		// 位置设置为叠放区
		material_state.set_zone(domain::Zone::OVERLAY);
		// 序列设置为本卡超量素材列表的位置
		material_state.set_zone_index(static_cast<domain::zone_sequence_type>(zone_index));

		// todo: 素材效果?

		return true;
	}

	auto Card::XyzHandler::remove_overlay(Card& material) noexcept -> bool
	{
		// 检查是否未叠放
		if (material.xyz().overlay_target() != &card_.get())
		{
			return true;
		}

		// 将超量素材移除出超量素材列表
		if (const auto erased = xyz_data().materials.erase(material);
			!erased)
		{
			// 不存在该素材返回false
			return false;
		}
		// 设置超量素材的叠放目标
		material.xyz().xyz_data().overlay_target = CardOptional{nullptr};

		// todo: 设置超量素材状态
		// auto material_state = material.state();

		// 重置其他素材的序列
		for (auto begin = xyz_data().materials.begin(), it = begin; it != xyz_data().materials.end(); ++it)
		{
			const auto index = std::ranges::distance(begin, it);
			it->get().state().set_zone_index(static_cast<domain::zone_sequence_type>(index));
		}

		// todo: 素材效果?

		return true;
	}

	auto Card::XyzHandler::materials() const noexcept -> View
	{
		return View{xyz_data().materials};
	}

	auto Card::XyzHandler::overlay_target() const noexcept -> CardOptional
	{
		return xyz_data().overlay_target;
	}

	auto Card::XyzHandler::has_material() const noexcept -> bool
	{
		return !xyz_data().materials.empty();
	}

	auto Card::XyzHandler::has_material(const Card& card) const noexcept -> bool
	{
		return xyz_data().materials.contains(card);
	}

	auto Card::XyzHandler::material_count() const noexcept -> std::size_t
	{
		return xyz_data().materials.size();
	}

	Card::EquipHandler::EquipHandler(Card& card) noexcept
		: card_{card} {}

	auto Card::EquipHandler::equip_data() noexcept -> Equip&
	{
		return card_.get().equip_;
	}

	auto Card::EquipHandler::equip_data() const noexcept -> const Equip&
	{
		return card_.get().equip_;
	}

	auto Card::EquipHandler::add_equip(Card& equip) noexcept -> bool
	{
		// 检查是否已装备
		if (equip.equip().owner() == &card_.get())
		{
			return true;
		}

		// 如果已经是别的卡的装备,将其从原宿主移除
		if (const auto owner = equip.equip().owner();
			owner != nullptr)
		{
			auto owner_equip = owner.get()->equip();
			owner_equip.remove_equip(equip);
		}
		else
		{
			// TODO: 移动装备卡位置?
		}

		// 将本卡作为装备的所有者
		equip.equip().equip_data().owner = CardOptional{&card_.get()};
		// 将装备添加到装备列表
		equip_data().equips.insert(equip);

		// todo: 装备效果?

		return true;
	}

	auto Card::EquipHandler::remove_equip(Card& equip) noexcept -> bool
	{
		// 检查是否未装备
		if (equip.equip().owner() != &card_.get())
		{
			return true;
		}

		// 将装备移除出装备列表
		if (const auto erased = equip_data().equips.erase(equip);
			!erased)
		{
			// 不存在该装备返回false
			return false;
		}
		// 设置装备的装备目标
		equip.equip().equip_data().owner = CardOptional{nullptr};

		// todo: 装备效果?

		return true;
	}

	auto Card::EquipHandler::equips() const noexcept -> View
	{
		return View{equip_data().equips};
	}

	auto Card::EquipHandler::owner() const noexcept -> CardOptional
	{
		return equip_data().owner;
	}

	auto Card::EquipHandler::has_equip() const noexcept -> bool
	{
		return !equip_data().equips.empty();
	}

	auto Card::EquipHandler::has_equip(const Card& card) const noexcept -> bool
	{
		return equip_data().equips.contains(card);
	}

	auto Card::EquipHandler::equip_count() const noexcept -> std::size_t
	{
		return equip_data().equips.size();
	}

	auto Card::EquipHandler::equippable() const noexcept -> bool
	{
		// 目标必须是:
		// 1.在场上
		// 2.表侧表示
		// 3.不是衍生物
		if (const auto target_state = card_.get().state();
			!target_state.is_field_zone() or !target_state.is_face_up_form())
		{
			return false;
		}
		if (card_.get().prototype_.get().card_type.any_of(domain::CardType::TOKEN_MONSTER))
		{
			return false;
		}

		// todo: 不能成为卡的目标效果

		return true;
	}

	Card::Card(Duel& duel, const card_instance_id_type instance_id, const Prototype& prototype) noexcept
		: duel_{duel},
		  instance_id_{instance_id},
		  prototype_{prototype},
		  state_
		  {
				  .controller = domain::Player::RED,
				  .zone = domain::Zone::DECK,
				  .zone_index = 0,
				  .field_zone = domain::FieldZoneSequence::MONSTER_MAIN_1,
				  .form = domain::FieldZoneForm::FACE_UP_ATTACK,
				  .reason_player = domain::Player::RED,
				  .reason_card = CardOptional{},
				  .reason_effect = EffectOptional{},
				  .reason = {domain::Reason::NORMAL_SUMMON},
		  },
		  summon_
		  {
				  .kind = domain::SummonKind::NORMAL,
				  .from_zone = domain::Zone::DECK,
				  .player = domain::Player::RED,
				  .turn_index = 0,
				  .materials = {},
		  },
		  battle_
		  {
				  .attacked_cards = {},
				  .battled_cards = {},
				  .attacked_count = 0,
				  .attack_announced_count = 0,
				  .attack_turn_index = 0,
				  .attack_canceled_turn_index = 0,
		  },
		  target_
		  {
				  .card_targets = {},
				  .owner_targets = {},
		  },
		  xyz_
		  {
				  .materials = {},
				  .overlay_target = CardOptional{},
		  },
		  equip_
		  {
				  .equips = {},
				  .owner = CardOptional{},
		  }
	{
		//
	}

	auto Card::instance_id() const noexcept -> card_instance_id_type
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

	auto Card::summon() noexcept -> SummonHandler
	{
		return SummonHandler{*this};
	}

	auto Card::battle() noexcept -> BattleHandler
	{
		return BattleHandler{*this};
	}

	auto Card::target() noexcept -> TargetHandler
	{
		return TargetHandler{*this};
	}

	auto Card::xyz() noexcept -> XyzHandler
	{
		return XyzHandler{*this};
	}

	auto Card::equip() noexcept -> EquipHandler
	{
		return EquipHandler{*this};
	}
}

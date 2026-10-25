#include <cg/dsl/vocabulary.hpp>

#include <algorithm>
#include <ranges>
#include <format>

#include <cg/utility/functional.hpp>
#include <cg/dsl/types.hpp>

namespace cg::dsl
{
	auto Vocabulary::generate_id() const noexcept -> PrimitiveId
	{
		const auto id = primitives_.size() + 1;
		return PrimitiveId{static_cast<PrimitiveId::value_type>(id)};
	}

	auto Vocabulary::do_register(
		std::string name,
		const PrimitiveKind kind,
		const utility::EnumMask<Phase> phases,
		std::vector<ParamDef> params,
		const TypeId returns,
		std::string doc
	) noexcept -> void
	{
		const auto id = generate_id();
		primitives_.emplace_back(id, std::move(name), kind, phases, std::move(params), returns, std::move(doc));
	}

	Vocabulary::Vocabulary(const TypeRegistry& types) noexcept
	{
		const auto type_id = [&](const std::string_view name) noexcept -> TypeId
		{
			const auto* type = types.find(name);
			return type ? type->id : TypeId{TypeId::invalid_id};
		};

		constexpr auto all_phases = [] noexcept -> utility::EnumMask<Phase>
		{
			return {Phase::REQUIRE, Phase::COST, Phase::TARGET, Phase::RESOLVE, Phase::STATIC};
		}();

		// ---------------- 谓词/查询 ----------------

		do_register(
			"is_code",
			PrimitiveKind::PREDICATE,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
					{.name = "code", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
			},
			type_id("bool"),
			"card code predicate"
		);
		do_register(
			"is_card_type",
			PrimitiveKind::PREDICATE,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
					{.name = "card_type", .kind = ParamDef::Value{.type = type_id("card_type")}, .required = true},
			},
			type_id("bool"),
			"卡是否为指定种类(monster/spell/trap)"
		);
		do_register(
			"is_card_subtype",
			PrimitiveKind::PREDICATE,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
					{.name = "subtype", .kind = ParamDef::Value{.type = type_id("card_subtype")}, .required = true},
			},
			type_id("bool"),
			"卡是否具有指定副种类(normal/effect/tuner/…)"
		);
		do_register(
			"is_archetype",
			PrimitiveKind::PREDICATE,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
					{.name = "archetype", .kind = ParamDef::Value{.type = type_id("archetype")}, .required = true},
			},
			type_id("bool"),
			"卡是否属于指定系列/字段"
		);
		do_register(
			"is_attribute",
			PrimitiveKind::PREDICATE,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
					{.name = "attribute", .kind = ParamDef::Value{.type = type_id("attribute")}, .required = true},
			},
			type_id("bool"),
			"卡是否具有指定属性"
		);
		do_register(
			"is_race",
			PrimitiveKind::PREDICATE,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
					{.name = "race", .kind = ParamDef::Value{.type = type_id("race")}, .required = true},
			},
			type_id("bool"),
			"卡是否具有指定种族"
		);
		do_register(
			"is_level",
			PrimitiveKind::PREDICATE,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
					{.name = "level", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
			},
			type_id("bool"),
			"是否为指定等级/阶级的怪兽"
		);
		do_register(
			"can_change_to_face_down",
			PrimitiveKind::PREDICATE,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
			},
			type_id("bool"),
			"是否可以变为里侧守备表示"
		);
		do_register(
			"is_in_zone",
			PrimitiveKind::PREDICATE,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
					{.name = "zones", .kind = ParamDef::Value{.type = type_id("zones")}, .required = true},
			},
			type_id("bool"),
			"是否位于指定区域"
		);
		do_register(
			"is_face_up",
			PrimitiveKind::PREDICATE,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
			},
			type_id("bool"),
			"卡是否表侧表示"
		);

		// ---------------- 区域查询(非取对象) ----------------

		do_register(
			"find_cards",
			PrimitiveKind::QUERY,
			all_phases,
			{
					{.name = "players", .kind = ParamDef::Value{.type = type_id("players")}, .required = true},
					{.name = "zones", .kind = ParamDef::Value{.type = type_id("zones")}, .required = true},
					{.name = "filter", .kind = ParamDef::Scoped{.element = type_id("card"), .type = type_id("bool")}, .required = false},
			},
			type_id("group"),
			"指定玩家范围/区域中满足过滤的全部卡(非取对象)"
		);
		do_register(
			"exists_matching",
			PrimitiveKind::QUERY,
			all_phases,
			{
					{.name = "players", .kind = ParamDef::Value{.type = type_id("players")}, .required = true},
					{.name = "zones", .kind = ParamDef::Value{.type = type_id("zones")}, .required = true},
					{.name = "min", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
					{.name = "filter", .kind = ParamDef::Scoped{.element = type_id("card"), .type = type_id("bool")}, .required = false},
			},
			type_id("bool"),
			"指定玩家范围/区域中是否至少有min张满足过滤的卡"
		);
		do_register(
			"current_attack",
			PrimitiveKind::QUERY,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
					{.name = "filter", .kind = ParamDef::Scoped{.element = type_id("card"), .type = type_id("bool")}, .required = false},
			},
			type_id("int"),
			"卡的当前攻击力(含修正)"
		);
		do_register(
			"current_defense",
			PrimitiveKind::QUERY,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
					{.name = "filter", .kind = ParamDef::Scoped{.element = type_id("card"), .type = type_id("bool")}, .required = false},
			},
			type_id("int"),
			"卡的当前守备力(含修正)"
		);
		do_register(
			"attack",
			PrimitiveKind::QUERY,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
			},
			type_id("int"),
			"卡的印刷攻击力(当前值见current_attack)"
		);
		do_register(
			"defense",
			PrimitiveKind::QUERY,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
			},
			type_id("int"),
			"卡的印刷守备力(当前值见current_defense)"
		);
		do_register(
			"level",
			PrimitiveKind::QUERY,
			all_phases,
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
			},
			type_id("int"),
			"卡的印刷等级/阶级"
		);
		do_register(
			"sole_card",
			PrimitiveKind::QUERY,
			all_phases,
			{
					{.name = "group", .kind = ParamDef::Value{.type = type_id("group")}, .required = true},
			},
			type_id("card"),
			"取组内唯一的卡(数量不为 1 时处理失败)"
		);
		do_register(
			"this_card",
			PrimitiveKind::QUERY,
			all_phases,
			{},
			type_id("card"),
			"效果来源卡(不依赖绑定)"
		);
		do_register(
			"$",
			PrimitiveKind::QUERY,
			all_phases,
			{},
			type_id("card"),
			"当前 Scoped 参数的元素(filter/apply_to/召唤素材等)"
		);
		do_register(
			"chain_exists",
			PrimitiveKind::QUERY,
			all_phases,
			{},
			type_id("bool"),
			"当前是否已有连锁(含正在响应的上一环)"
		);
		do_register(
			"event_card",
			PrimitiveKind::QUERY,
			all_phases,
			{},
			type_id("card"),
			"当前事件关联卡"
		);
		do_register(
			"event_amount",
			PrimitiveKind::QUERY,
			all_phases,
			{},
			type_id("int"),
			"当前事件数值(如 BATTLE_DAMAGE 的伤害)"
		);
		do_register(
			"event_player",
			PrimitiveKind::QUERY,
			all_phases,
			{},
			type_id("int"),
			"当前事件玩家(0=效果控制者,1=对方)"
		);
		do_register(
			"equipped_card",
			PrimitiveKind::QUERY,
			all_phases,
			{},
			type_id("card"),
			"被这张卡装备的怪兽(装备卡上下文引用)"
		);

		// ---------------- 选择 ----------------

		do_register(
			"pick_cost",
			PrimitiveKind::SELECT,
			{Phase::COST},
			{
					{.name = "players", .kind = ParamDef::Value{.type = type_id("players")}, .required = true},
					{.name = "zones", .kind = ParamDef::Value{.type = type_id("zones")}, .required = true},
					{.name = "min", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
					{.name = "max", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
					{.name = "filter", .kind = ParamDef::Scoped{.element = type_id("card"), .type = type_id("bool")}, .required = false},
					{.name = "exclude_source", .kind = ParamDef::Value{.type = type_id("bool")}, .required = false},
			},
			type_id("group"),
			"在指定玩家范围/区域中选择代价(仅COST阶段)"
		);
		do_register(
			"pick_target",
			PrimitiveKind::SELECT,
			{Phase::TARGET},
			{
					{.name = "players", .kind = ParamDef::Value{.type = type_id("players")}, .required = true},
					{.name = "zones", .kind = ParamDef::Value{.type = type_id("zones")}, .required = true},
					{.name = "min", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
					{.name = "max", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
					{.name = "filter", .kind = ParamDef::Scoped{.element = type_id("card"), .type = type_id("bool")}, .required = false},
			},
			type_id("group"),
			"在指定玩家范围/区域中取对象(仅TARGET阶段)"
		);
		do_register(
			"pick_resolution",
			PrimitiveKind::SELECT,
			{Phase::RESOLVE},
			{
					{.name = "players", .kind = ParamDef::Value{.type = type_id("players")}, .required = true},
					{.name = "zones", .kind = ParamDef::Value{.type = type_id("zones")}, .required = true},
					{.name = "min", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
					{.name = "max", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
					{.name = "filter", .kind = ParamDef::Scoped{.element = type_id("card"), .type = type_id("bool")}, .required = false},
			},
			type_id("group"),
			"结算时在指定玩家范围/区域中选择(不取对象)"
		);

		// ---------------- 动作 ----------------

		do_register(
			"destroy",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{
					{.name = "cards", .kind = ParamDef::Value{.type = type_id("group")}, .required = true},
					{.name = "reason", .kind = ParamDef::Value{.type = type_id("reason")}, .required = true},
			},
			type_id("void"),
			"破坏组内全部卡"
		);
		do_register(
			"negate_activation",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{},
			type_id("void"),
			"使直接响应的上一环(Chain Link N-1)的发动无效"
		);
		do_register(
			"negate_activation_and_destroy",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{},
			type_id("void"),
			"使直接响应的上一环(Chain Link N-1)的发动无效并破坏该卡"
		);
		do_register(
			"negate_effect",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{},
			type_id("void"),
			"使直接响应的上一环(Chain Link N-1)的效果无效"
		);
		do_register(
			"special_summon",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
					{.name = "method", .kind = ParamDef::Value{.type = type_id("summon_method")}, .required = true},
					{.name = "position", .kind = ParamDef::Value{.type = type_id("position")}, .required = true},
			},
			type_id("void"),
			"特殊召唤"
		);
		do_register(
			"ritual_summon",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{
					{.name = "position", .kind = ParamDef::Value{.type = type_id("position")}, .required = true},
					{.name = "filter", .kind = ParamDef::Scoped{.element = type_id("card"), .type = type_id("bool")}, .required = false},
			},
			type_id("void"),
			"效果驱动的仪式召唤:从手牌选择仪式怪兽与素材,素材送墓后特殊召唤;不开召唤宣言时点"
		);
		do_register(
			"discard",
			PrimitiveKind::ACTION,
			{Phase::COST, Phase::RESOLVE},
			{
					{.name = "cards", .kind = ParamDef::Value{.type = type_id("group")}, .required = true},
			},
			type_id("void"),
			"舍弃(手牌送墓;与 send_to_graveyard 语义不同)"
		);
		do_register(
			"release",
			PrimitiveKind::ACTION,
			{Phase::COST, Phase::RESOLVE},
			{
					{.name = "cards", .kind = ParamDef::Value{.type = type_id("group")}, .required = true},
			},
			type_id("void"),
			"解放(不视为破坏)"
		);
		do_register(
			"pay_life",
			PrimitiveKind::ACTION,
			{Phase::COST},
			{
					{.name = "amount", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
			},
			type_id("void"),
			"支付生命值"
		);
		do_register(
			"pay_half_life",
			PrimitiveKind::ACTION,
			{Phase::COST},
			{},
			type_id("void"),
			"支付一半生命值(向上取整)"
		);
		do_register(
			"send_to_graveyard",
			PrimitiveKind::ACTION,
			{Phase::COST, Phase::RESOLVE},
			{
					{.name = "cards", .kind = ParamDef::Value{.type = type_id("group")}, .required = true},
			},
			type_id("void"),
			"送入墓地(不触发舍弃;与 discard 语义不同)"
		);
		do_register(
			"return_to_deck",
			PrimitiveKind::ACTION,
			{Phase::COST, Phase::RESOLVE},
			{
					{.name = "cards", .kind = ParamDef::Value{.type = type_id("group")}, .required = true},
					{.name = "shuffle", .kind = ParamDef::Value{.type = type_id("bool")}, .required = false},
			},
			type_id("void"),
			"返回卡组(默认洗牌)"
		);
		do_register(
			"draw",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{
					{.name = "players", .kind = ParamDef::Value{.type = type_id("players")}, .required = true},
					{.name = "count", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
			},
			type_id("void"),
			"各指定玩家抽 count 张卡"
		);
		do_register(
			"add_to_hand",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{
					{.name = "cards", .kind = ParamDef::Value{.type = type_id("group")}, .required = true},
			},
			type_id("void"),
			"加入手牌(如从卡组检索、从墓地回收)"
		);
		do_register(
			"banish",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{
					{.name = "cards", .kind = ParamDef::Value{.type = type_id("group")}, .required = true},
					{.name = "face_down", .kind = ParamDef::Value{.type = type_id("bool")}, .required = true},
			},
			type_id("void"),
			"除外(face_down = 里侧除外)"
		);
		do_register(
			"change_position",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
					{.name = "position", .kind = ParamDef::Value{.type = type_id("position")}, .required = true},
			},
			type_id("void"),
			"变更表示形式"
		);
		do_register(
			"effect_damage",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{
					{.name = "players", .kind = ParamDef::Value{.type = type_id("players")}, .required = true},
					{.name = "amount", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
			},
			type_id("void"),
			"对指定玩家造成效果伤害"
		);
		do_register(
			"recover_life",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{
					{.name = "players", .kind = ParamDef::Value{.type = type_id("players")}, .required = true},
					{.name = "amount", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
			},
			type_id("void"),
			"指定玩家回复生命值"
		);

		// ---------------- 静态修饰符 ----------------

		do_register(
			"change_name",
			PrimitiveKind::STATIC,
			{Phase::STATIC},
			{
					{.name = "card", .kind = ParamDef::Value{.type = type_id("card")}, .required = true},
			},
			type_id("void"),
			"视为指定卡的卡名"
		);
		do_register(
			"add_card_type",
			PrimitiveKind::STATIC,
			{Phase::STATIC},
			{
					{.name = "card_type", .kind = ParamDef::Value{.type = type_id("card_type")}, .required = true},
			},
			type_id("void"),
			"增加卡牌类型(如:也视为怪兽)"
		);
		do_register(
			"add_card_subtype",
			PrimitiveKind::STATIC,
			{Phase::STATIC},
			{
					{.name = "subtype", .kind = ParamDef::Value{.type = type_id("card_subtype")}, .required = true},
			},
			type_id("void"),
			"增加卡牌子类型(如:也视为调整)"
		);
		do_register(
			"add_attribute",
			PrimitiveKind::STATIC,
			{Phase::STATIC},
			{
					{.name = "attribute", .kind = ParamDef::Value{.type = type_id("attribute")}, .required = true},
			},
			type_id("void"),
			"增加属性(如:也视为暗属性)"
		);
		do_register(
			"change_level",
			PrimitiveKind::STATIC,
			{Phase::STATIC},
			{
					{.name = "delta", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
			},
			type_id("void"),
			"改变等级/阶级(增量)"
		);
		do_register(
			"add_numeric_property",
			PrimitiveKind::STATIC,
			{Phase::STATIC},
			{
					{.name = "property", .kind = ParamDef::Value{.type = type_id("numeric_property")}, .required = true},
					{.name = "value", .kind = ParamDef::Value{.type = type_id("int")}, .required = true},
			},
			type_id("void"),
			"按数值项增加(如:攻击力上升500)"
		);
		do_register(
			"add_protection",
			PrimitiveKind::STATIC,
			{Phase::STATIC},
			{
					{.name = "protection", .kind = ParamDef::Value{.type = type_id("protection")}, .required = true},
			},
			type_id("void"),
			"给予抗性(如:不会被战斗破坏)"
		);
	}

	auto Vocabulary::validate(const TypeRegistry& types) const noexcept -> std::vector<std::string>
	{
		std::vector<std::string> errors{};

		const auto validate = [&](const std::tuple<std::ptrdiff_t, const PrimitiveDef&>& tuple) noexcept -> void
		{
			const auto [primitive_index, primitive] = tuple;

			if (!primitive.id.valid())
			{
				errors.push_back(std::format("[{}] id invalid", primitive_index));
			}
			if (primitive.name.empty())
			{
				errors.push_back(std::format("[{}] name empty", primitive_index));
			}
			if (!types.contains(primitive.returns))
			{
				errors.push_back(std::format("[{}] unregistered return type", primitive_index));
			}
			if (primitive.phases.empty())
			{
				errors.push_back(std::format("[{}] primitive `{}` has no phase", primitive_index, primitive.name));
			}
			else if (primitive.kind == PrimitiveKind::STATIC && !primitive.phases.contains(Phase::STATIC))
			{
				errors.push_back(std::format("[{}] static primitive `{}` has no STATIC phase", primitive_index, primitive.name));
			}

			for (const auto [param_index, param]: primitive.params | std::views::enumerate)
			{
				if (param.name.empty())
				{
					errors.push_back(std::format("[{}] empty param name [{}]", primitive_index, param_index));
				}

				std::visit(
					utility::Overloaded
					{
							[&](const ParamDef::Value& value) noexcept -> void
							{
								if (!types.contains(value.type))
								{
									errors.push_back(std::format("[{}] unregistered param type of {}[{}]", primitive_index, param.name, param_index));
								}
							},
							[&](const ParamDef::Scoped& scoped) noexcept -> void
							{
								if (!types.contains(scoped.type))
								{
									errors.push_back(std::format("[{}] unregistered param type of {}[{}]", primitive_index, param.name, param_index));
								}

								if (!types.contains(scoped.element))
								{
									errors.push_back(std::format("[{}] unregistered element type of scoped param `{}`", primitive_index, param.name));
								}
							},
					},
					param.kind
				);

				if (std::ranges::contains(primitive.params.begin(), primitive.params.begin() + param_index, param.name, &ParamDef::name))
				{
					errors.push_back(std::format("[{}] duplicate param name `{}`", primitive_index, param.name));
				}

				// 位置槽无空洞:可选参数必须全部位于末尾
				if (!param.required)
				{
					if (const auto it = std::ranges::find(primitive.params.begin() + param_index + 1, primitive.params.end(), true, &ParamDef::required);
						it != primitive.params.end())
					{
						errors.push_back(std::format("[{}] required param `{}` follows optional `{}`", param_index, it->name, param.name));
					}
				}
			}
		};

		std::ranges::for_each(std::views::enumerate(primitives_), validate);

		const auto check_dup = [&]<typename Result>(const std::string_view name, Result PrimitiveDef::* element) noexcept -> void
		{
			std::vector<Result> results{};
			results.reserve(primitives_.size());

			std::ranges::transform(primitives_, std::back_inserter(results), element);
			std::ranges::sort(results);

			for (auto groups = results | std::views::chunk_by(std::ranges::equal_to{});
			     const auto& group: groups)
			{
				if (std::ranges::distance(group) > 1)
				{
					if constexpr (std::is_same_v<Result, PrimitiveId>)
					{
						errors.push_back(std::format("duplicate primitive {}: {}", name, std::ranges::begin(group)->raw()));
					}
					else
					{
						errors.push_back(std::format("duplicate primitive {}: {}", name, *std::ranges::begin(group)));
					}
				}
			}
		};

		check_dup("id", &PrimitiveDef::id);
		check_dup("name", &PrimitiveDef::name);

		return errors;
	}

	auto Vocabulary::find(const PrimitiveId id) const noexcept -> const PrimitiveDef*
	{
		const auto it = std::ranges::find(primitives_, id, &PrimitiveDef::id);
		if (it == primitives_.end())
		{
			return nullptr;
		}

		return &*it;
	}

	auto Vocabulary::find(const std::string_view name) const noexcept -> const PrimitiveDef*
	{
		const auto it = std::ranges::find(primitives_, name, &PrimitiveDef::name);
		if (it == primitives_.end())
		{
			return nullptr;
		}

		return &*it;
	}

	auto Vocabulary::empty() const noexcept -> bool
	{
		return primitives_.empty();
	}

	auto Vocabulary::size() const noexcept -> size_type
	{
		return primitives_.size();
	}
}

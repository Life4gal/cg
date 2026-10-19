#include <cg/dsl/vocabulary.hpp>

#include <algorithm>
#include <ranges>
#include <format>

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
			"is_card_type",
			PrimitiveKind::PREDICATE,
			all_phases,
			{
					{.name = "card", .type = type_id("card"), .is_required = true},
					{.name = "card_type", .type = type_id("card_type"), .is_required = true},
			},
			type_id("bool"),
			"卡是否为指定种类(monster/spell/trap)"
		);
		do_register(
			"is_card_subtype",
			PrimitiveKind::PREDICATE,
			all_phases,
			{
					{.name = "card", .type = type_id("card"), .is_required = true},
					{.name = "subtype", .type = type_id("card_subtype"), .is_required = true},
			},
			type_id("bool"),
			"卡是否具有指定副种类(normal/effect/tuner/…)"
		);
		do_register(
			"is_archetype",
			PrimitiveKind::PREDICATE,
			all_phases,
			{
					{.name = "card", .type = type_id("card"), .is_required = true},
					{.name = "archetype", .type = type_id("archetype"), .is_required = true},
			},
			type_id("bool"),
			"卡是否属于指定系列/字段"
		);

		// ---------------- 区域查询(非取对象) ----------------

		do_register(
			"find_cards",
			PrimitiveKind::QUERY,
			all_phases,
			{
					{.name = "player", .type = type_id("player"), .is_required = true},
					{.name = "zones", .type = type_id("zone_set"), .is_required = true},
					{.name = "filter", .type = type_id("bool"), .is_required = false},
			},
			type_id("bool"),
			"指定玩家区域中满足过滤的全部卡(非取对象)"
		);
		do_register(
			"exists_matching",
			PrimitiveKind::QUERY,
			all_phases,
			{
					{.name = "player", .type = type_id("player"), .is_required = true},
					{.name = "zones", .type = type_id("zone_set"), .is_required = true},
					{.name = "min", .type = type_id("int"), .is_required = true},
					{.name = "filter", .type = type_id("bool"), .is_required = false},
			},
			type_id("bool"),
			"指定玩家区域中是否至少有min张满足过滤的卡"
		);
		do_register(
			"current_attack",
			PrimitiveKind::QUERY,
			all_phases,
			{
					{.name = "card", .type = type_id("card"), .is_required = true},
					{.name = "filter", .type = type_id("bool"), .is_required = false},
			},
			type_id("int"),
			"卡的当前攻击力(含修正)"
		);
		do_register(
			"current_defense",
			PrimitiveKind::QUERY,
			all_phases,
			{
					{.name = "card", .type = type_id("card"), .is_required = true},
					{.name = "filter", .type = type_id("bool"), .is_required = false},
			},
			type_id("int"),
			"卡的当前守备力(含修正)"
		);

		// ---------------- 选择 ----------------

		do_register(
			"pick_target",
			PrimitiveKind::SELECT,
			{Phase::TARGET},
			{
					{.name = "player", .type = type_id("player"), .is_required = true},
					{.name = "zone", .type = type_id("zone"), .is_required = true},
					{.name = "min", .type = type_id("min"), .is_required = true},
					{.name = "max", .type = type_id("max"), .is_required = true},
			},
			type_id("group"),
			"在指定玩家/区域中取对象(仅TARGET阶段)"
		);

		// ---------------- 动作 ----------------

		do_register(
			"destroy",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{
					{.name = "cards", .type = type_id("group"), .is_required = true},
					{.name = "reason", .type = type_id("reason"), .is_required = true},
			},
			type_id("void"),
			"破坏组内全部卡"
		);
		do_register(
			"special_summon",
			PrimitiveKind::ACTION,
			{Phase::RESOLVE},
			{
					{.name = "card", .type = type_id("card"), .is_required = true},
					{.name = "method", .type = type_id("summon_method"), .is_required = true},
					{.name = "position", .type = type_id("position"), .is_required = true},
			},
			type_id("void"),
			"特殊召唤"
		);

		// ---------------- 静态修饰符 ----------------

		do_register(
			"add_numeric_property",
			PrimitiveKind::STATIC,
			{Phase::STATIC},
			{
					{.name = "property", .type = type_id("numeric_property"), .is_required = true},
					{.name = "value", .type = type_id("int"), .is_required = true},
			},
			type_id("void"),
			"按数值项增加(如:攻击力上升500)"
		);
	}

	auto Vocabulary::validate(const TypeRegistry& types) const noexcept -> std::vector<std::string>
	{
		std::vector<std::string> errors{};

		const auto validate = [&](const std::tuple<std::ptrdiff_t, const PrimitiveDef&>& tuple) noexcept -> void
		{
			const auto [index, primitive] = tuple;

			if (!primitive.id.valid())
			{
				errors.push_back(std::format("[{}] id invalid", index));
			}
			if (primitive.name.empty())
			{
				errors.push_back(std::format("[{}] name empty", index));
			}
			if (!types.contains(primitive.returns))
			{
				errors.push_back(std::format("[{}] unregistered return type", index));
			}

			for (std::size_t i = 0; i < primitive.params.size(); ++i)
			{
				if (const auto& param = primitive.params[i];
					!types.contains(param.type))
				{
					errors.push_back(std::format("[{}] unregistered param type of {}[{}]", index, param.name, i));
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

			for (auto groups = results | std::views::chunk_by(std::ranges::equal_to{}); const auto& group: groups)
			{
				if (std::ranges::distance(group) > 1)
				{
					errors.push_back(std::format("duplicate primitive {}: {}", name, *std::ranges::begin(group)));
				}
			}
		};

		check_dup("id", &PrimitiveDef::id);
		check_dup("name", &PrimitiveDef::name);

		return errors;
	}

	auto Vocabulary::find(const PrimitiveId id) noexcept -> const PrimitiveDef*
	{
		const auto it = std::ranges::find(primitives_, id, &PrimitiveDef::id);
		if (it == primitives_.end())
		{
			return nullptr;
		}

		return &*it;
	}

	auto Vocabulary::find(const std::string_view name) noexcept -> const PrimitiveDef*
	{
		const auto it = std::ranges::find(primitives_, name, &PrimitiveDef::name);
		if (it == primitives_.end())
		{
			return nullptr;
		}

		return &*it;
	}
}

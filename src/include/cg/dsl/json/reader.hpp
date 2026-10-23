#pragma once

#include <cg/utility/hash.hpp>

#include <cg/dsl/vocabulary.hpp>
#include <cg/dsl/card_script.hpp>
#include <cg/dsl/script.hpp>

#include <nlohmann/json.hpp>

namespace cg::dsl
{
	class TypeRegistry;
	class PrimitiveDef;
	class Vocabulary;
}

namespace cg::dsl::json
{
	class Reader
	{
	public:
		//

	private:
		using scope_type = std::unordered_map<std::string, BindingId, utility::hash<std::string>, std::equal_to<>>;

		std::reference_wrapper<const TypeRegistry> types_;
		std::reference_wrapper<const Vocabulary> vocabulary_;
		Diagnostics diagnostics_;

		std::vector<scope_type> scopes_;
		BindingId::value_type binding_id_;

		auto fail(std::string path, std::string message) noexcept -> void;
		auto fail(std::string_view path, std::string message) noexcept -> void;

		[[nodiscard]] auto contains_binding(std::string_view name) const noexcept -> bool;
		[[nodiscard]] auto find_binding(std::string_view name) const noexcept -> std::optional<BindingId>;
		[[nodiscard]] auto generate_binding_id() noexcept -> BindingId;

		// =====================================================================================
		// dsl::Statement
		// =====================================================================================

		[[nodiscard]] auto read_statement_list(const nlohmann::json& json, Phase phase, std::string_view path) noexcept -> std::optional<StatementList>;
		[[nodiscard]] auto read_statement(const nlohmann::json& json, Phase phase, std::string_view path) noexcept -> std::optional<Statement>;

		[[nodiscard]] auto read_statement_conjunction(const nlohmann::json::const_iterator& conjunction, std::string_view path) noexcept -> std::optional<Conjunction>;
		[[nodiscard]] auto read_call_statement_body(const nlohmann::json& json, Phase phase, std::string_view path) noexcept -> std::optional<ActionCall>;
		[[nodiscard]] auto read_guard_statement_body(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<Guard>;
		[[nodiscard]] auto read_if_statement_body(const nlohmann::json& json, Phase phase, std::string_view path) noexcept -> std::optional<If>;
		[[nodiscard]] auto read_foreach_statement_body(const nlohmann::json& json, Phase phase, std::string_view path) noexcept -> std::optional<ForEach>;

		// =====================================================================================
		// dsl::Expression
		// =====================================================================================

		[[nodiscard]] auto read_expression(const nlohmann::json& json, TypeId expected, std::string_view path) noexcept -> std::optional<Expression>;

		[[nodiscard]] auto read_literal_number_expression(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<Expression>;
		[[nodiscard]] auto read_literal_boolean_expression(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<Expression>;
		[[nodiscard]] auto read_literal_symbol_expression(const nlohmann::json& json, TypeId expected, std::string_view path) noexcept -> std::optional<Expression>;
		[[nodiscard]] auto read_enum_set_constant_expression(const nlohmann::json& json, TypeId expected, std::string_view path) noexcept -> std::optional<Expression>;
		[[nodiscard]] auto read_binding_ref_expression(const nlohmann::json::const_iterator& binding, std::string_view path) noexcept -> std::optional<Expression>;
		[[nodiscard]] auto read_call_expression(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<Expression>;
		[[nodiscard]] auto read_boolean_operation_expression(const nlohmann::json::const_iterator& operation, BooleanOperator op, std::string_view path) noexcept -> std::optional<Expression>;
		[[nodiscard]] auto read_compare_operation_expression(const nlohmann::json::const_iterator& operation, CompareOperator op, std::string_view path) noexcept -> std::optional<Expression>;
		[[nodiscard]] auto read_arithmetic_operation_expression(const nlohmann::json::const_iterator& operation, ArithmeticOperator op, std::string_view path) noexcept -> std::optional<Expression>;

		[[nodiscard]] auto read_argument_list(const PrimitiveDef& primitive, const nlohmann::json::const_iterator& args, std::string_view path) noexcept -> std::optional<ExpressionList>;

		// =====================================================================================
		// dsl::CardScript::identity
		// =====================================================================================

		[[nodiscard]] auto read_identity(const nlohmann::json::const_iterator& identity, std::string_view path) noexcept -> std::optional<CardIdentity>;

		[[nodiscard]] auto read_identity_kind(const nlohmann::json::const_iterator& kind, std::string_view path) noexcept -> std::optional<domain::CardType>;
		[[nodiscard]] auto read_identity_subtypes(const nlohmann::json::const_iterator& subtypes, std::string_view path) noexcept -> std::optional<utility::EnumMask<domain::CardSubtype>>;
		[[nodiscard]] auto read_identity_monster(const nlohmann::json::const_iterator& monster, std::string_view path) noexcept -> std::optional<MonsterIdentity>;
		[[nodiscard]] auto read_identity_pendulum(const nlohmann::json::const_iterator& pendulum, std::string_view path) noexcept -> std::optional<PendulumIdentity>;
		[[nodiscard]] auto read_identity_link_marker(const nlohmann::json::const_iterator& link_marker, std::string_view path) noexcept -> std::optional<LinkMarkerIdentity>;
		[[nodiscard]] auto read_identity_archetypes(const nlohmann::json::const_iterator& archetypes, std::string_view path) noexcept -> std::optional<std::vector<ArchetypeId>>;
		[[nodiscard]] auto read_identity_code_list(const nlohmann::json::const_iterator& code_list, std::string_view path) noexcept -> std::optional<std::vector<domain::CardCode>>;

		// =====================================================================================
		// dsl::CardScript::summons
		// =====================================================================================

		[[nodiscard]] auto read_summon_list(const nlohmann::json::const_iterator& summons, std::string_view path) noexcept -> std::optional<SummonProcedureList>;
		[[nodiscard]] auto read_summon(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<SummonProcedure>;

		[[nodiscard]] auto read_summon_fusion_procedure(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<FusionProcedure>;
		[[nodiscard]] auto read_summon_fusion_procedure_material(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<MaterialSpec>;
		[[nodiscard]] auto read_summon_synchro_procedure(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<SynchroProcedure>;
		[[nodiscard]] auto read_summon_xyz_procedure(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<XyzProcedure>;
		[[nodiscard]] auto read_summon_link_procedure(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<LinkProcedure>;
		[[nodiscard]] auto read_summon_ritual_procedure(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<RitualProcedure>;
		[[nodiscard]] auto read_summon_self_special_procedure(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<SelfSpecialProcedure>;

		// 素材谓词:在 scoped card 作用域内读取 bool 表达式;候选用 candidate_card()
		[[nodiscard]] auto read_summon_material_expression(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<Expression>;

		// =====================================================================================
		// dsl::CardScript::effects
		// =====================================================================================

		[[nodiscard]] auto read_effect_list(const nlohmann::json::const_iterator& effects, std::string_view path) noexcept -> std::optional<EffectList>;
		[[nodiscard]] auto read_effect_body(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<EffectBody>;


		[[nodiscard]] auto read_activated_effect_body(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<ActivatedEffect>;
		[[nodiscard]] auto read_activated_effect_activation(const nlohmann::json::const_iterator& activation, std::string_view path) noexcept -> std::optional<ActivationKind>;
		[[nodiscard]] auto read_activated_effect_activation_trigger_source_list(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<TriggerSourceList>;
		[[nodiscard]] auto read_activated_effect_activation_trigger_source(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<TriggerSource>;
		[[nodiscard]] auto read_activated_effect_activation_trigger_spec(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<TriggerSpec>;

		[[nodiscard]] auto read_static_effect_body(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<StaticEffect>;
		[[nodiscard]] auto read_static_effect_modifier(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<StaticModifier>;

		[[nodiscard]] auto read_effect_usage_restriction(const nlohmann::json::const_iterator& usage, std::string_view path) noexcept -> std::optional<UsageRestriction>;
		[[nodiscard]] auto read_effect_properties(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<utility::EnumMask<EffectProperty>>;

	public:
		Reader(const TypeRegistry& types, const Vocabulary& vocabulary) noexcept;

		// 读取产生的诊断(不会清除)(按产生顺序;空 = 无错误)
		[[nodiscard]] auto diagnostics() const noexcept -> const Diagnostics&;

		// 获取产生的诊断
		[[nodiscard]] auto take_diagnostics() noexcept -> Diagnostics;

		[[nodiscard]] auto read_card_script(const nlohmann::json& json, std::string_view path) noexcept -> std::optional<CardScript>;
	};
}

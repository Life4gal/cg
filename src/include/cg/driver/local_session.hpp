#pragma once

#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <cg/domain/card.hpp>
#include <cg/domain/field.hpp>
#include <cg/domain/player.hpp>

#include <cg/execution/decision.hpp>

#include <cg/flow/duel.hpp>

#include <cg/state/id.hpp>

namespace cg::driver
{
	// ================================================================
	// 本地动作
	// ================================================================

	class PassAction {};

	class AdvancePhaseAction {};

	class EndTurnAction {};

	class SurrenderAction {};

	class NormalSummonAction
	{
	public:
		state::CardInstanceId card;
		domain::FieldColumn column;
	};

	class SetMonsterAction
	{
	public:
		state::CardInstanceId card;
		domain::FieldColumn column;
	};

	class TributeSummonAction
	{
	public:
		state::CardInstanceId card;
		domain::FieldColumn column;
		std::vector<state::CardInstanceId> tributes;
	};

	class ExtraSummonAction
	{
	public:
		domain::ExtraDeckSummonType kind;
		state::CardInstanceId card;
		domain::AnyMonsterPlacement placement;
		std::vector<state::CardInstanceId> materials;
	};

	class ActivateCardAction
	{
	public:
		state::CardInstanceId card;
	};

	class SetSpellTrapAction
	{
	public:
		state::CardInstanceId card;
		domain::FieldColumn column;
	};

	class DeclareAttackAction
	{
	public:
		state::CardInstanceId attacker;
		std::optional<state::CardInstanceId> target;
	};

	class ReplayAttackAction
	{
	public:
		std::optional<state::CardInstanceId> target;
	};

	class ActivateEffectAction
	{
	public:
		state::CardInstanceId card;
		std::size_t effect_index;
	};

	class LocalAction : public std::variant<
				PassAction,
				AdvancePhaseAction,
				EndTurnAction,
				SurrenderAction,
				NormalSummonAction,
				SetMonsterAction,
				TributeSummonAction,
				ExtraSummonAction,
				ActivateCardAction,
				SetSpellTrapAction,
				DeclareAttackAction,
				ReplayAttackAction,
				ActivateEffectAction
			>
	{
	public:
		using variant::variant;
	};

	// ================================================================
	// 玩家视图
	// ================================================================

	class PlayerView
	{
	public:
		class SelfSide
		{
		public:
			std::size_t deck_count;
			std::vector<state::CardInstanceId> extra_deck;
			std::vector<state::CardInstanceId> hand;
			std::vector<state::CardInstanceId> graveyard;
			std::vector<state::CardInstanceId> banished;
			std::vector<state::CardInstanceId> monsters;
			std::vector<state::CardInstanceId> spell_traps;
			state::CardInstanceId field_spell;
		};

		class OpponentSide
		{
		public:
			std::size_t deck_count;
			std::size_t extra_deck_count;
			std::size_t hand_count;
			std::vector<state::CardInstanceId> graveyard;
			std::vector<state::CardInstanceId> banished;
			std::vector<state::CardInstanceId> monsters;
			std::vector<state::CardInstanceId> spell_traps;
			state::CardInstanceId field_spell;
		};

		// 双方卡牌信息
		SelfSide self_side;
		OpponentSide opponent_side;
		// 实例 ID -> 卡密; 仅供 UI 显示,不暴露额外隐藏信息
		std::unordered_map<std::uint32_t, domain::CardCode> card_codes;
		// 双方LP信息
		std::array<std::underlying_type_t<domain::AttackDefense>, 2> life_points;

		// 视图所属玩家
		domain::Player player;
		// Turn
		domain::Player turn_player;
		flow::Turn::id_type turn_id;
		domain::Phase phase;
		bool first_turn;
		// Priority
		domain::Player priority_player;
		bool priority_both_passed;
		// Duel
		bool pending_attack;
		bool pending_replay;
		bool pending_summon;
		bool chain_pending;
	};

	// ================================================================
	// 控制器
	// ================================================================

	class LocalController
	{
	public:
		LocalController() noexcept = default;

		LocalController(const LocalController&) noexcept = delete;
		auto operator=(const LocalController&) noexcept -> LocalController& = delete;
		LocalController(LocalController&&) noexcept = delete;
		auto operator=(LocalController&&) noexcept -> LocalController& = delete;

		virtual ~LocalController() noexcept = default;

		// 返回空 = 当前控制器没有可执行动作, 对局暂停
		[[nodiscard]] virtual auto choose_action(const PlayerView& view) -> std::optional<LocalAction> = 0;

		[[nodiscard]] virtual auto select_cards(
			domain::Player player,
			const execution::Group& candidates,
			std::size_t min,
			std::size_t max
		) noexcept -> execution::Group = 0;

		[[nodiscard]] virtual auto select_monster_location(
			domain::Player player,
			state::CardInstanceId card,
			std::span<const domain::MonsterLocation> candidates
		) noexcept -> std::optional<domain::MonsterLocation> = 0;

		[[nodiscard]] virtual auto select_spell_trap_location(
			domain::Player player,
			state::CardInstanceId card,
			std::span<const domain::SpellTrapLocation> candidates
		) noexcept -> std::optional<domain::SpellTrapLocation> = 0;

		[[nodiscard]] virtual auto choose_triggers(const execution::TriggerDecision& decision) noexcept -> std::vector<std::size_t> = 0;
	};

	// 测试/脚本驱动用: 按顺序返回动作; 选择类决策默认取首个合法候选/必发全选
	class ScriptedController final : public LocalController
	{
	public:
		using actions_type = std::vector<LocalAction>;

	private:
		actions_type actions_;
		std::size_t next_;

	public:
		explicit ScriptedController(actions_type actions = {}) noexcept;

		auto set_actions(actions_type actions) noexcept -> void;

		[[nodiscard]] auto choose_action(const PlayerView& view) -> std::optional<LocalAction> override;

		[[nodiscard]] auto select_cards(
			domain::Player player,
			const execution::Group& candidates,
			std::size_t min,
			std::size_t max
		) noexcept -> execution::Group override;

		[[nodiscard]] auto select_monster_location(
			domain::Player player,
			state::CardInstanceId card,
			std::span<const domain::MonsterLocation> candidates
		) noexcept -> std::optional<domain::MonsterLocation> override;

		[[nodiscard]] auto select_spell_trap_location(
			domain::Player player,
			state::CardInstanceId card,
			std::span<const domain::SpellTrapLocation> candidates
		) noexcept -> std::optional<domain::SpellTrapLocation> override;

		[[nodiscard]] auto choose_triggers(const execution::TriggerDecision& decision) noexcept -> std::vector<std::size_t> override;
	};

	// ================================================================
	// Selector: 按玩家路由到对应控制器
	// ================================================================

	class LocalSelector final : public execution::Selector
	{
	public:
		using controllers_type = std::array<std::reference_wrapper<LocalController>, 2>;

	private:
		controllers_type controllers_;

		[[nodiscard]] auto controller_of(domain::Player player) const noexcept -> LocalController&;

	public:
		explicit LocalSelector(controllers_type controllers) noexcept;

		[[nodiscard]] auto select_cards(
			domain::Player player,
			const execution::Group& candidates,
			std::size_t min,
			std::size_t max
		) noexcept -> execution::Group override;

		[[nodiscard]] auto select_monster_location(
			domain::Player player,
			state::CardInstanceId card,
			std::span<const domain::MonsterLocation> candidates
		) noexcept -> std::optional<domain::MonsterLocation> override;

		[[nodiscard]] auto select_spell_trap_location(
			domain::Player player,
			state::CardInstanceId card,
			std::span<const domain::SpellTrapLocation> candidates
		) noexcept -> std::optional<domain::SpellTrapLocation> override;

		[[nodiscard]] auto choose_triggers(const execution::TriggerDecision& decision) noexcept -> std::vector<std::size_t> override;
	};

	// ================================================================
	// 本地对局会话
	// ================================================================

	class LocalDuelSession
	{
	public:
		enum class Status : std::uint8_t
		{
			MATCH_OVER,
			NO_ACTION,
			INVALID_ACTION,
			STEP_LIMIT,
		};

		class Result
		{
		public:
			Status status;
			std::optional<domain::Player> winner;
			std::string message;
		};

	private:
		std::array<std::unique_ptr<LocalController>, 2> controllers_;
		LocalSelector selector_;
		flow::Duel duel_;
		bool initialized_{false};

		[[nodiscard]] auto controller_of(domain::Player player) noexcept -> LocalController&;
		[[nodiscard]] auto controller_of(domain::Player player) const noexcept -> const LocalController&;

		auto apply(const LocalAction& action, domain::Player actor) noexcept -> bool;

	public:
		LocalDuelSession(std::unique_ptr<LocalController> first, std::unique_ptr<LocalController> second) noexcept;

		LocalDuelSession(const LocalDuelSession&) noexcept = delete;
		auto operator=(const LocalDuelSession&) noexcept -> LocalDuelSession& = delete;
		LocalDuelSession(LocalDuelSession&&) noexcept = delete;
		auto operator=(LocalDuelSession&&) noexcept -> LocalDuelSession& = delete;

		~LocalDuelSession() noexcept = default;

		[[nodiscard]] auto duel() noexcept -> flow::Duel&;
		[[nodiscard]] auto duel() const noexcept -> const flow::Duel&;

		[[nodiscard]] auto view(domain::Player player) const noexcept -> PlayerView;

		[[nodiscard]] auto load_scripts(const std::filesystem::path& directory) noexcept -> bool;
		[[nodiscard]] auto initialize(const flow::DuelConfig& config) noexcept -> bool;

		// 跑一局到结束/无动作/非法动作/步数上限
		[[nodiscard]] auto run(std::size_t max_steps = 10000) -> Result;
	};
}

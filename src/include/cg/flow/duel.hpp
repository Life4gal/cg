#pragma once

#include <filesystem>
#include <optional>
#include <span>
#include <vector>

#include <cg/dsl/types.hpp>
#include <cg/dsl/vocabulary.hpp>

#include <cg/execution/interpreter.hpp>
#include <cg/execution/script_registry.hpp>

#include <cg/state/game.hpp>

#include <cg/flow/battle.hpp>
#include <cg/flow/priority.hpp>
#include <cg/flow/turn.hpp>

namespace cg::flow
{
	// 最小决斗驱动:状态 + 回合/优先权/战斗流程 + 核心动作 + 卡脚本执行
	class Duel
	{
	public:
		// 回合内状态
		class TurnState
		{
		public:
			// 是否已抽卡(抽卡阶段)
			bool drew_in_draw_phase = false;
			// 是否已进行通常召唤
			bool normal_summoned = false;
			// 是否已进行灵摆召唤
			bool pendulum_summoned = false;
			// 本回合是否已进入过战斗阶段
			bool entered_battle_phase = false;
			// 本回合是否已进行过攻击
			bool attacked = false;

			// 本回合已攻击过的怪兽
			std::vector<state::CardInstanceId> attacked_monsters;
		};

	private:
		// ================================================================
		// 游戏状态
		// ================================================================

		state::Game game_;

		// ================================================================
		// 流程
		// ================================================================

		Turn turn_;
		Priority priority_;
		Battle battle_;

		// ================================================================
		// 回合内状态 / 胜负
		// ================================================================

		TurnState state_;
		// 判负方(抽卡失败等)
		std::optional<domain::Player> loser_;

		// ================================================================
		// 卡脚本与执行
		// ================================================================

		dsl::TypeRegistry types_;
		dsl::Vocabulary vocabulary_;
		execution::ScriptRegistry scripts_;
		execution::Interpreter interpreter_;

		// ================================================================
		// 内部动作
		// ================================================================

		auto perform_tribute_summon(state::CardInstanceId card, domain::FieldColumn column, std::span<const state::CardInstanceId> tributes, domain::Position position) noexcept -> bool;

		// 当前攻击力/守备力(含静态修正)
		[[nodiscard]] auto attack_value(state::CardInstanceId card) noexcept -> std::underlying_type_t<domain::AttackDefense>;
		[[nodiscard]] auto defense_value(state::CardInstanceId card) noexcept -> std::underlying_type_t<domain::AttackDefense>;

	public:
		Duel() noexcept;

		// ================================================================
		// 状态访问
		// ================================================================

		[[nodiscard]] auto game() noexcept -> state::Game&;
		[[nodiscard]] auto game() const noexcept -> const state::Game&;

		// [[nodiscard]] auto turn() noexcept -> Turn&;
		[[nodiscard]] auto turn() const noexcept -> const Turn&;

		// [[nodiscard]] auto priority() noexcept -> Priority&;
		[[nodiscard]] auto priority() const noexcept -> const Priority&;

		// [[nodiscard]] auto battle() noexcept -> Battle&;
		[[nodiscard]] auto battle() const noexcept -> const Battle&;

		// ================================================================
		// 胜负
		// ================================================================

		// 胜者(无人获胜/未结束返回空)
		[[nodiscard]] auto winner() const noexcept -> std::optional<domain::Player>;

		// ================================================================
		// 卡脚本
		// ================================================================

		// 从目录加载卡脚本(文件名即卡密)
		auto load_scripts(const std::filesystem::path& directory) noexcept -> bool;

		// [[nodiscard]] auto scripts() noexcept -> execution::ScriptRegistry&;
		[[nodiscard]] auto scripts() const noexcept -> const execution::ScriptRegistry&;

		// ================================================================
		// 回合 / 阶段
		// ================================================================

		// 抽卡阶段抽 1 张;卡组为空返回 false
		auto draw_for_turn() noexcept -> bool;

		// 推进阶段(需双方均已放弃优先权)
		auto change_phase() noexcept -> bool;

		// 当前优先权持有者放弃;双方均放弃时返回 true(窗口关闭)
		auto pass_priority() noexcept -> bool;

		// 结束回合(仅 END 阶段有效)
		auto end_turn() noexcept -> bool;

		// ================================================================
		// 召唤
		// ================================================================

		// 通常召唤(表侧攻击表示)
		auto normal_summon(state::CardInstanceId card, domain::FieldColumn column) noexcept -> bool;

		// 盖放怪兽(里侧守备表示)
		auto set_monster(state::CardInstanceId card, domain::FieldColumn column) noexcept -> bool;

		// 上级召唤(祭品数 = 等级 5~6:1,等级 7+:2)
		auto tribute_summon(state::CardInstanceId card, domain::FieldColumn column, std::span<const state::CardInstanceId> tributes) noexcept -> bool;

		// 上级盖放(里侧守备表示)
		auto tribute_set(state::CardInstanceId card, domain::FieldColumn column, std::span<const state::CardInstanceId> tributes) noexcept -> bool;

		// ================================================================
		// 战斗
		// ================================================================

		// 攻击宣言:target = 空表示直接攻击
		auto declare_attack(state::CardInstanceId attacker, std::optional<state::CardInstanceId> target) noexcept -> bool;

		// ================================================================
		// 效果
		// ================================================================

		// 主动发动(Ignition/Quick):需当前优先权持有者控制该卡
		auto activate_effect(state::CardInstanceId card, std::size_t effect_index) noexcept -> bool;

		// 事件发生后:收集触发 → 构建连锁 → 逆序结算
		auto trigger_event(dsl::TriggerSource source, std::optional<state::CardInstanceId> event_card = std::nullopt) noexcept -> bool;
	};
}

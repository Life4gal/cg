#pragma once

#include <filesystem>
#include <optional>
#include <span>
#include <vector>

#include <cg/dsl/types.hpp>
#include <cg/dsl/vocabulary.hpp>

#include <cg/execution/card_state_resolver.hpp>
#include <cg/execution/interpreter.hpp>
#include <cg/execution/script_registry.hpp>

#include <cg/state/game.hpp>

#include <cg/flow/event.hpp>
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
		// 使用次数记录
		// ================================================================

		class UsageKey
		{
		public:
			using owner_type = state::CardInstanceId::value_type;

			// state::CardInstanceId / domain::CardCode
			owner_type owner;

			dsl::UsageKind kind;
			// 计数器身份: 效果ID或互斥组代表
			dsl::EffectId counter;

			[[nodiscard]] constexpr auto operator<(const UsageKey& other) const noexcept -> bool
			{
				if (owner != other.owner)
				{
					return owner < other.owner;
				}
				if (kind != other.kind)
				{
					return kind < other.kind;
				}

				return counter < other.counter;
			}
		};

		// 互斥选择(按卡名、按回合)
		class MutexKey
		{
		public:
			using owner_type = state::CardInstanceId::value_type;

			// 卡号
			domain::CardCode owner;
			// 组代表
			dsl::EffectId group;

			[[nodiscard]] constexpr auto operator<(const MutexKey& other) const noexcept -> bool
			{
				if (owner != other.owner)
				{
					return owner < other.owner;
				}

				return group < other.group;
			}
		};

		// 键 = (拥有者ID, kind, 计数器);值 = 已使用次数
		std::map<UsageKey, dsl::UsageRestriction::size_type> usage_;

		// 键 = (卡号, 组代表);值 = 当前选中的效果ID
		std::map<MutexKey, dsl::EffectId> mutex_choices_;

		// ================================================================
		// 盖放记录
		// ================================================================

		// 卡实例 -> 盖放时的回合ID(盖放当回合不可发动)
		std::map<state::CardInstanceId, Turn::id_type> set_turns_;

		// ================================================================
		// 卡脚本与执行
		// ================================================================

		dsl::TypeRegistry types_;
		dsl::Vocabulary vocabulary_;

		execution::ScriptRegistry scripts_;
		execution::Interpreter interpreter_;
		execution::CardStateResolver card_state_;

		// 事件队列处理中(防重入)
		bool processing_events_;

		// 当前事件窗口(卡发动时点匹配)
		std::optional<Window> window_;

		// ================================================================
		// 内部动作
		// ================================================================

		// 如果当前可以召唤且指定位置为空,返回对应实例,否则返回空指针
		// - 检查是否是合适阶段(主要阶段1或者主要阶段2)
		// - 检查卡牌控制者是当前回合玩家
		// - 检查卡牌是怪兽类型
		// - 检查目标位置为空
		//
		// 不检查是否已通招         -- 通招点需检查是否已通招
		// 不检查来源               -- 通招点需检查是否在手牌
		// 不检查来源               -- 额外特招点需检查是否在额外卡组
		// 不检查子类型             -- 额外特招点需检查是否是对应特招类型
		// 不检查是否满足召唤条件   -- 通招点需要检查是否满足祭品要求
		// 不检查是否满足召唤条件   -- 额外特招点需要检查是否满足素材要求
		[[nodiscard]] auto summonable_instance_of(state::CardInstanceId card, domain::AnyMonsterPlacement placement) noexcept -> state::CardInstance*;

		auto perform_normal_summon(state::CardInstanceId card, domain::MonsterPlacement placement) noexcept -> bool;
		auto perform_tribute_summon(state::CardInstanceId card, domain::MonsterPlacement placement, std::span<const state::CardInstanceId> tributes) noexcept -> bool;

		// 结算后统一处理 Interpreter 产生的事件(破坏/特召等)
		auto process_pending_events() noexcept -> void;

		// 使用次数
		[[nodiscard]] auto usage_owner(dsl::UsageKind kind, const state::CardInstance& card) const noexcept -> UsageKey::owner_type;
		[[nodiscard]] auto make_usage_key(const dsl::Effect& effect, const state::CardInstance& card) const noexcept -> UsageKey;
		[[nodiscard]] auto make_mutex_key(const dsl::Effect& effect, const state::CardInstance& card) const noexcept -> MutexKey;
		[[nodiscard]] auto can_use(const dsl::Effect& effect, const state::CardInstance& card) const noexcept -> bool;
		auto mark_used(const dsl::Effect& effect, const state::CardInstance& card) noexcept -> void;

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

		// 同调召唤(1 Tuner + 1+ non-Tuner;等级合计等于目标)
		template<typename P>
			requires(utility::variant_index_v<P, domain::AnyMonsterPlacement> != utility::invalid_variant_index)
		auto synchro_summon(const state::CardInstanceId card, const P placement, const std::span<const state::CardInstanceId> materials) noexcept -> bool
		{
			return this->synchro_summon(card, {placement}, materials);
		}

		// 同调召唤(1 Tuner + 1+ non-Tuner;等级合计等于目标)
		auto synchro_summon(state::CardInstanceId card, domain::AnyMonsterPlacement placement, std::span<const state::CardInstanceId> materials) noexcept -> bool;

		// 超量召唤(2+ 同等级怪兽;作为超量素材)
		template<typename P>
			requires(utility::variant_index_v<P, domain::AnyMonsterPlacement> != utility::invalid_variant_index)
		auto xyz_summon(const state::CardInstanceId card, const P placement, const std::span<const state::CardInstanceId> materials) noexcept -> bool
		{
			return this->xyz_summon(card, {placement}, materials);
		}

		// 超量召唤(2+ 同等级怪兽;作为超量素材)
		auto xyz_summon(state::CardInstanceId card, domain::AnyMonsterPlacement placement, std::span<const state::CardInstanceId> materials) noexcept -> bool;

		// ================================================================
		// 卡牌发动(魔法/陷阱)
		// ================================================================

		// 发动魔法卡(当前仅手牌;场地/永续/装备留场,其余结算后送墓)
		auto activate_card(state::CardInstanceId card) noexcept -> bool;

		// 盖放魔法/陷阱卡(里侧守备表示)
		auto set_spell_trap(state::CardInstanceId card, domain::FieldColumn column) noexcept -> bool;

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

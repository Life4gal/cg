#pragma once

#include <filesystem>
#include <optional>
#include <random>
#include <span>
#include <vector>

#include <cg/dsl/types.hpp>
#include <cg/dsl/vocabulary.hpp>

#include <cg/execution/card_state_resolver.hpp>
#include <cg/execution/chain.hpp>
#include <cg/execution/decision.hpp>
#include <cg/execution/interpreter.hpp>
#include <cg/execution/script_registry.hpp>

#include <cg/state/game.hpp>

#include <cg/flow/battle.hpp>
#include <cg/flow/priority.hpp>
#include <cg/flow/turn.hpp>
#include <cg/flow/duel_config.hpp>

namespace cg::flow
{
	// 最小决斗驱动:状态 + 回合/优先权/战斗流程 + 核心动作 + 卡脚本执行
	class Duel
	{
	public:
		//

	private:
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

		// 当前攻击(攻击宣言后持续到伤害步骤结束时点关闭)
		class AttackState
		{
		public:
			// 攻击怪兽
			state::CardInstanceId attacker;
			// 攻击目标(空 = 直接攻击)
			std::optional<state::CardInstanceId> target;

			// 攻击者控制者/对方(攻击宣言时确定)
			domain::Player controller = domain::Player::FIRST;
			domain::Player opponent = domain::Player::SECOND;

			// 伤害计算时目标的表示形式
			domain::Position target_position = domain::Position::FACE_UP_ATTACK;
			// 目标是否因攻击翻面
			bool target_flipped = false;
			// 目标已离场,等待重新选择攻击目标
			bool replay_required = false;

			// 伤害计算确定的破坏(实际送墓在伤害步骤结束时)
			std::optional<state::CardInstanceId> destroy_attacker = std::nullopt;
			std::optional<state::CardInstanceId> destroy_target = std::nullopt;

			// 战斗伤害
			std::underlying_type_t<domain::AttackDefense> damage_to_opponent = 0;
			std::underlying_type_t<domain::AttackDefense> damage_to_controller = 0;
		};

		// 宣言中的召唤(尚未成功/无效)
		class SummonState
		{
		public:
			// 控制者
			domain::Player controller;
			// 目标位置
			domain::AnyMonsterPlacement placement;
			// 召唤方式
			domain::SummonMethod method;
			// 召唤的卡
			std::reference_wrapper<state::CardInstance> card;
			// 祭品/素材(已在宣言前处理)
			std::vector<std::reference_wrapper<state::CardInstance>> materials;
		};

		// 宣言中的灵摆召唤(尚未成功/无效)
		class PendulumSummonState
		{
		public:
			// 控制者
			domain::Player controller;

			class Entry
			{
			public:
				std::reference_wrapper<state::CardInstance> card;
				domain::AnyMonsterPlacement placement;
			};

			// 灵摆召唤使用:多张卡及各自目标区域
			std::vector<Entry> pendulum_cards;
		};

		// ================================================================
		// 流程
		// ================================================================

		Turn turn_;
		Priority priority_;
		Battle battle_;

		// ================================================================
		// 游戏状态
		// ================================================================

		state::Game game_;
		std::mt19937_64 random_;
		// 第一回合是否抽卡
		bool first_turn_draw_;

		// ================================================================
		// 流程状态
		// ================================================================

		// 当前攻击(空 = 无攻击进行中)
		std::optional<AttackState> attack_;

		// 当前宣言中的召唤(空 = 无)
		std::optional<SummonState> summon_;
		// 当前宣言中的灵摆召唤(空 = 无)
		std::optional<PendulumSummonState> pendulum_summon_;

		// 本次召唤宣言链是否无效了召唤
		bool summon_negated_{false};

		// 当前连锁(空 = 开放状态;下标 0 = Chain Link 1)
		execution::Chain chain_;

		// 当前事件时点(可能同时有多个事件)
		execution::EventWindowList windows_;

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
		std::map<state::CardInstanceId, domain::TurnId> set_turns_;

		// ================================================================
		// 卡脚本与执行
		// ================================================================

		dsl::TypeRegistry types_;
		dsl::Vocabulary vocabulary_;

		std::reference_wrapper<execution::Selector> selector_;
		execution::Interpreter interpreter_;

		// ================================================================
		// 内部动作
		// ================================================================

		// 开一个事件时点:收集触发 → 逐条发动入链 → 优先权交给最后发动者的对方
		// 返回是否建立了连锁
		auto open_timing(std::span<const execution::EventWindow> windows) noexcept -> bool;

		// 结算后统一处理 Interpreter 产生的事件(破坏/特召等)
		auto process_pending_events() noexcept -> void;

		// 结算整条连锁(LIFO)→ 回滚无效发动的次数 → 不能留场的卡送墓 → 开新时点
		auto resolve_chain() noexcept -> void;

		// ================================================================
		// 效果/发动合法性
		// ================================================================

		// 连锁中是否已存在同一张卡的同一效果
		// - 同一效果一链一次;同一张卡的不同效果可以同链
		[[nodiscard]] auto contains_duplicated_effect(state::CardInstanceId source, const dsl::Effect* effect) const noexcept -> bool;

		// 发动速度(由有效卡种/副种类与激活窗口派生)
		[[nodiscard]] auto activation_speed(const state::CardInstance& card, const dsl::ActivationKind& activation) const noexcept -> domain::EffectSpeed;

		// 连锁合法性:速度 1 不能作为 Chain Link 2+;其余须 ≥ 上一环速度
		[[nodiscard]] auto can_chain(domain::EffectSpeed speed) const noexcept -> bool;

		// 结算后是否留在场上(通常魔法/陷阱送墓;永续/场地/装备留场)
		[[nodiscard]] auto can_stay_on_field(const state::CardInstance& card) const noexcept -> bool;

		// 伤害步骤内是否允许手动发动该效果(不含自动收集的诱发效果)
		[[nodiscard]] auto can_activate_in_damage_step(const state::CardInstance& card, const dsl::Effect* effect) const noexcept -> bool;

		// 将已成功激活的效果接入连锁并转移优先权
		auto push_chain_link(
			domain::Player controller,
			domain::EffectSpeed speed,
			state::CardInstanceId source,
			const dsl::ActivatedEffect* activated,
			const dsl::Effect* effect,
			execution::Context context,
			bool mutex_inserted
		) noexcept -> void;

		// ================================================================
		// 召唤校验
		// ================================================================

		// 查找卡脚本中对应召唤方式的 procedure(空 = 无 procedure,不可召唤)
		[[nodiscard]] auto find_summon_procedure(const state::CardInstance& card, domain::SummonMethod method) const noexcept -> const dsl::SummonProcedure*;

		// 素材共同校验:非自身、不重复、同控制者、怪兽区、表侧表示
		[[nodiscard]] auto all_summon_materials_valid(const state::CardInstance& card, std::span<const state::CardInstanceId> materials) const noexcept -> bool;

		[[nodiscard]] auto all_summon_materials_valid(
			const state::CardInstance& card,
			std::span<const state::CardInstanceId> materials,
			utility::EnumMask<domain::Zone> allowed_zones
		) const noexcept -> bool;

		// 用 procedure 的素材谓词筛选素材($ = 候选)
		[[nodiscard]] auto match_summon_materials(
			const state::CardInstance& card,
			const dsl::Expression& predicate,
			std::span<const state::CardInstanceId> materials
		) noexcept -> std::optional<execution::Group>;

		// 融合:满足MaterialSpec条件
		[[nodiscard]] auto validate_fusion_materials(
			const state::CardInstance& summon_card,
			const dsl::FusionProcedure& procedure,
			std::span<const state::CardInstanceId> materials
		) noexcept -> bool;

		// 同调:至少 2 素材;tuner/non_tuner 谓词各命中且可用不同素材;等级合计 = 目标
		[[nodiscard]] auto validate_synchro_materials(
			const state::CardInstance& card,
			const dsl::SynchroProcedure& procedure,
			std::span<const state::CardInstanceId> materials
		) noexcept -> bool;

		// 超量:素材数 = procedure.count(0 = 至少 2);全部素材命中谓词;等级一致且 = 目标
		[[nodiscard]] auto validate_xyz_materials(
			const state::CardInstance& summon_card,
			const dsl::XyzProcedure& procedure,
			std::span<const state::CardInstanceId> materials
		) noexcept -> bool;

		// 链接:TODO
		[[nodiscard]] auto validate_link_materials(
			const state::CardInstance& summon_card,
			const dsl::LinkProcedure& procedure,
			std::span<const state::CardInstanceId> materials
		) noexcept -> bool;

		// 仪式: 
		[[nodiscard]] auto validate_ritual_materials(
			const state::CardInstance& summon_card,
			const dsl::RitualProcedure& procedure,
			std::span<const state::CardInstanceId> materials
		) noexcept -> bool;

		// ================================================================
		// 召唤动作
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

		[[nodiscard]] auto has_pending_summon() const noexcept -> bool;

		// 额外卡组/仪式召唤的通用前置校验
		// - 无待处理召唤、目标位置为表侧表示
		// - 卡牌位于指定区域且具备指定子类型
		[[nodiscard]] auto summonable_procedure_instance(
			state::CardInstanceId card,
			domain::AnyMonsterPlacement placement,
			domain::CardSubtype subtype,
			domain::Zone source_zone,
			bool allow_face_up_defense
		) noexcept -> state::CardInstance*;

		// 将召唤素材送墓/移出场地
		[[nodiscard]] auto release_summon_materials(std::span<const state::CardInstanceId> materials, domain::Reason reason) noexcept -> std::vector<std::reference_wrapper<state::CardInstance>>;
		[[nodiscard]] auto detach_summon_materials(std::span<const state::CardInstanceId> materials) noexcept -> std::vector<std::reference_wrapper<state::CardInstance>>;

		// 放置召唤成功卡牌并写入召唤记录
		auto place_and_record_summon(
			state::CardInstance& instance,
			domain::AnyMonsterPlacement placement,
			domain::Player controller,
			domain::SummonMethod method,
			std::span<const std::reference_wrapper<state::CardInstance>> materials
		) noexcept -> void;

		// 召唤宣言:保存待结算状态并打开 DECLARED 时点
		auto begin_summon(SummonState summon, domain::EventCode declared) noexcept -> bool;
		auto begin_summon(PendulumSummonState summon, domain::EventCode declared) noexcept -> bool;

		// 召唤成功:放置/记录/成功时点
		auto complete_summon(SummonState& summon) noexcept -> bool;
		auto complete_summon(PendulumSummonState& summon) noexcept -> bool;

		// 召唤无效:卡与未处理素材送墓,打开 NEGATED 时点
		auto negate_pending_summon(SummonState& summon) noexcept -> void;
		auto negate_pending_summon(PendulumSummonState& summon) noexcept -> void;

		// 宣言时点/连锁关闭后:成功或无效
		auto resolve_summon() noexcept -> void;

		auto perform_normal_summon(state::CardInstanceId card, domain::MonsterPlacement placement) noexcept -> bool;
		auto perform_tribute_summon(state::CardInstanceId card, domain::MonsterPlacement placement, std::span<const state::CardInstanceId> tributes) noexcept -> bool;

		// ================================================================
		// 使用限制/互斥
		// ================================================================

		// 使用限制相关
		[[nodiscard]] auto usage_owner(dsl::UsageKind kind, const state::CardInstance& card) const noexcept -> UsageKey::owner_type;
		[[nodiscard]] auto make_usage_key(const dsl::Effect& effect, const state::CardInstance& card) const noexcept -> UsageKey;
		[[nodiscard]] auto make_mutex_key(const dsl::Effect& effect, const state::CardInstance& card) const noexcept -> MutexKey;

		// 检查效果能否使用
		[[nodiscard]] auto can_use(const dsl::Effect& effect, const state::CardInstance& card) const noexcept -> bool;

		// 返回是否由本次调用建立了新的互斥选择
		auto mark_used(const dsl::Effect& effect, const state::CardInstance& card) noexcept -> bool;
		// 发动/效果被无效时回滚次数
		auto unmark_used(const dsl::Effect& effect, const state::CardInstance& card, bool mutex_inserted) noexcept -> void;

		// ================================================================
		// 战斗/伤害步骤
		// ================================================================

		// 卡是否仍在怪兽区(攻击者/目标可用性)
		[[nodiscard]] auto is_on_field(state::CardInstanceId card) const noexcept -> bool;

		// 目标是否可作为对手的攻击对象(空目标表示直接攻击)
		[[nodiscard]] auto is_valid_attack_target(domain::Player opponent, std::optional<state::CardInstanceId> target) const noexcept -> bool;

		// 打开 ATTACK_DECLARED 时点
		auto open_attack_declared(state::CardInstanceId attacker) noexcept -> void;

		// 推进当前攻击到下一个战斗/伤害步骤,并开启对应时点
		// - 攻击宣言窗口关闭 → 伤害步骤开始
		// - 伤害步骤各窗口关闭 → 下一个伤害步骤
		// - 伤害步骤结束时点关闭 → 清除攻击状态
		auto advance_battle() noexcept -> void;

		// 开一个无关联卡的事件时点(伤害步骤)
		auto open_battle_window(dsl::TriggerSource source) noexcept -> void;

		// 攻击结束:清除攻击状态并回到战斗步骤
		auto clear_attack() noexcept -> void;

		// 目标在伤害步骤开始时翻面并确定表示形式;目标已离场返回 false
		auto prepare_attack_target(AttackState& attack) noexcept -> bool;

		// 伤害计算;攻击者/目标已离场返回 false
		auto calculate_battle(AttackState& attack) noexcept -> bool;

		// 伤害步骤结束:送墓,开翻转/战斗破坏/伤害步骤结束时点
		auto finish_damage_step(AttackState& attack) noexcept -> void;

		// 伤害计算后时点:BATTLE_DAMAGE事件 + AFTER_CALCULATION
		auto open_after_calculation_window(AttackState& attack) noexcept -> void;

	public:
		// ================================================================
		// 构造 / 装配
		// ================================================================

		// 必须注入底层驱动;引擎不自动决策
		explicit Duel(execution::Selector& selector) noexcept;

		// Duel 内部保存对 Game/Selector 的引用, 禁止复制/移动以免悬空
		Duel(const Duel&) noexcept = delete;
		auto operator=(const Duel&) noexcept -> Duel& = delete;
		Duel(Duel&&) noexcept = delete;
		auto operator=(Duel&&) noexcept -> Duel& = delete;

		~Duel() noexcept;

		// 从目录加载卡脚本(文件名即卡密)
		auto load_scripts(const std::filesystem::path& directory) noexcept -> bool;

		// 加载卡脚本(文件名即卡密)
		auto load_script(const std::filesystem::path& file) noexcept -> bool;

		// 取出并清空脚本加载错误
		[[nodiscard]] auto take_script_errors() noexcept -> std::vector<std::string>;

		// 按配置建立初始对局: 卡组入区、洗牌、初始手牌、LP、先手
		auto initialize(const DuelConfig& config) noexcept -> bool;

		// ================================================================
		// 状态查询(只读;供驱动/UI)
		// ================================================================

		[[nodiscard]] auto game() const noexcept -> const state::Game&;

		[[nodiscard]] auto turn() const noexcept -> const Turn&;

		[[nodiscard]] auto priority() const noexcept -> const Priority&;

		[[nodiscard]] auto battle() const noexcept -> const Battle&;

		// 是否有攻击正在进行
		[[nodiscard]] auto pending_attack() const noexcept -> bool;

		// 是否等待攻击目标 replay(目标离场后重新选择)
		[[nodiscard]] auto pending_replay() const noexcept -> bool;

		// 是否有召唤正在宣言/等待结算
		[[nodiscard]] auto pending_summon() const noexcept -> bool;

		// 当前连锁(空 = 开放状态)
		[[nodiscard]] auto chain() const noexcept -> const execution::Chain&;

		// ================================================================
		// 胜负
		// ================================================================

		// 胜者(无人获胜/未结束返回空)
		[[nodiscard]] auto winner() const noexcept -> std::optional<domain::Player>;

		// 投降
		auto concede(domain::Player player) noexcept -> bool;

		// ================================================================
		// 驱动动作: 回合/阶段
		// ================================================================

		// 抽卡阶段抽 1 张;卡组为空返回 false
		auto draw_for_turn() noexcept -> bool;

		// 推进阶段(需双方均已放弃优先权)
		auto change_phase() noexcept -> bool;

		// 当前优先权持有者放弃
		// - 连锁中:结算整条连锁(返回 true)
		// - 开放状态:双方都放弃时窗口关闭(返回 true)
		// 返回值 = 一个窗口结束;若结算后建立了新连锁,需再次检查 chain()
		auto pass_priority() noexcept -> bool;

		// 结束回合(仅 END 阶段有效)
		auto end_turn() noexcept -> bool;

		// ================================================================
		// 驱动动作: 召唤
		// ================================================================

		// 通常召唤(表侧攻击表示)
		auto normal_summon(state::CardInstanceId card, domain::FieldColumn column) noexcept -> bool;

		// 翻转召唤(里侧守备表示 -> 表侧攻击表示)
		auto flip_summon(state::CardInstanceId card) noexcept -> bool;

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

		auto synchro_summon(state::CardInstanceId card, domain::AnyMonsterPlacement placement, std::span<const state::CardInstanceId> materials) noexcept -> bool;

		// 超量召唤(2+ 同等级怪兽;作为超量素材)
		template<typename P>
			requires(utility::variant_index_v<P, domain::AnyMonsterPlacement> != utility::invalid_variant_index)
		auto xyz_summon(const state::CardInstanceId card, const P placement, const std::span<const state::CardInstanceId> materials) noexcept -> bool
		{
			return this->xyz_summon(card, {placement}, materials);
		}

		auto xyz_summon(state::CardInstanceId card, domain::AnyMonsterPlacement placement, std::span<const state::CardInstanceId> materials) noexcept -> bool;

		template<typename P>
			requires(utility::variant_index_v<P, domain::AnyMonsterPlacement> != utility::invalid_variant_index)
		auto fusion_summon(const state::CardInstanceId card, const P placement, const std::span<const state::CardInstanceId> materials) noexcept -> bool
		{
			return this->fusion_summon(card, {placement}, materials);
		}

		auto fusion_summon(state::CardInstanceId card, domain::AnyMonsterPlacement placement, std::span<const state::CardInstanceId> materials) noexcept -> bool;

		template<typename P>
			requires(utility::variant_index_v<P, domain::AnyMonsterPlacement> != utility::invalid_variant_index)
		auto link_summon(const state::CardInstanceId card, const P placement, const std::span<const state::CardInstanceId> materials) noexcept -> bool
		{
			return this->link_summon(card, {placement}, materials);
		}

		auto link_summon(state::CardInstanceId card, domain::AnyMonsterPlacement placement, std::span<const state::CardInstanceId> materials) noexcept -> bool;

		template<typename P>
			requires(utility::variant_index_v<P, domain::AnyMonsterPlacement> != utility::invalid_variant_index)
		auto ritual_summon(const state::CardInstanceId card, const P placement, const std::span<const state::CardInstanceId> materials) noexcept -> bool
		{
			return this->ritual_summon(card, {placement}, materials);
		}

		auto ritual_summon(state::CardInstanceId card, domain::AnyMonsterPlacement placement, std::span<const state::CardInstanceId> materials) noexcept -> bool;

		template<typename P>
			requires(utility::variant_index_v<P, domain::AnyMonsterPlacement> != utility::invalid_variant_index)
		auto self_special_summon(const state::CardInstanceId card, const P placement) noexcept -> bool
		{
			return this->self_special_summon(card, {placement});
		}

		auto self_special_summon(state::CardInstanceId card, domain::AnyMonsterPlacement placement) noexcept -> bool;

		// 灵摆召唤:召唤列表中的怪兽
		auto pendulum_summon(std::span<const state::CardInstanceId> cards) noexcept -> bool;

		// ================================================================
		// 驱动动作: 卡牌发动(魔法/陷阱)
		// ================================================================

		// 将灵摆怪兽放置到自己的灵摆区域
		auto set_pendulum(state::CardInstanceId card, domain::PendulumSide side) noexcept -> bool;

		// 发动魔法卡(当前仅手牌;场地/永续/装备留场,其余结算后送墓)
		auto activate_card(state::CardInstanceId card) noexcept -> bool;

		// 盖放魔法/陷阱卡(里侧守备表示)
		auto set_spell_trap(state::CardInstanceId card, domain::FieldColumn column) noexcept -> bool;

		// ================================================================
		// 驱动动作: 战斗
		// ================================================================

		// 攻击宣言:target = 空表示直接攻击
		auto declare_attack(state::CardInstanceId attacker, std::optional<state::CardInstanceId> target) noexcept -> bool;

		// 目标离场后重新选择攻击目标:target = 空表示直接攻击
		auto replay_attack(std::optional<state::CardInstanceId> target) noexcept -> bool;

		// ================================================================
		// 驱动动作: 效果
		// ================================================================

		// 主动发动(Ignition/Quick):需当前优先权持有者控制该卡
		auto activate_effect(state::CardInstanceId card, std::size_t effect_index) noexcept -> bool;

		// ================================================================
		// 测试 / 调试支持
		// - 仅为 tests/ 与本地工具暴露;不属于稳定对外 API
		// - 生产驱动应使用上面的查询接口与 Selector 决策
		// ================================================================

		// 可变状态访问:测试构造用
		[[nodiscard]] auto game() noexcept -> state::Game&;

		// 脚本注册表:测试断言用
		[[nodiscard]] auto find_script(domain::CardCode code) const noexcept -> const dsl::CardScript*;

		// 当前事件时点:测试断言用;触发选择通过Selector
		[[nodiscard]] auto windows() const noexcept -> std::span<const execution::EventWindow>;

		// 原始事件注入:测试/调试用;正常流程应走召唤/战斗等动作
		auto trigger_event(dsl::TriggerSource source, std::optional<state::CardInstanceId> event_card = std::nullopt) noexcept -> bool;
	};
}

#pragma once

#include <core/utility/random.hpp>

#include <core/engine/playground.hpp>
#include <core/engine/toss_info.hpp>
#include <core/engine/turn_info.hpp>
#include <core/engine/event.hpp>
#include <core/engine/game_event.hpp>
#include <core/engine/chain.hpp>

namespace cg::engine
{
	class Duel;

	class Field
	{
	public:
		using size_type = Playground::size_type;

	private:
		std::reference_wrapper<Duel> duel_;

		Playground playground_;
		TossInfo toss_;
		TurnInfo turn_;
		Events events_;
		GameEvents game_events_;
		ChainLink chain_;

		[[nodiscard]] auto duel() const noexcept -> Duel&;

	public:
		// ==================== 场地 ====================

		class PlaygroundHandler
		{
			friend Field;

			std::reference_wrapper<Field> field_;

			explicit PlaygroundHandler(Field& field) noexcept;

			// 传播const
			[[nodiscard]] auto playground() noexcept -> Playground&;
			[[nodiscard]] auto playground() const noexcept -> const Playground&;

		public:
			// ---------- 起手 ----------

			// 设置玩家生命值
			auto set_life_point(domain::Player player, domain::life_point_type life_point) noexcept -> void;
			// 设置玩家起手手牌数量
			auto set_start_hand(domain::Player player, size_type start_hand) noexcept -> void;
			// 设置玩家每回合抽牌数量
			auto set_draw_count(domain::Player player, size_type draw_count) noexcept -> void;

			// 获取玩家生命值
			[[nodiscard]] auto life_point(domain::Player player) const noexcept -> domain::life_point_type;
			// 获取玩家起手手牌数量
			[[nodiscard]] auto start_hand(domain::Player player) const noexcept -> size_type;
			// 获取玩家每回合抽牌数量
			[[nodiscard]] auto draw_count(domain::Player player) const noexcept -> size_type;

			// ---------- 生命值 ----------

			// 可以承受指定生命值变化
			[[nodiscard]] auto affordable(domain::Player player, domain::life_point_type delta) const noexcept -> bool;
			// 承受指定生命值变化
			auto afford(domain::Player player, domain::life_point_type delta) noexcept -> void;

			// ---------- 抽牌 ----------

			// 起始抽牌
			auto start_draw(domain::Player player) noexcept -> void;
			// 从卡组抽牌
			auto draw(domain::Player player, size_type count) noexcept -> void;

			// ---------- 卡组 ----------

			// 往指定区域追加卡牌 -- 仅设置卡牌的控制者和其所在区域位置,一般用于初始化卡组/额外卡组
			auto add(domain::Player player, Card& card, domain::AutoZone zone) noexcept -> void;

			// 洗指定区域的牌
			auto shuffle(domain::Player player, domain::AutoZone zone, utility::Random& random) noexcept -> void;
			// 反转指定区域的牌
			auto reverse(domain::Player player, domain::AutoZone zone) noexcept -> void;

			// ---------- 区域 ----------

			// 获取指定区域的牌
			[[nodiscard]] auto select(domain::Player player, domain::Zone zone) const noexcept -> CardOptional;
			// 检查指定区域是否被占用
			[[nodiscard]] auto occupied(domain::Player player, domain::Zone zone) const noexcept -> bool;

			// 获取指定区域卡牌数量 -- zone只决定类型,一般不用这个接口
			[[nodiscard]] auto count(domain::Player player, domain::Zone zone) const noexcept -> size_type;
			// 获取指定自动区域卡牌数量
			[[nodiscard]] auto count(domain::Player player, domain::AutoZone zone) const noexcept -> size_type;
			// 获取指定场地区域卡牌数量
			[[nodiscard]] auto count(domain::Player player, domain::FieldZone zone) const noexcept -> size_type;

			// 获取指定区域所有可用位置 -- zone只决定类型,一般不用这个接口
			[[nodiscard]] auto free(domain::Player player, domain::Zone zone) const noexcept -> std::vector<Playground::size_type>;
			// 获取指定自动区域所有可用位置 -- 没有这个概念
			//
			// 获取指定场地区域所有可用位置
			[[nodiscard]] auto free(domain::Player player, domain::FieldZone zone) const noexcept -> std::vector<Playground::size_type>;

			// ---------- 移动卡牌 ----------

			// 将卡牌移动到指定位置
			// todo: 如何设计?
			//  - 衍生物 -> 直接移除(不进入任何区域)
			//  - 怪兽卡:
			//    - 有装备卡:
			//      - 取消所有装备卡的装备目标
			//      - 清空装备列表
			//      - 将所有装备卡送入墓地
			//    - 有超量素材:
			//      - 取消所有超量素材的叠放目标
			//      - 清空超量素材列表
			//      - 将所有超量素材送入墓地
			//  - 
			auto move_card(CardReference card, domain::Player player, domain::Zone zone, domain::Reason reason) noexcept -> bool;
		};

		// ==================== 掷骰子/硬币 ====================

		class TossInfoHandler
		{
			friend Field;

			std::reference_wrapper<Field> field_;

			explicit TossInfoHandler(Field& field) noexcept;

			// 传播const
			[[nodiscard]] auto toss_info() noexcept -> TossInfo&;
			[[nodiscard]] auto toss_info() const noexcept -> const TossInfo&;

		public:
			auto toss_dice(domain::Player player, TossInfo::size_type count, utility::Random& random) noexcept -> void;
			auto toss_coin(domain::Player player, TossInfo::size_type count, utility::Random& random) noexcept -> void;
		};

		// ==================== 回合/阶段 ====================

		class TurnHandler
		{
			friend Field;

			std::reference_wrapper<Field> field_;

			explicit TurnHandler(Field& field) noexcept;

			// 传播const
			[[nodiscard]] auto turn_info() noexcept -> TurnInfo&;
			[[nodiscard]] auto turn_info() const noexcept -> const TurnInfo&;

		public:
			auto set_turn_id(domain::TurnId id) noexcept -> void;
			auto set_phase(domain::Phase phase) noexcept -> void;
			auto set_turn_phase(domain::TurnPhase turn_phase) noexcept -> void;
			auto set_battle_step(domain::BattleStep battle_step) noexcept -> void;
			auto set_damage_step(domain::DamageStep damage_step) noexcept -> void;
			auto set_player(domain::Player player) noexcept -> void;
			auto set_can_battle(bool can_battle) noexcept -> void;
			auto set_can_normal_summon(bool can_normal_summon) noexcept -> void;
			auto set_can_special_summon(bool can_special_summon) noexcept -> void;

			[[nodiscard]] auto turn_id() const noexcept -> domain::TurnId;
			[[nodiscard]] auto phase() const noexcept -> domain::Phase;
			[[nodiscard]] auto turn_phase() const noexcept -> domain::TurnPhase;
			[[nodiscard]] auto battle_step() const noexcept -> domain::BattleStep;
			[[nodiscard]] auto damage_step() const noexcept -> domain::DamageStep;
			[[nodiscard]] auto can_battle() const noexcept -> bool;
			[[nodiscard]] auto can_normal_summon() const noexcept -> bool;
			[[nodiscard]] auto can_special_summon() const noexcept -> bool;

			[[nodiscard]] auto is_turn_id(domain::TurnId expected_id) const noexcept -> bool;
			[[nodiscard]] auto is_phase(domain::Phase expected_phase) const noexcept -> bool;
			[[nodiscard]] auto is_turn_phase(domain::TurnPhase expected_turn_phase) const noexcept -> bool;
			[[nodiscard]] auto is_battle_step(domain::BattleStep expected_battle_step) const noexcept -> bool;
			[[nodiscard]] auto is_damage_step(domain::DamageStep expected_damage_step) const noexcept -> bool;

			// 开始下一回合
			auto new_turn(domain::Player player) noexcept -> void;
			// 前进到指定阶段(可用于跳过某个阶段)
			auto advance_phase(domain::TurnPhase turn_phase) noexcept -> void;
		};

		// ==================== 事件 ====================

		class EventHandler
		{
			friend Field;

			std::reference_wrapper<Field> field_;

			explicit EventHandler(Field& field) noexcept;

			// 传播const
			[[nodiscard]] auto events() noexcept -> Events&;
			[[nodiscard]] auto events() const noexcept -> const Events&;

		public:
			// 触发事件
			auto raise(const Event& event) noexcept -> void;

			// 目标类型事件是否被触发
			[[nodiscard]] auto triggered(domain::EventType type) const noexcept -> bool;
		};

		// ==================== 游戏事件 ====================

		class GameEventHandler
		{
			friend Field;

			std::reference_wrapper<Field> field_;

			explicit GameEventHandler(Field& field) noexcept;

			// 传播const
			[[nodiscard]] auto events() noexcept -> GameEvents&;
			[[nodiscard]] auto events() const noexcept -> const GameEvents&;

		public:
			// 触发事件
			auto emit(domain::GameEvent event) noexcept -> void;
		};

		// ==================== 连锁 ====================

		class ChainHandler
		{
			friend Field;

			std::reference_wrapper<Field> field_;

			explicit ChainHandler(Field& field) noexcept;

			// 传播const
			[[nodiscard]] auto chain() noexcept -> ChainLink&;
			[[nodiscard]] auto chain() const noexcept -> const ChainLink&;

		public:
			//
		};

		// ==================== 输入 ====================


		// ==================== | ====================

		explicit Field(Duel& duel) noexcept;

		[[nodiscard]] auto playground() noexcept -> PlaygroundHandler;
		[[nodiscard]] auto random() noexcept -> TossInfoHandler;
		[[nodiscard]] auto turn() noexcept -> TurnHandler;
		[[nodiscard]] auto events() noexcept -> EventHandler;
		[[nodiscard]] auto game_events() noexcept -> GameEventHandler;
	};
}

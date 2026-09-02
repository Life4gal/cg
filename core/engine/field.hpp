#pragma once

#include <core/engine/playground.hpp>
#include <core/engine/toss_info.hpp>
#include <core/engine/turn_info.hpp>

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

			// 改变生命值
			auto update_life_point(domain::Player player, domain::life_point_type delta) noexcept -> void;

			// 起始抽牌
			auto start_draw(domain::Player player) noexcept -> void;
			// 从卡组抽牌
			auto draw(domain::Player player, size_type count) noexcept -> void;
			// 切洗卡组
			auto shuffle_deck(domain::Player player) noexcept -> void;
			// 反转卡组
			auto reverse_deck(domain::Player player) noexcept -> void;
			// 切洗额外卡组
			auto shuffle_extra_deck(domain::Player player) noexcept -> void;
			// 切洗手牌
			auto shuffle_hand(domain::Player player) noexcept -> void;

			// 检查指定区域是否被占用
			[[nodiscard]] auto occupied(domain::Player player, domain::Zone zone) const noexcept -> bool;
			// 获取指定区域的卡牌
			[[nodiscard]] auto select(domain::Player player, domain::Zone zone) const noexcept -> CardOptional;
			// 获取指定区域的卡牌数量
			[[nodiscard]] auto count(domain::Player player, domain::Zone zone) const noexcept -> size_type;
			// 获取指定场地区域可用区域
			[[nodiscard]] auto free_area(domain::Player player, domain::Zone zone) const noexcept -> std::vector<domain::Zone::size_type>;

			// 将卡牌从其所在区域移除
			auto remove_card(CardReference card) noexcept -> void;
			// 将卡牌移动到指定位置
			auto move_card(CardReference card, domain::Player player, domain::Zone zone) noexcept -> void;
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
			auto toss_dice(domain::Player player, std::size_t count) noexcept -> void;
			auto toss_coin(domain::Player player, std::size_t count) noexcept -> void;

			[[nodiscard]] auto get_dice(domain::Player player) const noexcept -> const TossInfo::dice_results_type&;
			[[nodiscard]] auto get_coin(domain::Player player) const noexcept -> const TossInfo::coin_results_type&;
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

		// ==================== | ====================

		explicit Field(Duel& duel) noexcept;

		[[nodiscard]] auto playground() noexcept -> PlaygroundHandler;
		[[nodiscard]] auto random() noexcept -> TossInfoHandler;
		[[nodiscard]] auto turn() noexcept -> TurnHandler;
	};
}

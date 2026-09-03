#pragma once

#include <core/utility/random.hpp>

#include <core/engine/series_registry.hpp>
#include <core/engine/prototype_registry.hpp>
#include <core/engine/card.hpp>
#include <core/engine/effect.hpp>
#include <core/engine/field.hpp>
#include <core/engine/api.hpp>

#include <core/script/host.hpp>

namespace cg::engine
{
	class Duel
	{
	public:
		using random_type = utility::Random;

		class CardInstanceIdWrapper : public utility::Enum<
					domain::CardInstanceId,
					// +, +=, -, -=
					utility::EnumSupport::ARITHMETIC
				> {};

		class EffectInstanceIdWrapper : public utility::Enum<
					domain::EffectInstanceId,
					// +, +=, -, -=
					utility::EnumSupport::ARITHMETIC
				> {};

	private:
		mutable random_type random_;

		Api api_;
		std::unique_ptr<script::Host> host_;
		PrototypeRegistry prototype_registry_;
		SeriesRegistry series_registry_;

		Field field_;

		// 所有卡牌实例
		std::list<Card> cards_;
		// 所有效果实例
		std::list<Effect> effects_;

		// 卡牌实例ID生成器
		CardInstanceIdWrapper card_instance_id_generator_;
		// 效果实例ID生成器
		EffectInstanceIdWrapper effect_instance_id_generator_;

		// 注册卡牌原型
		auto register_prototype(domain::CardCode code) noexcept -> const Prototype*;

		// 获取卡牌实例ID
		[[nodiscard]] auto make_card_instance_id() noexcept -> domain::CardInstanceId;

		// 获取效果实例ID
		[[nodiscard]] auto make_effect_instance_id() noexcept -> domain::EffectInstanceId;

	public:
		explicit Duel(random_type::result_type seed) noexcept;

		Duel() noexcept;

		Duel(const Duel&) noexcept = delete;
		auto operator=(const Duel&) noexcept -> Duel& = delete;
		Duel(Duel&&) noexcept;
		auto operator=(Duel&&) noexcept -> Duel&;

		~Duel() noexcept;

		// ==================== host ====================

		// 绑定脚本宿主
		auto bind_host(std::unique_ptr<script::Host> host) noexcept -> void;

		// ==================== field ====================

		// 初始化玩家信息
		auto set_player_info(domain::Player player, domain::life_point_type life_point, Field::size_type start_hand, Field::size_type draw_count) noexcept -> void;

		// 添加卡牌实例到指定区域
		auto register_card(domain::CardCode code, domain::Player player, domain::Zone zone) noexcept -> bool;

		// ==================== DUEL! ====================

		// 开始决斗
		auto start() noexcept -> void;
	};
}

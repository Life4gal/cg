#pragma once

#include <string>

#include <core/card_target.hpp>

namespace cg
{
	class Dual;
	class Player;

	enum class CardCategory : std::uint8_t
	{
		NONE = 0,

		// ====================================

		MONSTER = 1 << 0,
		SPELL = 1 << 1,
		TRAP = 1 << 2,
	};

	class Card
	{
	public:
		using code_type = std::uint32_t;

	protected:
		CardCategory card_category_;

		code_type code_;
		code_type game_code_;

		std::string name_;
		std::string game_name_;

	public:
		Card() noexcept;

		Card(const Card&) noexcept = delete;
		auto operator=(const Card&) noexcept -> Card& = delete;

		Card(Card&&) noexcept;
		auto operator=(Card&&) noexcept -> Card&;

		virtual ~Card() noexcept;

		// 获取卡片类型
		// MONSTER -> 怪兽
		// SPELL -> 魔法
		// TRAP -> 陷阱
		// MONSTER | TRAP -> 陷阱怪兽
		[[nodiscard]] auto card_category() const noexcept -> CardCategory;

		// 检查当前决斗能否发动该卡效果
		[[nodiscard]] virtual auto can_activate(const Dual& dual, const Player& owner) const noexcept -> bool = 0;

		// 获取所有可用于对象的卡
		// 如果不去对象则返回空?如何和无法取得有效对象做区分?
		[[nodiscard]] virtual auto get_activate_targets(const Dual& dual, const Player& owner) const noexcept -> TargetCards = 0;

		// 使用指定对象尝试发动
		// 返回是否发动成功
		[[nodiscard]] virtual auto try_activate(const Dual& dual, const Player& owner, TargetCards& targets) noexcept -> bool = 0;

		// todo: 其他接口
	};
}

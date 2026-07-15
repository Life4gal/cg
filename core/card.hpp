#pragma once

#include <string>

#include <core/defs.hpp>

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

		// 检查能否发动效果
		[[nodiscard]] virtual auto can_activate(const Dual& dual, const Player& owner) const noexcept -> bool = 0;

		// 尝试发动效果
		// todo: 以某些卡为对象?
		virtual auto try_activate(Dual& dual, Player& owner) noexcept -> void = 0;

		// todo
	};
}

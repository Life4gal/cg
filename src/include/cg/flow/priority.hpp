#pragma once

#include <cg/domain/player.hpp>

namespace cg::flow
{
	// 每个阶段/步骤内,回合玩家拥有优先权
	// 当回合玩家放弃优先权后,非回合玩家获得优先权
	// 双方都放弃后,才能推进阶段/步骤
	class Priority
	{
	public:
		// 直接使用domain::RelativePlayer?
		enum class Holder : std::uint8_t
		{
			// 回合玩家
			TURN_PLAYER,
			// 非回合玩家
			NON_TURN_PLAYER,
		};

	private:
		// 当前持有优先权的玩家
		Holder holder_;
		// 当前持有者是否已放弃优先权
		bool passed_;

	public:
		Priority() noexcept;

		// 当前持有者
		[[nodiscard]] auto holder() const noexcept -> Holder;

		// 当前持有者是否已放弃优先权
		[[nodiscard]] auto passed() const noexcept -> bool;

		// 双方是否都已放弃优先权(可以推进阶段/步骤)
		[[nodiscard]] auto both_passed() const noexcept -> bool;

		// 放弃优先权
		// 如果回合玩家放弃 → 转移给非回合玩家
		// 如果非回合玩家放弃 → 双方都放弃,可以推进
		auto pass() noexcept -> void;

		// 发动后:把优先权交给发动者的对方
		// 连锁由「未发动最近连锁的一方」继续响应
		auto transfer_from(domain::Player activator, domain::Player turn_player) noexcept -> void;

		// 重置优先权(新阶段/步骤开始时调用)
		// 回合玩家重新获得优先权,放弃标记清除
		auto reset() noexcept -> void;

		// 获取当前持有优先权的玩家(绝对代号)
		[[nodiscard]] auto holder_player(domain::Player turn_player) const noexcept -> domain::Player;
	};
}

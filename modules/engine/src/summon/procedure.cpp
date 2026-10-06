#include <engine/summon/procedure.hpp>

#include <algorithm>
#include <ranges>

#include <engine/state/zone.hpp>
#include <engine/state/card.hpp>

#include <libassert/assert.hpp>

namespace cg::engine
{
	namespace
	{
		// 检查指定区域是否可用
		[[nodiscard]] auto placement_available(const GameState& state, const SummonRequest& request, SummonPlacement placement) noexcept -> bool
		{
			const auto occupant = std::visit(
				[&](const auto& location) noexcept -> CardInstanceId
				{
					return engine::select_location_card(state, request.controller, location);
				},
				placement
			);

			// 目标位置不存在卡牌实例
			// 可用
			if (!occupant.valid())
			{
				return true;
			}

			// 目标位置存在卡牌实例
			// 如果该卡是召唤素材,那依然视为可用
			if (std::ranges::contains(request.materials, occupant))
			{
				return true;
			}

			// 还有什么例外情况?
			//

			// 不可用
			return false;
		}

		// 检查额外怪兽区是否可用
		[[nodiscard]] auto is_extra_monster_zones_blocked(const GameState& state,const domain::Player player, const SummonRequest& request) noexcept -> bool
		{
			for (const auto slot : domain::ExtraMonsterLocation::slots)
			{
				const auto location = domain::ExtraMonsterLocation{.slot = slot};
				const auto occupant = select_location_card(state, player, location);
				if (!occupant.valid())
				{
					// 空的额外怪兽区
					continue;
				}

				const auto controller = controller_of(state, occupant);
				// 合法的ID应该总能找到合法的实例
				LIBASSERT_DEBUG_ASSERT(controller.has_value());
				if (!controller.has_value() || *controller != player)
				{
					// 对方占用不阻止本方使用另一个
					continue;
				}

				// 如果占用者是召唤素材,则也视为可用
				if (std::ranges::contains(request.materials, occupant))
				{
					continue;
				}

				// 如果已经使用了一个额外怪兽区,则有且仅有可以形成Extra-Link才可以使用
				for (const auto other_slot : domain::ExtraMonsterLocation::slots)
				{
					if (other_slot == slot)
					{
						continue;
					}

					const auto other_location = domain::ExtraMonsterLocation{.slot = other_slot};
					const auto other_occupant = select_location_card(state, player, other_location);
					if (occupant.valid())
					{
						continue;
					}

					// TODO
				}
			}
		}

		// 解析连接召唤可选落点
		// - placement == domain::MonsterZone:
		//   指定区域可用且被场上任一连接怪兽的连接箭头指向
		// - placement == domain::ExtraMonsterZone:
		//   1.我方未占用任意额外怪兽区且指定区域可用
		//   2.满足`Extra Link`条件
		[[nodiscard]] auto resolve_link_placement(const GameState& state, const SummonRequest& request) noexcept -> std::optional<SummonPlacement>
		{
			// 必须是连接召唤
			LIBASSERT_DEBUG_ASSERT(request.method == domain::SummonMethod::LINK);
			if (request.method != domain::SummonMethod::LINK)
			{
				return std::nullopt;
			}
		}
	}
}

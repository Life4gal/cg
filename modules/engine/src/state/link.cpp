#include <engine/state/link.hpp>

#include <engine/state/state.hpp>

namespace cg::engine
{
	auto collect_link_targets(const GameState& state, CardInstanceId card) noexcept -> std::vector<LinkTargetCell>
	{
		const auto* instance = find_instance(state, card);
		if (instance == nullptr)
		{
			return {};
		}

		const auto* definition = find_definition(state, instance->code);
	}

}

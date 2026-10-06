#include <engine/state/state.hpp>

#include <algorithm>

namespace cg::engine
{
	auto has_definition(const GameState& state, const CardCode code) noexcept -> bool
	{
		return state.card_definitions.contains(code);
	}

	auto find_definition(const GameState& state, const CardCode code) noexcept -> const CardDefinition*
	{
		if (const auto it = state.card_definitions.find(code);
			it != state.card_definitions.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	auto has_instance(const GameState& state, const CardInstanceId card) noexcept -> bool
	{
		return std::ranges::contains(state.card_instances, card, &CardInstance::id);
	}

	auto find_instance(const GameState& state, const CardInstanceId card) noexcept -> const CardInstance*
	{
		if (const auto it = std::ranges::find(state.card_instances, card, &CardInstance::id);
			it != state.card_instances.end())
		{
			return &*it;
		}

		return nullptr;
	}
}

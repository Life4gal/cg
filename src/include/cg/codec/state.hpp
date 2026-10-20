#pragma once

#include <cg/state/id.hpp>

#include <cg/codec/codec.hpp>

// ReSharper disable once CppRedundantNamespaceDefinition
namespace cg::codec
{
	template<>
	class ValueCodec<state::CardInstanceId>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_number_unsigned();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<state::CardInstanceId>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto id = json.get<state::CardInstanceId::value_type>();
			return state::CardInstanceId{id};
		}

		[[nodiscard]] static auto encode(const state::CardInstanceId id) noexcept -> nlohmann::json
		{
			return id.raw();
		}
	};
}

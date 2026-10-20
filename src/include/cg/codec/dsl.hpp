#pragma once

#include <cg/dsl/id.hpp>

#include <cg/codec/codec.hpp>

// ReSharper disable once CppRedundantNamespaceDefinition
namespace cg::codec
{
	template<>
	class ValueCodec<dsl::EffectId>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_number_unsigned();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<dsl::EffectId>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto id = json.get<dsl::EffectId::value_type>();
			return dsl::EffectId{id};
		}

		[[nodiscard]] static auto encode(const dsl::EffectId id) noexcept -> nlohmann::json
		{
			return id.raw();
		}
	};
}

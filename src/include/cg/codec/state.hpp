#pragma once

#include <cg/state/id.hpp>

#include <cg/codec/codec.hpp>

// ReSharper disable once CppRedundantNamespaceDefinition
namespace cg::codec
{
	template<typename T>
	// state::CardInstanceId / state::EffectId 结构完全相同,一并处理
		requires(std::is_same_v<T, state::CardInstanceId> || std::is_same_v<T, state::EffectId>)
	class ValueCodec<T>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_number_unsigned();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<T>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto id = json.get<typename T::value_type>();
			return T{id};
		}

		[[nodiscard]] static auto encode(const T& id) noexcept -> nlohmann::json
		{
			return id.raw();
		}
	};
}

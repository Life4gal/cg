#pragma once

#include <array>
#include <vector>
#include <optional>
#include <variant>

#include <cg/utility/enum.hpp>
#include <cg/utility/variant.hpp>

#include <nlohmann/json.hpp>

namespace cg::codec
{
	template<typename T>
	class ValueCodec;

	// array
	template<typename T, std::size_t N>
	class ValueCodec<std::array<T, N>>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_array() || json.size() != N)
			{
				return false;
			}

			// elements
			// return std::ranges::all_of(json, &ValueCodec<T>::valid);
			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<std::array<T, N>>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			std::array<T, N> result;

			for (std::size_t index = 0;
			     const auto& item: json)
			{
				auto r = ValueCodec<T>::decode(item);
				if (!r.has_value())
				{
					return std::nullopt;
				}

				result[index] = *std::move(r);
				index += 1;
			}

			return result;
		}

		[[nodiscard]] static auto encode(const std::array<T, N>& data) noexcept -> nlohmann::json
		{
			auto array = nlohmann::json::array();

			for (const auto& element: data)
			{
				array.push_back(ValueCodec<T>::encode(element));
			}

			return array;
		}
	};

	// vector
	template<typename T>
	class ValueCodec<std::vector<T>>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_array())
			{
				return false;
			}

			// elements
			// return std::ranges::all_of(json, &ValueCodec<T>::valid);
			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<std::vector<T>>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			std::vector<T> result;
			result.reserve(json.size());

			for (const auto& item: json)
			{
				auto r = ValueCodec<T>::decode(item);
				if (!r.has_value())
				{
					return std::nullopt;
				}

				result.push_back(*std::move(r));
			}

			return result;
		}

		[[nodiscard]] static auto encode(const std::vector<T>& data) noexcept -> nlohmann::json
		{
			auto array = nlohmann::json::array();

			for (const auto& element: data)
			{
				array.push_back(ValueCodec<T>::encode(element));
			}

			return array;
		}
	};

	// optional
	template<typename T>
	class ValueCodec<std::optional<T>>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_null() || ValueCodec<T>::valid(json);
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<std::optional<T>>
		{
			if (json.is_null())
			{
				return std::optional<std::optional<T>>{std::nullopt};
			}

			auto value = ValueCodec<T>::decode(json);
			if (!value.has_value())
			{
				return std::nullopt;
			}

			return std::optional<std::optional<T>>{*std::move(value)};
		}

		[[nodiscard]] static auto encode(const std::optional<T>& data) noexcept -> nlohmann::json
		{
			if (!data.has_value())
			{
				return nullptr;
			}

			return ValueCodec<T>::encode(*data);
		}
	};

	template<typename T>
	class VariantWrap;

	// 额外包装,不特化ValueCodec<variant>
	template<utility::variant_or_like_t T>
	class ValueCodec<VariantWrap<T>>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// variant_index
			if (!json.contains("variant_index") || !json.at("variant_index").is_number_unsigned())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<T>
		{
			const auto variant_index_it = json.find("variant_index");
			const auto variant_index = variant_index_it.value().get<nlohmann::json::number_unsigned_t>();

			// 下标超出变体数量,直接失败
			if (variant_index >= utility::variant_size_v<T>)
			{
				return std::nullopt;
			}

			std::optional<T> result;
			[&]<std::size_t... Index>(std::index_sequence<Index...>) noexcept -> void
			{
				const auto process = [&]<std::size_t I>() noexcept -> bool
				{
					if (I != variant_index)
					{
						return true;
					}

					using type = utility::variant_alternative_t<I, T>;

					if (auto r = ValueCodec<type>::decode(json);
						r.has_value())
					{
						result = T{*std::move(r)};
					}

					// 找到目标,不论decode是否成功都停下来
					return false;
				};

				// 第一个返回false就停下来
				(process.template operator()<Index>() && ...);
			}(std::make_index_sequence<utility::variant_size_v<T>>{});

			return result;
		}

		[[nodiscard]] static auto encode(const T& any) noexcept -> nlohmann::json
		{
			auto object = std::visit(
				[&]<typename Type>(const Type& t) noexcept -> nlohmann::json
				{
					return ValueCodec<Type>::encode(t);
				},
				any
			);

			object["variant_index"] = any.index();

			return object;
		}
	};

	// utility::EnumMask
	template<typename T>
	class ValueCodec<utility::EnumMask<T>>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_string();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<utility::EnumMask<T>>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto& s = json.get_ref<const nlohmann::json::string_t&>();
			return utility::from_mask_string<T>(s);
		}

		[[nodiscard]] static auto encode(const utility::EnumMask<T>& mask) noexcept -> nlohmann::json
		{
			return utility::to_string(mask);
		}
	};
}

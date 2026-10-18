#pragma once

#include <algorithm>
#include <array>
#include <expected>
#include <string>
#include <string_view>

#include <cg/utility/enum.hpp>
#include <cg/utility/variant.hpp>

#include <cg/driver/request.hpp>
#include <cg/driver/response.hpp>

#include <cg/server/message.hpp>

#include <nlohmann/json.hpp>

namespace cg::server
{
	namespace codec_detail
	{
		template<typename T>
		class Wrap;
	}

	template<typename T>
	class ValueCodec;

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

	// 额外包装,不特化ValueCodec<variant>
	template<utility::variant_or_like_t T>
	class ValueCodec<codec_detail::Wrap<T>>
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

	// ==============================================================================================
	// DOMAIN
	// ==============================================================================================

	template<>
	class ValueCodec<domain::CardCode>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_number_unsigned();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<domain::CardCode>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto card = json.get<nlohmann::json::number_unsigned_t>();
			return domain::CardCode{static_cast<std::underlying_type_t<domain::CardCode>>(card)};
		}

		[[nodiscard]] static auto encode(const domain::CardCode card) noexcept -> nlohmann::json
		{
			return std::to_underlying(card);
		}
	};

	template<>
	class ValueCodec<domain::Player>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_string();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<domain::Player>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto& s = json.get_ref<const nlohmann::json::string_t&>();
			return utility::from_string<domain::Player>(s);
		}

		[[nodiscard]] static auto encode(const domain::Player player) noexcept -> nlohmann::json
		{
			return utility::to_string(player);
		}
	};

	template<>
	class ValueCodec<domain::PendulumSide>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_string();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<domain::PendulumSide>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto& s = json.get_ref<const nlohmann::json::string_t&>();
			return utility::from_string<domain::PendulumSide>(s);
		}

		[[nodiscard]] static auto encode(const domain::PendulumSide side) noexcept -> nlohmann::json
		{
			return utility::to_string(side);
		}
	};

	template<>
	class ValueCodec<domain::Position>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_string();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<domain::Position>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto& s = json.get_ref<const nlohmann::json::string_t&>();
			return utility::from_string<domain::Position>(s);
		}

		[[nodiscard]] static auto encode(const domain::Position position) noexcept -> nlohmann::json
		{
			return utility::to_string(position);
		}
	};

	template<>
	class ValueCodec<domain::SummonMethod>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_string();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<domain::SummonMethod>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto& s = json.get_ref<const nlohmann::json::string_t&>();
			return utility::from_string<domain::SummonMethod>(s);
		}

		[[nodiscard]] static auto encode(const domain::SummonMethod method) noexcept -> nlohmann::json
		{
			return utility::to_string(method);
		}
	};

	template<>
	class ValueCodec<domain::NormalSummonType>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_string();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<domain::NormalSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto& s = json.get_ref<const nlohmann::json::string_t&>();
			return utility::from_string<domain::NormalSummonType>(s);
		}

		[[nodiscard]] static auto encode(const domain::NormalSummonType type) noexcept -> nlohmann::json
		{
			return utility::to_string(type);
		}
	};

	template<>
	class ValueCodec<domain::SummonMaterialSource>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_string();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<domain::SummonMaterialSource>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto& s = json.get_ref<const nlohmann::json::string_t&>();
			return utility::from_string<domain::SummonMaterialSource>(s);
		}

		[[nodiscard]] static auto encode(const domain::SummonMaterialSource source) noexcept -> nlohmann::json
		{
			return utility::to_string(source);
		}
	};

	template<>
	class ValueCodec<domain::Phase>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_string();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<domain::Phase>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto& s = json.get_ref<const nlohmann::json::string_t&>();
			return utility::from_string<domain::Phase>(s);
		}

		[[nodiscard]] static auto encode(const domain::Phase phase) noexcept -> nlohmann::json
		{
			return utility::to_string(phase);
		}
	};

	template<>
	class ValueCodec<domain::SelectionContext>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_string();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<domain::SelectionContext>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto& s = json.get_ref<const nlohmann::json::string_t&>();
			return utility::from_string<domain::SelectionContext>(s);
		}

		[[nodiscard]] static auto encode(const domain::SelectionContext context) noexcept -> nlohmann::json
		{
			return utility::to_string(context);
		}
	};

	template<>
	class ValueCodec<domain::RejectReason>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_string();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<domain::RejectReason>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto& s = json.get_ref<const nlohmann::json::string_t&>();
			return utility::from_string<domain::RejectReason>(s);
		}

		[[nodiscard]] static auto encode(const domain::RejectReason reason) noexcept -> nlohmann::json
		{
			return utility::to_string(reason);
		}
	};

	template<typename T>
	// domain::DeckLocation / domain::ExtraDeckLocation / domain::HandLocation / domain::GraveyardLocation / domain::BanishedLocation 结构完全相同,一并处理
		requires(
			std::is_same_v<T, domain::DeckLocation> ||
			std::is_same_v<T, domain::ExtraDeckLocation> ||
			std::is_same_v<T, domain::HandLocation> ||
			std::is_same_v<T, domain::GraveyardLocation> ||
			std::is_same_v<T, domain::BanishedLocation>
		)
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

			const auto index = json.get<typename T::size_type>();
			return T{.index = index};
		}

		[[nodiscard]] static auto encode(const T& data) noexcept -> nlohmann::json
		{
			return data.index;
		}
	};

	template<typename T>
	// domain::MonsterLocation / domain::ExtraMonsterLocation / domain::SpellTrapLocation 结构完全相同,一并处理
		requires(
			std::is_same_v<T, domain::MonsterLocation> ||
			std::is_same_v<T, domain::ExtraMonsterLocation> ||
			std::is_same_v<T, domain::SpellTrapLocation>
		)
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

			const auto column = json.get<std::underlying_type_t<domain::FieldColumn>>();
			return T{.column = static_cast<domain::FieldColumn>(column)};
		}

		[[nodiscard]] static auto encode(const T& data) noexcept -> nlohmann::json
		{
			return std::to_underlying(data.column);
		}
	};

	template<typename T>
	// domain::Location / domain::FieldLocation / domain::AnyMonsterLocation 处理方式完全相同,一并处理
		requires(
			std::is_same_v<T, domain::Location> ||
			std::is_same_v<T, domain::FieldLocation> ||
			std::is_same_v<T, domain::AnyMonsterLocation>
		)
	class ValueCodec<T>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// type相比于variant_index更方便校验
			if (json.contains("type") && !json.at("type").is_string())
			{
				return false;
			}

			if (!ValueCodec<codec_detail::Wrap<T>>::valid(json))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<T>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			// 如果有type则按照type匹配
			if (const auto type_it = json.find("type");
				type_it != json.end())
			{
				const auto do_decode = [&]<typename L> noexcept -> std::optional<T>
				{
					if (const auto r = ValueCodec<L>::decode(json);
						r.has_value())
					{
						return T{*std::move(r)};
					}

					return std::nullopt;
				};

				const auto& type = type_it.value().get_ref<const nlohmann::json::string_t&>();

				if constexpr (std::is_same_v<T, domain::Location>)
				{
					if (type == "deck")
					{
						return do_decode.template operator()<domain::DeckLocation>();
					}
					if (type == "extra_deck")
					{
						return do_decode.template operator()<domain::ExtraDeckLocation>();
					}
					if (type == "hand")
					{
						return do_decode.template operator()<domain::HandLocation>();
					}
					if (type == "graveyard")
					{
						return do_decode.template operator()<domain::GraveyardLocation>();
					}
					if (type == "banished")
					{
						return do_decode.template operator()<domain::BanishedLocation>();
					}
					if (type == "monster")
					{
						return do_decode.template operator()<domain::MonsterLocation>();
					}
					if (type == "extra_monster")
					{
						return do_decode.template operator()<domain::ExtraMonsterLocation>();
					}
					if (type == "spell_trap")
					{
						return do_decode.template operator()<domain::SpellTrapLocation>();
					}
					if (type == "field_spell")
					{
						return do_decode.template operator()<domain::FieldSpellLocation>();
					}
					return std::nullopt;
				}
				else if constexpr (std::is_same_v<T, domain::FieldLocation>)
				{
					if (type == "monster")
					{
						return do_decode.template operator()<domain::MonsterLocation>();
					}
					if (type == "extra_monster")
					{
						return do_decode.template operator()<domain::ExtraMonsterLocation>();
					}
					if (type == "spell_trap")
					{
						return do_decode.template operator()<domain::SpellTrapLocation>();
					}
					if (type == "field_spell")
					{
						return do_decode.template operator()<domain::FieldSpellLocation>();
					}
					return std::nullopt;
				}
				else if constexpr (std::is_same_v<T, domain::AnyMonsterLocation>)
				{
					if (type == "monster")
					{
						return do_decode.template operator()<domain::MonsterLocation>();
					}
					if (type == "extra_monster")
					{
						return do_decode.template operator()<domain::ExtraMonsterLocation>();
					}
					return std::nullopt;
				}
				else
				{
					std::unreachable();
				}
			}

			// 没有type按照index匹配
			return ValueCodec<codec_detail::Wrap<T>>::decode(json);
		}

		[[nodiscard]] static auto encode(const T& any) noexcept -> nlohmann::json
		{
			auto object = ValueCodec<codec_detail::Wrap<T>>::encode(any);

			if constexpr (std::is_same_v<T, domain::Location>)
			{
				switch (any.index())
				{
					case utility::variant_index_v<domain::DeckLocation, T>:
					{
						object["type"] = "deck";
						break;
					}
					case utility::variant_index_v<domain::ExtraDeckLocation, T>:
					{
						object["type"] = "extra_deck";
						break;
					}
					case utility::variant_index_v<domain::HandLocation, T>:
					{
						object["type"] = "hand";
						break;
					}
					case utility::variant_index_v<domain::GraveyardLocation, T>:
					{
						object["type"] = "graveyard";
						break;
					}
					case utility::variant_index_v<domain::BanishedLocation, T>:
					{
						object["type"] = "banished";
						break;
					}
					case utility::variant_index_v<domain::MonsterLocation, T>:
					{
						object["type"] = "monster";
						break;
					}
					case utility::variant_index_v<domain::ExtraMonsterLocation, T>:
					{
						object["type"] = "extra_monster";
						break;
					}
					case utility::variant_index_v<domain::SpellTrapLocation, T>:
					{
						object["type"] = "spell_trap";
						break;
					}
					case utility::variant_index_v<domain::FieldSpellLocation, T>:
					{
						object["type"] = "field_spell";
						break;
					}
					default: {}
				}
			}
			else if constexpr (std::is_same_v<T, domain::FieldLocation>)
			{
				switch (any.index())
				{
					case utility::variant_index_v<domain::MonsterLocation, T>:
					{
						object["type"] = "monster";
						break;
					}
					case utility::variant_index_v<domain::ExtraMonsterLocation, T>:
					{
						object["type"] = "extra_monster";
						break;
					}
					case utility::variant_index_v<domain::SpellTrapLocation, T>:
					{
						object["type"] = "spell_trap";
						break;
					}
					case utility::variant_index_v<domain::FieldSpellLocation, T>:
					{
						object["type"] = "field_spell";
						break;
					}
					default: {}
				}
			}
			else if constexpr (std::is_same_v<T, domain::AnyMonsterLocation>)
			{
				switch (any.index())
				{
					case utility::variant_index_v<domain::MonsterLocation, T>:
					{
						object["type"] = "monster";
						break;
					}
					case utility::variant_index_v<domain::ExtraMonsterLocation, T>:
					{
						object["type"] = "extra_monster";
						break;
					}
					default: {}
				}
			}
			else
			{
				std::unreachable();
			}

			return object;
		}
	};

	template<typename T>
	// domain::DeckPlacement / domain::GraveyardPlacement 结构完全相同,一并处理
		requires(
			std::is_same_v<T, domain::DeckPlacement> ||
			std::is_same_v<T, domain::GraveyardPlacement>
		)
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

			const auto index = json.get<typename T::size_type>();
			return T{.index = index};
		}

		[[nodiscard]] static auto encode(const T& data) noexcept -> nlohmann::json
		{
			return data.index;
		}
	};

	template<typename T>
	// domain::ExtraDeckPlacement / domain::HandPlacement / BanishedPlacement 结构完全相同,一并处理
		requires(
			std::is_same_v<T, domain::ExtraDeckPlacement> ||
			std::is_same_v<T, domain::HandPlacement> ||
			std::is_same_v<T, domain::BanishedPlacement>
		)
	class ValueCodec<T>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// index
			if (!json.contains("index") || !json.at("index").is_number_unsigned())
			{
				return false;
			}

			// position
			// if (!json.contains("position") || !ValueCodec<domain::Position>::valid(json.at("position")))
			if (!json.contains("position"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<T>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto index_it = json.find("index");
			const auto index = index_it.value().get<typename T::size_type>();
			const auto position_it = json.find("position");
			const auto position = ValueCodec<domain::Position>::decode(position_it.value());
			if (!position.has_value())
			{
				return std::nullopt;
			}

			return T{.index = index, .position = *position};
		}

		[[nodiscard]] static auto encode(const T& data) noexcept -> nlohmann::json
		{
			auto json = nlohmann::json::object();

			json["index"] = data.index;
			json["position"] = ValueCodec<domain::Position>::encode(data.position);

			return json;
		}
	};

	template<typename T>
	// domain::MonsterPlacement / domain::ExtraMonsterPlacement / domain::SpellTrapPlacement 结构完全相同,一并处理
		requires(
			std::is_same_v<T, domain::MonsterPlacement> ||
			std::is_same_v<T, domain::ExtraMonsterPlacement> ||
			std::is_same_v<T, domain::SpellTrapPlacement>
		)
	class ValueCodec<T>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// column
			if (!json.contains("column") || !json.at("column").is_number_unsigned())
			{
				return false;
			}

			// position
			// if (!json.contains("position") || !ValueCodec<domain::Position>::valid(json.at("position")))
			if (!json.contains("position"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<T>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto column_it = json.find("column");
			const auto column = column_it.value().get<std::underlying_type_t<domain::FieldColumn>>();
			const auto position_it = json.find("position");
			const auto position = ValueCodec<domain::Position>::decode(position_it.value());
			if (!position.has_value())
			{
				return std::nullopt;
			}

			return T{.column = static_cast<domain::FieldColumn>(column), .position = *position};
		}

		[[nodiscard]] static auto encode(const T& data) noexcept -> nlohmann::json
		{
			auto json = nlohmann::json::object();

			json["column"] = std::to_underlying(data.column);
			json["position"] = ValueCodec<domain::Position>::encode(data.position);

			return json;
		}
	};

	template<typename T>
	// domain::Placement / domain::FieldPlacement / domain::AnyMonsterPlacement 处理方式完全相同,一并处理
		requires(
			std::is_same_v<T, domain::Placement> ||
			std::is_same_v<T, domain::FieldPlacement> ||
			std::is_same_v<T, domain::AnyMonsterPlacement>
		)
	class ValueCodec<T>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// type相比于variant_index更方便校验
			if (json.contains("type") && !json.at("type").is_string())
			{
				return false;
			}

			if (!ValueCodec<codec_detail::Wrap<T>>::valid(json))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<T>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			// 如果有type则按照type匹配
			if (const auto type_it = json.find("type");
				type_it != json.end())
			{
				const auto do_decode = [&]<typename L> noexcept -> std::optional<T>
				{
					if (const auto r = ValueCodec<L>::decode(json); r.has_value())
					{
						return T{*std::move(r)};
					}

					return std::nullopt;
				};

				const auto& type = type_it.value().get_ref<const nlohmann::json::string_t&>();

				if constexpr (std::is_same_v<T, domain::Placement>)
				{
					if (type == "deck")
					{
						return do_decode.template operator()<domain::DeckPlacement>();
					}
					if (type == "extra_deck")
					{
						return do_decode.template operator()<domain::ExtraDeckPlacement>();
					}
					if (type == "hand")
					{
						return do_decode.template operator()<domain::HandPlacement>();
					}
					if (type == "graveyard")
					{
						return do_decode.template operator()<domain::GraveyardPlacement>();
					}
					if (type == "banished")
					{
						return do_decode.template operator()<domain::BanishedPlacement>();
					}
					if (type == "monster")
					{
						return do_decode.template operator()<domain::MonsterPlacement>();
					}
					if (type == "extra_monster")
					{
						return do_decode.template operator()<domain::ExtraMonsterPlacement>();
					}
					if (type == "spell_trap")
					{
						return do_decode.template operator()<domain::SpellTrapPlacement>();
					}
					if (type == "field_spell")
					{
						return do_decode.template operator()<domain::FieldSpellPlacement>();
					}
					return std::nullopt;
				}
				else if constexpr (std::is_same_v<T, domain::FieldPlacement>)
				{
					if (type == "monster")
					{
						return do_decode.template operator()<domain::MonsterPlacement>();
					}
					if (type == "extra_monster")
					{
						return do_decode.template operator()<domain::ExtraMonsterPlacement>();
					}
					if (type == "spell_trap")
					{
						return do_decode.template operator()<domain::SpellTrapPlacement>();
					}
					if (type == "field_spell")
					{
						return do_decode.template operator()<domain::FieldSpellPlacement>();
					}
					return std::nullopt;
				}
				else if constexpr (std::is_same_v<T, domain::AnyMonsterPlacement>)
				{
					if (type == "monster")
					{
						return do_decode.template operator()<domain::MonsterPlacement>();
					}
					if (type == "extra_monster")
					{
						return do_decode.template operator()<domain::ExtraMonsterPlacement>();
					}
					return std::nullopt;
				}
				else
				{
					std::unreachable();
				}
			}

			// 没有type按照index匹配
			return ValueCodec<codec_detail::Wrap<T>>::decode(json);
		}

		[[nodiscard]] static auto encode(const T& any) noexcept -> nlohmann::json
		{
			auto object = ValueCodec<codec_detail::Wrap<T>>::encode(any);

			if constexpr (std::is_same_v<T, domain::Placement>)
			{
				switch (any.index())
				{
					case utility::variant_index_v<domain::DeckPlacement, T>:
					{
						object["type"] = "deck";
						break;
					}
					case utility::variant_index_v<domain::ExtraDeckPlacement, T>:
					{
						object["type"] = "extra_deck";
						break;
					}
					case utility::variant_index_v<domain::HandPlacement, T>:
					{
						object["type"] = "hand";
						break;
					}
					case utility::variant_index_v<domain::GraveyardPlacement, T>:
					{
						object["type"] = "graveyard";
						break;
					}
					case utility::variant_index_v<domain::BanishedPlacement, T>:
					{
						object["type"] = "banished";
						break;
					}
					case utility::variant_index_v<domain::MonsterPlacement, T>:
					{
						object["type"] = "monster";
						break;
					}
					case utility::variant_index_v<domain::ExtraMonsterPlacement, T>:
					{
						object["type"] = "extra_monster";
						break;
					}
					case utility::variant_index_v<domain::SpellTrapPlacement, T>:
					{
						object["type"] = "spell_trap";
						break;
					}
					case utility::variant_index_v<domain::FieldSpellPlacement, T>:
					{
						object["type"] = "field_spell";
						break;
					}
					default: {}
				}
			}
			else if constexpr (std::is_same_v<T, domain::FieldPlacement>)
			{
				switch (any.index())
				{
					case utility::variant_index_v<domain::MonsterPlacement, T>:
					{
						object["type"] = "monster";
						break;
					}
					case utility::variant_index_v<domain::ExtraMonsterPlacement, T>:
					{
						object["type"] = "extra_monster";
						break;
					}
					case utility::variant_index_v<domain::SpellTrapPlacement, T>:
					{
						object["type"] = "spell_trap";
						break;
					}
					case utility::variant_index_v<domain::FieldSpellPlacement, T>:
					{
						object["type"] = "field_spell";
						break;
					}
					default: {}
				}
			}
			else if constexpr (std::is_same_v<T, domain::AnyMonsterPlacement>)
			{
				switch (any.index())
				{
					case utility::variant_index_v<domain::MonsterPlacement, T>:
					{
						object["type"] = "monster";
						break;
					}
					case utility::variant_index_v<domain::ExtraMonsterPlacement, T>:
					{
						object["type"] = "extra_monster";
						break;
					}
					default: {}
				}
			}
			else
			{
				std::unreachable();
			}

			return object;
		}
	};

	// ==============================================================================================
	// STATE
	// ==============================================================================================

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

	// ==============================================================================================
	// ACTION
	// ==============================================================================================

	template<>
	class ValueCodec<driver::action::AdvancePhase>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_object();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::AdvancePhase>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			return driver::action::AdvancePhase{};
		}

		[[nodiscard]] static auto encode([[maybe_unused]] const driver::action::AdvancePhase& data) noexcept -> nlohmann::json
		{
			return nlohmann::json::object();
		}
	};

	template<>
	class ValueCodec<driver::action::NormalSummon>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// placement
			// if (!json.contains("placement") || !ValueCodec<domain::MonsterPlacement>::valid(json.at("placement")))
			if (!json.contains("placement"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::NormalSummon>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto placement_it = json.find("placement");
			const auto placement = ValueCodec<domain::MonsterPlacement>::decode(placement_it.value());
			if (!placement.has_value())
			{
				return std::nullopt;
			}

			return driver::action::NormalSummon{.card = *card, .placement = *placement};
		}

		[[nodiscard]] static auto encode(const driver::action::NormalSummon& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["placement"] = ValueCodec<domain::MonsterPlacement>::encode(data.placement);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::action::AdvanceSummon>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// materials
			// if (!json.contains("materials") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("materials")))
			if (!json.contains("materials"))
			{
				return false;
			}

			// placement
			// if (!json.contains("placement") || !ValueCodec<domain::MonsterPlacement>::valid(json.at("placement")))
			if (!json.contains("placement"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::AdvanceSummon>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto materials_it = json.find("materials");
			auto materials = ValueCodec<std::vector<state::CardInstanceId>>::decode(materials_it.value());
			if (!materials.has_value())
			{
				return std::nullopt;
			}
			const auto placement_it = json.find("placement");
			const auto placement = ValueCodec<domain::MonsterPlacement>::decode(placement_it.value());
			if (!placement.has_value())
			{
				return std::nullopt;
			}

			return driver::action::AdvanceSummon{.card = *card, .materials = *std::move(materials), .placement = *placement};
		}

		[[nodiscard]] static auto encode(const driver::action::AdvanceSummon& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["materials"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.materials);
			object["placement"] = ValueCodec<domain::MonsterPlacement>::encode(data.placement);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::action::FlipSummon>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::FlipSummon>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::action::FlipSummon{.card = *card};
		}

		[[nodiscard]] static auto encode(const driver::action::FlipSummon& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::action::DualSummon>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::DualSummon>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::action::DualSummon{.card = *card};
		}

		[[nodiscard]] static auto encode(const driver::action::DualSummon& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::action::ExtraDeckSummon>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// method
			// if (!json.contains("method") || !ValueCodec<std::vector<domain::SummonMethod>>::valid(json.at("method")))
			if (!json.contains("method"))
			{
				return false;
			}

			// materials
			// if (!json.contains("materials") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("materials")))
			if (!json.contains("materials") || !json.at("materials").is_array())
			{
				return false;
			}

			// placement
			// if (!json.contains("placement") || !ValueCodec<domain::AnyMonsterPlacement>::valid(json.at("placement")))
			if (!json.contains("placement"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::ExtraDeckSummon>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto method_it = json.find("method");
			const auto method = ValueCodec<domain::SummonMethod>::decode(method_it.value());
			if (!method.has_value())
			{
				return std::nullopt;
			}
			const auto materials_it = json.find("materials");
			auto materials = ValueCodec<std::vector<state::CardInstanceId>>::decode(materials_it.value());
			if (!materials.has_value())
			{
				return std::nullopt;
			}
			const auto placement_it = json.find("placement");
			const auto placement = ValueCodec<domain::AnyMonsterPlacement>::decode(placement_it.value());
			if (!placement.has_value())
			{
				return std::nullopt;
			}

			return driver::action::ExtraDeckSummon{.card = *card, .method = *method, .materials = *std::move(materials), .placement = *placement};
		}

		[[nodiscard]] static auto encode(const driver::action::ExtraDeckSummon& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["method"] = ValueCodec<domain::SummonMethod>::encode(data.method);
			object["materials"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.materials);
			object["placement"] = ValueCodec<domain::AnyMonsterPlacement>::encode(data.placement);

			return object;
		}
	};


	template<>
	class ValueCodec<driver::action::PendulumSummon>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// card
			// if (!json.contains("cards") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("cards")))
			if (!json.contains("cards") || !json.at("cards").is_array())
			{
				return false;
			}

			// placements
			// if (!json.contains("placements") || !ValueCodec<std::vector<domain::MonsterPlacement>>::valid(json.at("placements")))
			if (!json.contains("placements") || !json.at("placements").is_array())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::PendulumSummon>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto cards_it = json.find("cards");
			auto cards = ValueCodec<std::vector<state::CardInstanceId>>::decode(cards_it.value());
			if (!cards.has_value())
			{
				return std::nullopt;
			}
			const auto placements_it = json.find("placements");
			auto placements = ValueCodec<std::vector<domain::MonsterPlacement>>::decode(placements_it.value());
			if (!placements.has_value())
			{
				return std::nullopt;
			}

			return driver::action::PendulumSummon{.cards = *std::move(cards), .placements = *std::move(placements)};
		}

		[[nodiscard]] static auto encode(const driver::action::PendulumSummon& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["cards"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.cards);
			object["placements"] = ValueCodec<std::vector<domain::MonsterPlacement>>::encode(data.placements);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::action::SpecialSummon>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// placement
			// if (!json.contains("placement") || !ValueCodec<domain::MonsterPlacement>::valid(json.at("placement")))
			if (!json.contains("placement"))
			{
				return false;
			}

			// controller
			// if (!json.contains("controller") || !ValueCodec<domain::Player>::valid(json.at("controller")))
			if (!json.contains("controller"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::SpecialSummon>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto placement_it = json.find("placement");
			const auto placement = ValueCodec<domain::MonsterPlacement>::decode(placement_it.value());
			if (!placement.has_value())
			{
				return std::nullopt;
			}
			const auto controller_it = json.find("controller");
			const auto controller = ValueCodec<domain::Player>::decode(controller_it.value());
			if (!controller.has_value())
			{
				return std::nullopt;
			}

			return driver::action::SpecialSummon{.card = *card, .placement = *placement, .controller = *controller};
		}

		[[nodiscard]] static auto encode(const driver::action::SpecialSummon& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["placement"] = ValueCodec<domain::MonsterPlacement>::encode(data.placement);
			object["controller"] = ValueCodec<domain::Player>::encode(data.controller);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::action::ChangePosition>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// position
			// if (!json.contains("position") || !ValueCodec<domain::Position>::valid(json.at("position")))
			if (!json.contains("position"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::ChangePosition>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto position_it = json.find("position");
			const auto position = ValueCodec<domain::Position>::decode(position_it.value());
			if (!position.has_value())
			{
				return std::nullopt;
			}

			return driver::action::ChangePosition{.card = *card, .position = *position};
		}

		[[nodiscard]] static auto encode(const driver::action::ChangePosition& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["position"] = ValueCodec<domain::Position>::encode(data.position);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::action::AttackDeclare>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// attacker
			// if (!json.contains("attacker") || !ValueCodec<state::CardInstanceId>::valid(json.at("attacker")))
			if (!json.contains("attacker"))
			{
				return false;
			}

			// target
			// if (json.contains("target") && !ValueCodec<state::CardInstanceId>::valid(json.at("target")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::AttackDeclare>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto attacker_it = json.find("attacker");
			const auto attacker = ValueCodec<state::CardInstanceId>::decode(attacker_it.value());
			if (!attacker.has_value())
			{
				return std::nullopt;
			}
			const auto target_it = json.find("target");
			if (target_it == json.end())
			{
				return driver::action::AttackDeclare{.attacker = *attacker, .target = std::nullopt};
			}
			const auto target = ValueCodec<state::CardInstanceId>::decode(target_it.value());
			if (!target.has_value())
			{
				return std::nullopt;
			}

			return driver::action::AttackDeclare{.attacker = *attacker, .target = *target};
		}

		[[nodiscard]] static auto encode(const driver::action::AttackDeclare& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["attacker"] = ValueCodec<state::CardInstanceId>::encode(data.attacker);
			if (data.target.has_value())
			{
				object["target"] = ValueCodec<state::CardInstanceId>::encode(*data.target);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::action::ActiveEffect>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// effect
			// if (!json.contains("effect") || !ValueCodec<state::EffectId>::valid(json.at("effect")))
			if (!json.contains("effect"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::ActiveEffect>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto effect_it = json.find("effect");
			const auto effect = ValueCodec<state::EffectId>::decode(effect_it.value());
			if (!effect.has_value())
			{
				return std::nullopt;
			}

			return driver::action::ActiveEffect{.card = *card, .effect = *effect};
		}

		[[nodiscard]] static auto encode(const driver::action::ActiveEffect& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["effect"] = ValueCodec<state::EffectId>::encode(data.effect);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::action::SetSpellTrap>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// location
			// if (!json.contains("location") || !ValueCodec<domain::SpellTrapLocation>::valid(json.at("location")))
			if (!json.contains("location"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::SetSpellTrap>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto location_it = json.find("location");
			const auto location = ValueCodec<domain::SpellTrapLocation>::decode(location_it.value());
			if (!location.has_value())
			{
				return std::nullopt;
			}

			return driver::action::SetSpellTrap{.card = *card, .location = *location};
		}

		[[nodiscard]] static auto encode(const driver::action::SetSpellTrap& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["location"] = ValueCodec<domain::SpellTrapLocation>::encode(data.location);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::action::ActivatePendulum>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// side
			// if (!json.contains("side") || !ValueCodec<domain::PendulumSide>::valid(json.at("side")))
			if (!json.contains("side"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::ActivatePendulum>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto side_it = json.find("side");
			const auto side = ValueCodec<domain::PendulumSide>::decode(side_it.value());
			if (!side.has_value())
			{
				return std::nullopt;
			}

			return driver::action::ActivatePendulum{.card = *card, .side = *side};
		}

		[[nodiscard]] static auto encode(const driver::action::ActivatePendulum& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["side"] = ValueCodec<domain::PendulumSide>::encode(data.side);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::action::Action>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// type相比于variant_index更方便校验
			if (json.contains("type") && !json.at("type").is_string())
			{
				return false;
			}

			if (!ValueCodec<codec_detail::Wrap<driver::action::Action>>::valid(json))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::Action>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			// 如果有type则按照type匹配
			if (const auto type_it = json.find("type");
				type_it != json.end())
			{
				const auto do_decode = [&]<typename L> noexcept -> std::optional<driver::action::Action>
				{
					if (const auto r = ValueCodec<L>::decode(json);
						r.has_value())
					{
						return driver::action::Action{*std::move(r)};
					}

					return std::nullopt;
				};

				const auto& type = type_it.value().get_ref<const nlohmann::json::string_t&>();

				if (type == "advance_phase")
				{
					return do_decode.operator()<driver::action::AdvancePhase>();
				}
				if (type == "normal_summon")
				{
					return do_decode.operator()<driver::action::NormalSummon>();
				}
				if (type == "advance_summon")
				{
					return do_decode.operator()<driver::action::AdvanceSummon>();
				}
				if (type == "flip_summon")
				{
					return do_decode.operator()<driver::action::FlipSummon>();
				}
				if (type == "dual_summon")
				{
					return do_decode.operator()<driver::action::DualSummon>();
				}
				if (type == "extra_deck_summon")
				{
					return do_decode.operator()<driver::action::ExtraDeckSummon>();
				}
				if (type == "pendulum_summon")
				{
					return do_decode.operator()<driver::action::PendulumSummon>();
				}
				if (type == "special_summon")
				{
					return do_decode.operator()<driver::action::SpecialSummon>();
				}
				if (type == "change_position")
				{
					return do_decode.operator()<driver::action::ChangePosition>();
				}
				if (type == "attack_declare")
				{
					return do_decode.operator()<driver::action::AttackDeclare>();
				}
				if (type == "active_effect")
				{
					return do_decode.operator()<driver::action::ActiveEffect>();
				}
				if (type == "set_spell_trap")
				{
					return do_decode.operator()<driver::action::SetSpellTrap>();
				}
				if (type == "activate_pendulum")
				{
					return do_decode.operator()<driver::action::ActivatePendulum>();
				}
			}

			// 没有type按照index匹配
			return ValueCodec<codec_detail::Wrap<driver::action::Action>>::decode(json);
		}

		[[nodiscard]] static auto encode(const driver::action::Action& data) noexcept -> nlohmann::json
		{
			auto object = ValueCodec<codec_detail::Wrap<driver::action::Action>>::encode(data);

			switch (data.index())
			{
				case utility::variant_index_v<driver::action::AdvancePhase, driver::action::Action>:
				{
					object["type"] = "advance_phase";
					break;
				}
				case utility::variant_index_v<driver::action::NormalSummon, driver::action::Action>:
				{
					object["type"] = "normal_summon";
					break;
				}
				case utility::variant_index_v<driver::action::AdvanceSummon, driver::action::Action>:
				{
					object["type"] = "advance_summon";
					break;
				}
				case utility::variant_index_v<driver::action::FlipSummon, driver::action::Action>:
				{
					object["type"] = "flip_summon";
					break;
				}
				case utility::variant_index_v<driver::action::DualSummon, driver::action::Action>:
				{
					object["type"] = "dual_summon";
					break;
				}
				case utility::variant_index_v<driver::action::ExtraDeckSummon, driver::action::Action>:
				{
					object["type"] = "extra_deck_summon";
					break;
				}
				case utility::variant_index_v<driver::action::PendulumSummon, driver::action::Action>:
				{
					object["type"] = "pendulum_summon";
					break;
				}
				case utility::variant_index_v<driver::action::SpecialSummon, driver::action::Action>:
				{
					object["type"] = "special_summon";
					break;
				}
				case utility::variant_index_v<driver::action::ChangePosition, driver::action::Action>:
				{
					object["type"] = "change_position";
					break;
				}
				case utility::variant_index_v<driver::action::AttackDeclare, driver::action::Action>:
				{
					object["type"] = "attack_declare";
					break;
				}
				case utility::variant_index_v<driver::action::ActiveEffect, driver::action::Action>:
				{
					object["type"] = "active_effect";
					break;
				}
				case utility::variant_index_v<driver::action::SetSpellTrap, driver::action::Action>:
				{
					object["type"] = "set_spell_trap";
					break;
				}
				case utility::variant_index_v<driver::action::ActivatePendulum, driver::action::Action>:
				{
					object["type"] = "activate_pendulum";
					break;
				}
				default: {}
			}

			return object;
		}
	};

	// ==============================================================================================
	// REQUEST
	// ==============================================================================================

	template<>
	class ValueCodec<driver::request::QueryIdleCommand>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryIdleCommand>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryIdleCommand{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryIdleCommand& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SubmitAction>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// action
			// if (!json.contains("action") || !ValueCodec<driver::action::Action>::valid(json))
			if (!json.contains("action"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SubmitAction>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto action_it = json.find("action");
			auto action = ValueCodec<driver::action::Action>::decode(action_it.value());
			if (!action.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SubmitAction{.player = *player, .action = *std::move(action)};
		}

		[[nodiscard]] static auto encode(const driver::request::SubmitAction& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["action"] = ValueCodec<driver::action::Action>::encode(data.action);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			// min
			if (!json.contains("min") || !json.at("min").is_number_unsigned())
			{
				return false;
			}

			// max
			if (!json.contains("max") || !json.at("max").is_number_unsigned())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}
			const auto min_it = json.find("min");
			const auto min = min_it.value().get<domain::DeckLocation::size_type>();
			const auto max_it = json.find("max");
			const auto max = max_it.value().get<domain::DeckLocation::size_type>();


			return driver::request::SelectCards{.player = *player, .candidates = *std::move(candidates), .min = min, .max = max};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);
			object["min"] = data.max;
			object["max"] = data.max;

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectYesNo>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectYesNo>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectYesNo{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectYesNo& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectNumbers>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// candidates
			if (!json.contains("candidates") || !json.at("candidates").is_array())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectNumbers>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto candidates_it = json.find("candidates");
			std::vector<std::int64_t> candidates;
			try
			{
				candidates = candidates_it.value().get<std::vector<std::int64_t>>();
			}
			catch ([[maybe_unused]] const nlohmann::json::parse_error& error)
			{
				return std::nullopt;
			}

			return driver::request::SelectNumbers{.player = *player, .candidates = std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectNumbers& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["candidates"] = data.candidates;

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::OrderCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// candidates
			if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::OrderCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::request::OrderCards{.player = *player, .candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::request::OrderCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectMonsterZoneLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// location
			// if (!json.contains("location") || !ValueCodec<domain::MonsterLocation>::valid(json.at("location")))
			if (!json.contains("location"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectMonsterZoneLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto location_it = json.find("location");
			const auto location = ValueCodec<domain::MonsterLocation>::decode(location_it.value());
			if (!location.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectMonsterZoneLocation{.player = *player, .card = *card, .location = *location};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectMonsterZoneLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["location"] = ValueCodec<domain::MonsterLocation>::encode(data.location);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectAnyMonsterZoneLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// location
			// if (!json.contains("location") || !ValueCodec<domain::AnyMonsterLocation>::valid(json.at("location")))
			if (!json.contains("location"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectAnyMonsterZoneLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto location_it = json.find("location");
			const auto location = ValueCodec<domain::AnyMonsterLocation>::decode(location_it.value());
			if (!location.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectAnyMonsterZoneLocation{.player = *player, .card = *card, .location = *location};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectAnyMonsterZoneLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["location"] = ValueCodec<domain::AnyMonsterLocation>::encode(data.location);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectMonsterPosition>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// position
			// if (!json.contains("position") || !ValueCodec<domain::Position>::valid(json.at("position")))
			if (!json.contains("position"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectMonsterPosition>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto position_it = json.find("position");
			const auto position = ValueCodec<domain::Position>::decode(position_it.value());
			if (!position.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectMonsterPosition{.player = *player, .card = *card, .position = *position};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectMonsterPosition& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["position"] = ValueCodec<domain::Position>::encode(data.position);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectSpellTrapLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// location
			// if (!json.contains("location") || !ValueCodec<domain::SpellTrapLocation>::valid(json.at("location")))
			if (!json.contains("location"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectSpellTrapLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto location_it = json.find("location");
			const auto location = ValueCodec<domain::SpellTrapLocation>::decode(location_it.value());
			if (!location.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectSpellTrapLocation{.player = *player, .card = *card, .location = *location};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectSpellTrapLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["location"] = ValueCodec<domain::SpellTrapLocation>::encode(data.location);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryPositionChangeableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryPositionChangeableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryPositionChangeableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryPositionChangeableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};


	template<>
	class ValueCodec<driver::request::QueryPositionChangeablePositions>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryPositionChangeablePositions>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryPositionChangeablePositions{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryPositionChangeablePositions& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryNormalSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryNormalSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryNormalSummonableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryNormalSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryNormalSummonType>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryNormalSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryNormalSummonType{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryNormalSummonType& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::CheckNormalSummonType>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// type
			// if (!json.contains("type") || !ValueCodec<domain::NormalSummonType>::valid(json.at("type")))
			if (!json.contains("type"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::CheckNormalSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto type_it = json.find("type");
			const auto type = ValueCodec<domain::NormalSummonType>::decode(type_it.value());
			if (!type.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::CheckNormalSummonType{.player = *player, .type = *type, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::CheckNormalSummonType& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["type"] = ValueCodec<domain::NormalSummonType>::encode(data.type);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryNormalSummonLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryNormalSummonLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryNormalSummonLocation{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryNormalSummonLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryNormalSummonPlacement>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryNormalSummonPlacement>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryNormalSummonPlacement{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryNormalSummonPlacement& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryAdvanceSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryAdvanceSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			return driver::request::QueryAdvanceSummonableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryAdvanceSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryAdvanceCandidates>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryAdvanceCandidates>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryAdvanceCandidates{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryAdvanceCandidates& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryAdvanceSummonType>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryAdvanceSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryAdvanceSummonType{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryAdvanceSummonType& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::CheckAdvanceSummonType>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// type
			// if (!json.contains("type") || !ValueCodec<domain::NormalSummonType>::valid(json.at("type")))
			if (!json.contains("type"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::CheckAdvanceSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto type_it = json.find("type");
			const auto type = ValueCodec<domain::NormalSummonType>::decode(type_it.value());
			if (!type.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::CheckAdvanceSummonType{.player = *player, .type = *type, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::CheckAdvanceSummonType& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["type"] = ValueCodec<domain::NormalSummonType>::encode(data.type);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectAdvanceTributes>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}


			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// tributes
			// if (!json.contains("tributes") || !ValueCodec<std::vector<state::CardInstanceId>>::valid("tributes"))
			if (!json.contains("tributes") || !json.at("tributes").is_array())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectAdvanceTributes>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto tributes_it = json.find("tributes");
			auto tributes = ValueCodec<std::vector<state::CardInstanceId>>::decode(tributes_it.value());
			if (!tributes.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectAdvanceTributes{.player = *player, .card = *card, .tributes = *std::move(tributes)};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectAdvanceTributes& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["tributes"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.tributes);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryAdvanceSummonLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryAdvanceSummonLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryAdvanceSummonLocation{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryAdvanceSummonLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryFlipSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryFlipSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryFlipSummonableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryFlipSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryDualSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryDualSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryDualSummonableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryDualSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryExtraDeckSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// method
			// if (!json.contains("method") || !ValueCodec<domain::SummonMethod>::valid(json.at("method")))
			if (!json.contains("method"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryExtraDeckSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto method_it = json.find("method");
			const auto method = ValueCodec<domain::SummonMethod>::decode(method_it.value());
			if (!method.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryExtraDeckSummonableCards{.player = *player, .method = *method};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryExtraDeckSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["method"] = ValueCodec<domain::SummonMethod>::encode(data.method);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryExtraDeckSummonMaterialSources>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// method
			// if (!json.contains("method") || !ValueCodec<domain::SummonMethod>::valid(json.at("method")))
			if (!json.contains("method"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryExtraDeckSummonMaterialSources>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto method_it = json.find("method");
			const auto method = ValueCodec<domain::SummonMethod>::decode(method_it.value());
			if (!method.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryExtraDeckSummonMaterialSources{.player = *player, .card = *card, .method = *method};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryExtraDeckSummonMaterialSources& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["method"] = ValueCodec<domain::SummonMethod>::encode(data.method);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryExtraDeckSummonMaterials>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// method
			// if (!json.contains("method") || !ValueCodec<domain::SummonMethod>::valid(json.at("method")))
			if (!json.contains("method"))
			{
				return false;
			}

			// material_source
			// if (!json.contains("material_source") || !ValueCodec<utility::EnumMask<domain::SummonMaterialSource>>::valid(json.at("material_source")))
			if (!json.contains("material_source"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryExtraDeckSummonMaterials>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto method_it = json.find("method");
			const auto method = ValueCodec<domain::SummonMethod>::decode(method_it.value());
			if (!method.has_value())
			{
				return std::nullopt;
			}
			const auto material_source_it = json.find("material_source");
			const auto material_source = ValueCodec<utility::EnumMask<domain::SummonMaterialSource>>::decode(material_source_it.value());
			if (!material_source.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryExtraDeckSummonMaterials{.player = *player, .card = *card, .method = *method, .material_source = *material_source};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryExtraDeckSummonMaterials& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["method"] = ValueCodec<domain::SummonMethod>::encode(data.method);
			object["material_source"] = ValueCodec<utility::EnumMask<domain::SummonMaterialSource>>::encode(data.material_source);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryExtraDeckSummonLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// method
			// if (!json.contains("method") || !ValueCodec<domain::SummonMethod>::valid(json.at("method")))
			if (!json.contains("method"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryExtraDeckSummonLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto method_it = json.find("method");
			const auto method = ValueCodec<domain::SummonMethod>::decode(method_it.value());
			if (!method.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryExtraDeckSummonLocation{.player = *player, .card = *card, .method = *method};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryExtraDeckSummonLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["method"] = ValueCodec<domain::SummonMethod>::encode(data.method);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryPendulumSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryPendulumSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryPendulumSummonableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryPendulumSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QuerySpecialSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QuerySpecialSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QuerySpecialSummonableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QuerySpecialSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QuerySpecialSummonLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QuerySpecialSummonLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QuerySpecialSummonLocation{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QuerySpecialSummonLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QuerySpecialSummonPlacements>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QuerySpecialSummonPlacements>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QuerySpecialSummonPlacements{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QuerySpecialSummonPlacements& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QuerySetSpellTrapCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QuerySetSpellTrapCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QuerySetSpellTrapCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QuerySetSpellTrapCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QuerySpellTrapSetLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QuerySpellTrapSetLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QuerySpellTrapSetLocation{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QuerySpellTrapSetLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryActivatePendulumCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryActivatePendulumCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryActivatePendulumCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryActivatePendulumCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryActivatePendulumSide>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryActivatePendulumSide>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryActivatePendulumSide{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryActivatePendulumSide& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryActivatableEffects>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (json.contains("card") && !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryActivatableEffects>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			if (card_it == json.end())
			{
				return driver::request::QueryActivatableEffects{.player = *player, .card = std::nullopt};
			}
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryActivatableEffects{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryActivatableEffects& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			if (data.card.has_value())
			{
				object["card"] = ValueCodec<state::CardInstanceId>::encode(*data.card);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryEffectTargets>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// effect
			// if (!json.contains("effect") || !ValueCodec<state::EffectId>::valid(json.at("effect")))
			if (!json.contains("effect"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryEffectTargets>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto effect_it = json.find("effect");
			const auto effect = ValueCodec<state::EffectId>::decode(effect_it.value());
			if (!effect.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryEffectTargets{.player = *player, .card = *card, .effect = *effect};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryEffectTargets& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["effect"] = ValueCodec<state::EffectId>::encode(data.effect);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectEffectActivation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// effect
			// if (!json.contains("effect") || !ValueCodec<state::EffectId>::valid(json.at("effect")))
			if (!json.contains("effect"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectEffectActivation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto effect_it = json.find("effect");
			const auto effect = ValueCodec<state::EffectId>::decode(effect_it.value());
			if (!effect.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectEffectActivation{.player = *player, .card = *card, .effect = *effect};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectEffectActivation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["effect"] = ValueCodec<state::EffectId>::encode(data.effect);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectEffectTargets>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// effect
			// if (!json.contains("effect") || !ValueCodec<state::EffectId>::valid(json.at("effect")))
			if (!json.contains("effect"))
			{
				return false;
			}

			// targets
			// if (!json.contains("targets") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("targets")))
			if (!json.contains("targets"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectEffectTargets>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto effect_it = json.find("effect");
			const auto effect = ValueCodec<state::EffectId>::decode(effect_it.value());
			if (!effect.has_value())
			{
				return std::nullopt;
			}
			const auto targets_it = json.find("targets");
			auto targets = ValueCodec<std::vector<state::CardInstanceId>>::decode(targets_it.value());
			if (!targets.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectEffectTargets{.player = *player, .card = *card, .effect = *effect, .targets = *std::move(targets)};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectEffectTargets& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["effect"] = ValueCodec<state::EffectId>::encode(data.effect);
			object["targets"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.targets);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryChainableEffects>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryChainableEffects>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryChainableEffects{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryChainableEffects& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectChain>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// effect
			// if (!json.contains("effect") || !ValueCodec<state::EffectId>::valid(json.at("effect")))
			if (!json.contains("effect"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectChain>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto effect_it = json.find("effect");
			const auto effect = ValueCodec<state::EffectId>::decode(effect_it.value());
			if (!effect.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectChain{.player = *player, .card = *card, .effect = *effect};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectChain& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["effect"] = ValueCodec<state::EffectId>::encode(data.effect);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryAttackableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryAttackableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryAttackableCards{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryAttackableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::CheckDirectAttackable>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::CheckDirectAttackable>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::CheckDirectAttackable{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::CheckDirectAttackable& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryAttackTargets>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryAttackTargets>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryAttackTargets{.player = *player, .card = *card};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryAttackTargets& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectAttack>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// card
			// if (!json.contains("card") || !ValueCodec<state::CardInstanceId>::valid(json.at("card")))
			if (!json.contains("card"))
			{
				return false;
			}

			// target
			// if (json.contains("target") && !ValueCodec<state::CardInstanceId>::valid(json.at("target")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectAttack>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto card_it = json.find("card");
			const auto card = ValueCodec<state::CardInstanceId>::decode(card_it.value());
			if (!card.has_value())
			{
				return std::nullopt;
			}
			const auto target_it = json.find("target");
			if (target_it == json.end())
			{
				return driver::request::SelectAttack{.player = *player, .card = *card, .target = std::nullopt};
			}
			const auto target = ValueCodec<state::CardInstanceId>::decode(target_it.value());
			if (!target.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectAttack{.player = *player, .card = *card, .target = *target};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectAttack& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			if (data.target.has_value())
			{
				object["target"] = ValueCodec<state::CardInstanceId>::encode(*data.target);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::QueryPhaseTransitions>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::QueryPhaseTransitions>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::QueryPhaseTransitions{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::QueryPhaseTransitions& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::SelectPhaseTransition>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// to
			// if (!json.contains("to") || !ValueCodec<domain::Phase>::valid(json.at("to")))
			if (!json.contains("to"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::SelectPhaseTransition>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto to_it = json.find("to");
			const auto to = ValueCodec<domain::Phase>::decode(to_it.value());
			if (!to.has_value())
			{
				return std::nullopt;
			}

			return driver::request::SelectPhaseTransition{.player = *player, .to = *to};
		}

		[[nodiscard]] static auto encode(const driver::request::SelectPhaseTransition& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["to"] = ValueCodec<domain::Phase>::encode(data.to);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::CancelSelection>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// context
			// if (!json.contains("context") || !ValueCodec<domain::SelectionContext>::valid(json.at("context")))
			if (!json.contains("context"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::CancelSelection>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto context_it = json.find("context");
			const auto context = ValueCodec<domain::SelectionContext>::decode(context_it.value());
			if (!context.has_value())
			{
				return std::nullopt;
			}

			return driver::request::CancelSelection{.player = *player, .context = *context};
		}

		[[nodiscard]] static auto encode(const driver::request::CancelSelection& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["context"] = ValueCodec<domain::SelectionContext>::encode(data.context);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::ConfirmSelection>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// context
			// if (!json.contains("context") || !ValueCodec<domain::SelectionContext>::valid(json.at("context")))
			if (!json.contains("context"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::ConfirmSelection>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto context_it = json.find("context");
			const auto context = ValueCodec<domain::SelectionContext>::decode(context_it.value());
			if (!context.has_value())
			{
				return std::nullopt;
			}

			return driver::request::ConfirmSelection{.player = *player, .context = *context};
		}

		[[nodiscard]] static auto encode(const driver::request::ConfirmSelection& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["context"] = ValueCodec<domain::SelectionContext>::encode(data.context);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::PassResponse>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::PassResponse>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}

			return driver::request::PassResponse{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::request::PassResponse& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::request::Request>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// type相比于variant_index更方便校验
			if (json.contains("type") && !json.at("type").is_string())
			{
				return false;
			}

			if (!ValueCodec<codec_detail::Wrap<driver::request::Request>>::valid(json))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::request::Request>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			// 如果有type则按照type匹配
			if (const auto type_it = json.find("type");
				type_it != json.end())
			{
				const auto do_decode = [&]<typename L> noexcept -> std::optional<driver::request::Request>
				{
					if (const auto r = ValueCodec<L>::decode(json);
						r.has_value())
					{
						return driver::request::Request{*std::move(r)};
					}

					return std::nullopt;
				};

				const auto& type = type_it.value().get_ref<const nlohmann::json::string_t&>();

				if (type == "query_idle_command")
				{
					return do_decode.operator()<driver::request::QueryIdleCommand>();
				}
				if (type == "submit_action")
				{
					return do_decode.operator()<driver::request::SubmitAction>();
				}
				if (type == "select_cards")
				{
					return do_decode.operator()<driver::request::SelectCards>();
				}
				if (type == "select_yes_no")
				{
					return do_decode.operator()<driver::request::SelectYesNo>();
				}
				if (type == "select_numbers")
				{
					return do_decode.operator()<driver::request::SelectNumbers>();
				}
				if (type == "order_cards")
				{
					return do_decode.operator()<driver::request::OrderCards>();
				}
				if (type == "select_monster_zone_location")
				{
					return do_decode.operator()<driver::request::SelectMonsterZoneLocation>();
				}
				if (type == "select_any_monster_zone_location")
				{
					return do_decode.operator()<driver::request::SelectAnyMonsterZoneLocation>();
				}
				if (type == "select_monster_position")
				{
					return do_decode.operator()<driver::request::SelectMonsterPosition>();
				}
				if (type == "select_spell_trap_location")
				{
					return do_decode.operator()<driver::request::SelectSpellTrapLocation>();
				}
				if (type == "query_position_changeable_cards")
				{
					return do_decode.operator()<driver::request::QueryPositionChangeableCards>();
				}
				if (type == "query_position_changeable_positions")
				{
					return do_decode.operator()<driver::request::QueryPositionChangeablePositions>();
				}
				if (type == "query_normal_summonable_cards")
				{
					return do_decode.operator()<driver::request::QueryNormalSummonableCards>();
				}
				if (type == "query_normal_summon_type")
				{
					return do_decode.operator()<driver::request::QueryNormalSummonType>();
				}
				if (type == "check_normal_summon_type")
				{
					return do_decode.operator()<driver::request::CheckNormalSummonType>();
				}
				if (type == "query_normal_summon_location")
				{
					return do_decode.operator()<driver::request::QueryNormalSummonLocation>();
				}
				if (type == "query_normal_summon_placement")
				{
					return do_decode.operator()<driver::request::QueryNormalSummonPlacement>();
				}
				if (type == "query_advance_summonable_cards")
				{
					return do_decode.operator()<driver::request::QueryAdvanceSummonableCards>();
				}
				if (type == "query_advance_candidates")
				{
					return do_decode.operator()<driver::request::QueryAdvanceCandidates>();
				}
				if (type == "query_advance_summon_type")
				{
					return do_decode.operator()<driver::request::QueryAdvanceSummonType>();
				}
				if (type == "check_advance_summon_type")
				{
					return do_decode.operator()<driver::request::CheckAdvanceSummonType>();
				}
				if (type == "select_advance_tributes")
				{
					return do_decode.operator()<driver::request::SelectAdvanceTributes>();
				}
				if (type == "query_advance_summon_location")
				{
					return do_decode.operator()<driver::request::QueryAdvanceSummonLocation>();
				}
				if (type == "query_flip_summonable_cards")
				{
					return do_decode.operator()<driver::request::QueryFlipSummonableCards>();
				}
				if (type == "query_dual_summonable_cards")
				{
					return do_decode.operator()<driver::request::QueryDualSummonableCards>();
				}
				if (type == "query_extra_deck_summonable_cards")
				{
					return do_decode.operator()<driver::request::QueryExtraDeckSummonableCards>();
				}
				if (type == "query_extra_deck_summon_material_sources")
				{
					return do_decode.operator()<driver::request::QueryExtraDeckSummonMaterialSources>();
				}
				if (type == "query_extra_deck_summon_materials")
				{
					return do_decode.operator()<driver::request::QueryExtraDeckSummonMaterials>();
				}
				if (type == "query_extra_deck_summon_location")
				{
					return do_decode.operator()<driver::request::QueryExtraDeckSummonLocation>();
				}
				if (type == "query_pendulum_summonable_cards")
				{
					return do_decode.operator()<driver::request::QueryPendulumSummonableCards>();
				}
				if (type == "query_special_summonable_cards")
				{
					return do_decode.operator()<driver::request::QuerySpecialSummonableCards>();
				}
				if (type == "query_special_summon_location")
				{
					return do_decode.operator()<driver::request::QuerySpecialSummonLocation>();
				}
				if (type == "query_special_summon_placements")
				{
					return do_decode.operator()<driver::request::QuerySpecialSummonPlacements>();
				}
				if (type == "query_set_spell_trap_cards")
				{
					return do_decode.operator()<driver::request::QuerySetSpellTrapCards>();
				}
				if (type == "query_spell_trap_set_location")
				{
					return do_decode.operator()<driver::request::QuerySpellTrapSetLocation>();
				}
				if (type == "query_activate_pendulum_cards")
				{
					return do_decode.operator()<driver::request::QueryActivatePendulumCards>();
				}
				if (type == "query_activate_pendulum_side")
				{
					return do_decode.operator()<driver::request::QueryActivatePendulumSide>();
				}
				if (type == "query_activatable_effects")
				{
					return do_decode.operator()<driver::request::QueryActivatableEffects>();
				}
				if (type == "query_effect_targets")
				{
					return do_decode.operator()<driver::request::QueryEffectTargets>();
				}
				if (type == "select_effect_activation")
				{
					return do_decode.operator()<driver::request::SelectEffectActivation>();
				}
				if (type == "select_effect_targets")
				{
					return do_decode.operator()<driver::request::SelectEffectTargets>();
				}
				if (type == "query_chainable_effects")
				{
					return do_decode.operator()<driver::request::QueryChainableEffects>();
				}
				if (type == "select_chain")
				{
					return do_decode.operator()<driver::request::SelectChain>();
				}
				if (type == "query_attackable_cards")
				{
					return do_decode.operator()<driver::request::QueryAttackableCards>();
				}
				if (type == "check_direct_attackable")
				{
					return do_decode.operator()<driver::request::CheckDirectAttackable>();
				}
				if (type == "query_attack_targets")
				{
					return do_decode.operator()<driver::request::QueryAttackTargets>();
				}
				if (type == "select_attack")
				{
					return do_decode.operator()<driver::request::SelectAttack>();
				}
				if (type == "query_phase_transitions")
				{
					return do_decode.operator()<driver::request::QueryPhaseTransitions>();
				}
				if (type == "select_phase_transition")
				{
					return do_decode.operator()<driver::request::SelectPhaseTransition>();
				}
				if (type == "cancel_selection")
				{
					return do_decode.operator()<driver::request::CancelSelection>();
				}
				if (type == "confirm_selection")
				{
					return do_decode.operator()<driver::request::ConfirmSelection>();
				}
				if (type == "pass_response")
				{
					return do_decode.operator()<driver::request::PassResponse>();
				}

				// 未知type直接失败
				return std::nullopt;
			}

			// 没有type按照index匹配
			return ValueCodec<codec_detail::Wrap<driver::request::Request>>::decode(json);
		}

		[[nodiscard]] static auto encode(const driver::request::Request& data) noexcept -> nlohmann::json
		{
			auto object = ValueCodec<codec_detail::Wrap<driver::request::Request>>::encode(data);

			switch (data.index())
			{
				case utility::variant_index_v<driver::request::QueryIdleCommand, driver::request::Request>:
				{
					object["type"] = "query_idle_command";
					break;
				}
				case utility::variant_index_v<driver::request::SubmitAction, driver::request::Request>:
				{
					object["type"] = "submit_action";
					break;
				}
				case utility::variant_index_v<driver::request::SelectCards, driver::request::Request>:
				{
					object["type"] = "select_cards";
					break;
				}
				case utility::variant_index_v<driver::request::SelectYesNo, driver::request::Request>:
				{
					object["type"] = "select_yes_no";
					break;
				}
				case utility::variant_index_v<driver::request::SelectNumbers, driver::request::Request>:
				{
					object["type"] = "select_numbers";
					break;
				}
				case utility::variant_index_v<driver::request::OrderCards, driver::request::Request>:
				{
					object["type"] = "order_cards";
					break;
				}
				case utility::variant_index_v<driver::request::SelectMonsterZoneLocation, driver::request::Request>:
				{
					object["type"] = "select_monster_zone_location";
					break;
				}
				case utility::variant_index_v<driver::request::SelectAnyMonsterZoneLocation, driver::request::Request>:
				{
					object["type"] = "select_any_monster_zone_location";
					break;
				}
				case utility::variant_index_v<driver::request::SelectMonsterPosition, driver::request::Request>:
				{
					object["type"] = "select_monster_position";
					break;
				}
				case utility::variant_index_v<driver::request::SelectSpellTrapLocation, driver::request::Request>:
				{
					object["type"] = "select_spell_trap_location";
					break;
				}
				case utility::variant_index_v<driver::request::QueryPositionChangeableCards, driver::request::Request>:
				{
					object["type"] = "query_position_changeable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryPositionChangeablePositions, driver::request::Request>:
				{
					object["type"] = "query_position_changeable_positions";
					break;
				}
				case utility::variant_index_v<driver::request::QueryNormalSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_normal_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryNormalSummonType, driver::request::Request>:
				{
					object["type"] = "query_normal_summon_type";
					break;
				}
				case utility::variant_index_v<driver::request::CheckNormalSummonType, driver::request::Request>:
				{
					object["type"] = "check_normal_summon_type";
					break;
				}
				case utility::variant_index_v<driver::request::QueryNormalSummonLocation, driver::request::Request>:
				{
					object["type"] = "query_normal_summon_location";
					break;
				}
				case utility::variant_index_v<driver::request::QueryNormalSummonPlacement, driver::request::Request>:
				{
					object["type"] = "query_normal_summon_placement";
					break;
				}
				case utility::variant_index_v<driver::request::QueryAdvanceSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_advance_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryAdvanceCandidates, driver::request::Request>:
				{
					object["type"] = "query_advance_candidates";
					break;
				}
				case utility::variant_index_v<driver::request::QueryAdvanceSummonType, driver::request::Request>:
				{
					object["type"] = "query_advance_summon_type";
					break;
				}
				case utility::variant_index_v<driver::request::CheckAdvanceSummonType, driver::request::Request>:
				{
					object["type"] = "check_advance_summon_type";
					break;
				}
				case utility::variant_index_v<driver::request::SelectAdvanceTributes, driver::request::Request>:
				{
					object["type"] = "select_advance_tributes";
					break;
				}
				case utility::variant_index_v<driver::request::QueryAdvanceSummonLocation, driver::request::Request>:
				{
					object["type"] = "query_advance_summon_location";
					break;
				}
				case utility::variant_index_v<driver::request::QueryFlipSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_flip_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryDualSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_dual_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryExtraDeckSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_extra_deck_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryExtraDeckSummonMaterialSources, driver::request::Request>:
				{
					object["type"] = "query_extra_deck_summon_material_sources";
					break;
				}
				case utility::variant_index_v<driver::request::QueryExtraDeckSummonMaterials, driver::request::Request>:
				{
					object["type"] = "query_extra_deck_summon_materials";
					break;
				}
				case utility::variant_index_v<driver::request::QueryExtraDeckSummonLocation, driver::request::Request>:
				{
					object["type"] = "query_extra_deck_summon_location";
					break;
				}
				case utility::variant_index_v<driver::request::QueryPendulumSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_pendulum_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QuerySpecialSummonableCards, driver::request::Request>:
				{
					object["type"] = "query_special_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QuerySpecialSummonLocation, driver::request::Request>:
				{
					object["type"] = "query_special_summon_location";
					break;
				}
				case utility::variant_index_v<driver::request::QuerySpecialSummonPlacements, driver::request::Request>:
				{
					object["type"] = "query_special_summon_placements";
					break;
				}
				case utility::variant_index_v<driver::request::QuerySetSpellTrapCards, driver::request::Request>:
				{
					object["type"] = "query_set_spell_trap_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QuerySpellTrapSetLocation, driver::request::Request>:
				{
					object["type"] = "query_spell_trap_set_location";
					break;
				}
				case utility::variant_index_v<driver::request::QueryActivatePendulumCards, driver::request::Request>:
				{
					object["type"] = "query_activate_pendulum_cards";
					break;
				}
				case utility::variant_index_v<driver::request::QueryActivatePendulumSide, driver::request::Request>:
				{
					object["type"] = "query_activate_pendulum_side";
					break;
				}
				case utility::variant_index_v<driver::request::QueryActivatableEffects, driver::request::Request>:
				{
					object["type"] = "query_activatable_effects";
					break;
				}
				case utility::variant_index_v<driver::request::QueryEffectTargets, driver::request::Request>:
				{
					object["type"] = "query_effect_targets";
					break;
				}
				case utility::variant_index_v<driver::request::SelectEffectActivation, driver::request::Request>:
				{
					object["type"] = "select_effect_activation";
					break;
				}
				case utility::variant_index_v<driver::request::SelectEffectTargets, driver::request::Request>:
				{
					object["type"] = "select_effect_targets";
					break;
				}
				case utility::variant_index_v<driver::request::QueryChainableEffects, driver::request::Request>:
				{
					object["type"] = "query_chainable_effects";
					break;
				}
				case utility::variant_index_v<driver::request::SelectChain, driver::request::Request>:
				{
					object["type"] = "select_chain";
					break;
				}
				case utility::variant_index_v<driver::request::QueryAttackableCards, driver::request::Request>:
				{
					object["type"] = "query_attackable_cards";
					break;
				}
				case utility::variant_index_v<driver::request::CheckDirectAttackable, driver::request::Request>:
				{
					object["type"] = "check_direct_attackable";
					break;
				}
				case utility::variant_index_v<driver::request::QueryAttackTargets, driver::request::Request>:
				{
					object["type"] = "query_attack_targets";
					break;
				}
				case utility::variant_index_v<driver::request::SelectAttack, driver::request::Request>:
				{
					object["type"] = "select_attack";
					break;
				}
				case utility::variant_index_v<driver::request::QueryPhaseTransitions, driver::request::Request>:
				{
					object["type"] = "query_phase_transitions";
					break;
				}
				case utility::variant_index_v<driver::request::SelectPhaseTransition, driver::request::Request>:
				{
					object["type"] = "select_phase_transition";
					break;
				}
				case utility::variant_index_v<driver::request::CancelSelection, driver::request::Request>:
				{
					object["type"] = "cancel_selection";
					break;
				}
				case utility::variant_index_v<driver::request::ConfirmSelection, driver::request::Request>:
				{
					object["type"] = "confirm_selection";
					break;
				}
				case utility::variant_index_v<driver::request::PassResponse, driver::request::Request>:
				{
					object["type"] = "pass_response";
					break;
				}
				default: {}
			}

			return object;
		}
	};

	// ==============================================================================================
	// RESPONSE
	// ==============================================================================================

	template<>
	class ValueCodec<driver::response::QueryIdleCommand>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<driver::action::Action>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryIdleCommand>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<driver::action::Action>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryIdleCommand{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryIdleCommand& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<driver::action::Action>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::SubmitAction>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SubmitAction>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SubmitAction{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SubmitAction{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SubmitAction& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::SelectCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SelectCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SelectCards{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SelectCards{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SelectCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::SelectYesNo>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SelectYesNo>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SelectYesNo{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SelectYesNo{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SelectYesNo& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::OrderCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::OrderCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::OrderCards{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::OrderCards{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::OrderCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::SelectMonsterZoneLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SelectMonsterZoneLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SelectMonsterZoneLocation{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SelectMonsterZoneLocation{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SelectMonsterZoneLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::SelectAnyMonsterZoneLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SelectAnyMonsterZoneLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SelectAnyMonsterZoneLocation{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SelectAnyMonsterZoneLocation{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SelectAnyMonsterZoneLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::SelectMonsterPosition>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SelectMonsterPosition>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SelectMonsterPosition{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SelectMonsterPosition{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SelectMonsterPosition& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::SelectSpellTrapLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SelectSpellTrapLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SelectSpellTrapLocation{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SelectSpellTrapLocation{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SelectSpellTrapLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryPositionChangeableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryPositionChangeableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryPositionChangeableCards{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryPositionChangeableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryPositionChangeablePositions>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::Position>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryPositionChangeablePositions>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<domain::Position>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryPositionChangeablePositions{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryPositionChangeablePositions& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<domain::Position>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryNormalSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryNormalSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryNormalSummonableCards{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryNormalSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryNormalSummonType>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::NormalSummonType>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryNormalSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<domain::NormalSummonType>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryNormalSummonType{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryNormalSummonType& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<domain::NormalSummonType>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::CheckNormalSummonType>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::CheckNormalSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::CheckNormalSummonType{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::CheckNormalSummonType{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::CheckNormalSummonType& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryNormalSummonLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::MonsterLocation>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryNormalSummonLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<domain::MonsterLocation>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryNormalSummonLocation{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryNormalSummonLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<domain::MonsterLocation>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryNormalSummonPlacement>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::MonsterPlacement>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryNormalSummonPlacement>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<domain::MonsterPlacement>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryNormalSummonPlacement{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryNormalSummonPlacement& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<domain::MonsterPlacement>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryAdvanceSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryAdvanceSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryAdvanceSummonableCards{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryAdvanceSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryAdvanceCandidates>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryAdvanceCandidates>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryAdvanceCandidates{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryAdvanceCandidates& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryAdvanceSummonType>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::NormalSummonType>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryAdvanceSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<domain::NormalSummonType>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryAdvanceSummonType{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryAdvanceSummonType& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<domain::NormalSummonType>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::CheckAdvanceSummonType>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::CheckAdvanceSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::CheckAdvanceSummonType{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::CheckAdvanceSummonType{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::CheckAdvanceSummonType& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::SelectAdvanceTributes>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SelectAdvanceTributes>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SelectAdvanceTributes{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SelectAdvanceTributes{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SelectAdvanceTributes& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryAdvanceSummonLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::MonsterLocation>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryAdvanceSummonLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<domain::MonsterLocation>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryAdvanceSummonLocation{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryAdvanceSummonLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<domain::MonsterLocation>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryFlipSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryFlipSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryFlipSummonableCards{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryFlipSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryDualSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryDualSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryDualSummonableCards{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryDualSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryExtraDeckSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryExtraDeckSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryExtraDeckSummonableCards{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryExtraDeckSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryExtraDeckSummonMaterialSources>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::SummonMaterialSource>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryExtraDeckSummonMaterialSources>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<domain::SummonMaterialSource>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryExtraDeckSummonMaterialSources{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryExtraDeckSummonMaterialSources& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<domain::SummonMaterialSource>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryExtraDeckSummonMaterials>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryExtraDeckSummonMaterials>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryExtraDeckSummonMaterials{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryExtraDeckSummonMaterials& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryExtraDeckSummonLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// player
			// if (!json.contains("player") || !ValueCodec<domain::Player>::valid(json.at("player")))
			if (!json.contains("player"))
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::AnyMonsterLocation>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryExtraDeckSummonLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto player_it = json.find("player");
			const auto player = ValueCodec<domain::Player>::decode(player_it.value());
			if (!player.has_value())
			{
				return std::nullopt;
			}
			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<domain::AnyMonsterLocation>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryExtraDeckSummonLocation{.player = *player, .candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryExtraDeckSummonLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["candidates"] = ValueCodec<std::vector<domain::AnyMonsterLocation>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryPendulumSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryPendulumSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryPendulumSummonableCards{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryPendulumSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QuerySpecialSummonableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QuerySpecialSummonableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QuerySpecialSummonableCards{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QuerySpecialSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QuerySpecialSummonLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::MonsterLocation>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QuerySpecialSummonLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<domain::MonsterLocation>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QuerySpecialSummonLocation{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QuerySpecialSummonLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<domain::MonsterLocation>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QuerySpecialSummonPlacements>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::MonsterPlacement>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QuerySpecialSummonPlacements>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<domain::MonsterPlacement>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QuerySpecialSummonPlacements{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QuerySpecialSummonPlacements& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<domain::MonsterPlacement>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QuerySetSpellTrapCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QuerySetSpellTrapCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QuerySetSpellTrapCards{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QuerySetSpellTrapCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QuerySpellTrapSetLocation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::SpellTrapLocation>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QuerySpellTrapSetLocation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<domain::SpellTrapLocation>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QuerySpellTrapSetLocation{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QuerySpellTrapSetLocation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<domain::SpellTrapLocation>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryActivatePendulumCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryActivatePendulumCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryActivatePendulumCards{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryActivatePendulumCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryActivatePendulumSide>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::PendulumSide>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryActivatePendulumSide>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<domain::PendulumSide>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryActivatePendulumSide{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryActivatePendulumSide& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<domain::PendulumSide>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryActivatableEffects>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			if (!json.contains("candidates") || !json.at("candidates").is_object())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryActivatableEffects>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			std::map<state::CardInstanceId, std::vector<state::EffectId>> candidates;
			for (const auto& [key, value]: candidates_it.value().items())
			{
				const auto key_json = nlohmann::json::parse(key, nullptr, false);
				if (key_json.is_discarded())
				{
					return std::nullopt;
				}

				const auto card = ValueCodec<state::CardInstanceId>::decode(key_json);
				if (!card.has_value())
				{
					return std::nullopt;
				}
				auto effects = ValueCodec<std::vector<state::EffectId>>::decode(value);
				if (!effects.has_value())
				{
					return std::nullopt;
				}

				candidates.emplace(*card, *std::move(effects));
			}

			return driver::response::QueryActivatableEffects{.candidates = std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryActivatableEffects& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			auto candidates = nlohmann::json::object();
			for (const auto& [card, effects]: data.candidates)
			{
				auto key = ValueCodec<state::CardInstanceId>::encode(card);

				candidates[std::move(key)] = ValueCodec<std::vector<state::EffectId>>::encode(effects);
			}
			object["candidates"] = std::move(candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryEffectTargets>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			// min
			if (!json.contains("min") || !json.at("min").is_number_unsigned())
			{
				return false;
			}

			// max
			if (!json.contains("max") || !json.at("max").is_number_unsigned())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryEffectTargets>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}
			const auto min_it = json.find("min");
			const auto min = min_it.value().get<domain::DeckLocation::size_type>();
			const auto max_it = json.find("max");
			const auto max = max_it.value().get<domain::DeckLocation::size_type>();

			return driver::response::QueryEffectTargets{.candidates = *std::move(candidates), .min = min, .max = max};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryEffectTargets& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);
			object["min"] = data.min;
			object["max"] = data.max;

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::SelectEffectActivation>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SelectEffectActivation>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SelectEffectActivation{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SelectEffectActivation{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SelectEffectActivation& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::SelectEffectTargets>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SelectEffectTargets>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SelectEffectTargets{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SelectEffectTargets{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SelectEffectTargets& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryChainableEffects>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			if (!json.contains("candidates") || !json.at("candidates").is_object())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryChainableEffects>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			std::map<state::CardInstanceId, std::vector<state::EffectId>> candidates;
			for (const auto& [key, value]: candidates_it.value().items())
			{
				const auto key_json = nlohmann::json::parse(key, nullptr, false);
				if (key_json.is_discarded())
				{
					return std::nullopt;
				}

				const auto card = ValueCodec<state::CardInstanceId>::decode(key_json);
				if (!card.has_value())
				{
					return std::nullopt;
				}
				auto effects = ValueCodec<std::vector<state::EffectId>>::decode(value);
				if (!effects.has_value())
				{
					return std::nullopt;
				}

				candidates.emplace(*card, *std::move(effects));
			}

			return driver::response::QueryChainableEffects{.candidates = std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryChainableEffects& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			auto candidates = nlohmann::json::object();
			for (const auto& [card, effects]: data.candidates)
			{
				auto key = ValueCodec<state::CardInstanceId>::encode(card);

				candidates[std::move(key)] = ValueCodec<std::vector<state::EffectId>>::encode(effects);
			}
			object["candidates"] = std::move(candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::SelectChain>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SelectChain>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SelectChain{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SelectChain{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SelectChain& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryAttackableCards>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryAttackableCards>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryAttackableCards{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryAttackableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::CheckDirectAttackable>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::CheckDirectAttackable>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::CheckDirectAttackable{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::CheckDirectAttackable{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::CheckDirectAttackable& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryAttackTargets>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::CardInstanceId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryAttackTargets>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<state::CardInstanceId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryAttackTargets{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryAttackTargets& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::SelectAttack>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SelectAttack>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SelectAttack{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SelectAttack{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SelectAttack& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryPhaseTransitions>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// candidates
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::Phase>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryPhaseTransitions>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto candidates_it = json.find("candidates");
			auto candidates = ValueCodec<std::vector<domain::Phase>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::response::QueryPhaseTransitions{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryPhaseTransitions& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<domain::Phase>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::SelectPhaseTransition>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SelectPhaseTransition>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SelectPhaseTransition{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SelectPhaseTransition{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SelectPhaseTransition& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::CancelSelection>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::CancelSelection>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::CancelSelection{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::CancelSelection{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::CancelSelection& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::ConfirmSelection>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::ConfirmSelection>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::ConfirmSelection{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::ConfirmSelection{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::ConfirmSelection& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::PassResponse>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// reject
			// if (json.contains("reject") && !ValueCodec<domain::RejectReason>::valid(json.at("reject")))
			// {
			// 	return false;
			// }

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::PassResponse>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::PassResponse{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::PassResponse{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::PassResponse& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			if (data.reject.has_value())
			{
				object["reject"] = ValueCodec<domain::RejectReason>::encode(*data.reject);
			}

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::Response>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// type相比于variant_index更方便校验
			if (json.contains("type") && !json.at("type").is_string())
			{
				return false;
			}

			if (!ValueCodec<codec_detail::Wrap<driver::response::Response>>::valid(json))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::Response>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			// 如果有type则按照type匹配
			if (const auto type_it = json.find("type");
				type_it != json.end())
			{
				const auto do_decode = [&]<typename L> noexcept -> std::optional<driver::response::Response>
				{
					if (const auto r = ValueCodec<L>::decode(json);
						r.has_value())
					{
						return driver::response::Response{*std::move(r)};
					}

					return std::nullopt;
				};

				const auto& type = type_it.value().get_ref<const nlohmann::json::string_t&>();

				if (type == "query_idle_command")
				{
					return do_decode.operator()<driver::response::QueryIdleCommand>();
				}
				if (type == "submit_action")
				{
					return do_decode.operator()<driver::response::SubmitAction>();
				}
				if (type == "select_cards")
				{
					return do_decode.operator()<driver::response::SelectCards>();
				}
				if (type == "select_yes_no")
				{
					return do_decode.operator()<driver::response::SelectYesNo>();
				}
				if (type == "order_cards")
				{
					return do_decode.operator()<driver::response::OrderCards>();
				}
				if (type == "select_monster_zone_location")
				{
					return do_decode.operator()<driver::response::SelectMonsterZoneLocation>();
				}
				if (type == "select_any_monster_zone_location")
				{
					return do_decode.operator()<driver::response::SelectAnyMonsterZoneLocation>();
				}
				if (type == "select_monster_position")
				{
					return do_decode.operator()<driver::response::SelectMonsterPosition>();
				}
				if (type == "select_spell_trap_location")
				{
					return do_decode.operator()<driver::response::SelectSpellTrapLocation>();
				}
				if (type == "query_position_changeable_cards")
				{
					return do_decode.operator()<driver::response::QueryPositionChangeableCards>();
				}
				if (type == "query_position_changeable_positions")
				{
					return do_decode.operator()<driver::response::QueryPositionChangeablePositions>();
				}
				if (type == "query_normal_summonable_cards")
				{
					return do_decode.operator()<driver::response::QueryNormalSummonableCards>();
				}
				if (type == "query_normal_summon_type")
				{
					return do_decode.operator()<driver::response::QueryNormalSummonType>();
				}
				if (type == "check_normal_summon_type")
				{
					return do_decode.operator()<driver::response::CheckNormalSummonType>();
				}
				if (type == "query_normal_summon_location")
				{
					return do_decode.operator()<driver::response::QueryNormalSummonLocation>();
				}
				if (type == "query_normal_summon_placement")
				{
					return do_decode.operator()<driver::response::QueryNormalSummonPlacement>();
				}
				if (type == "query_advance_summonable_cards")
				{
					return do_decode.operator()<driver::response::QueryAdvanceSummonableCards>();
				}
				if (type == "query_advance_candidates")
				{
					return do_decode.operator()<driver::response::QueryAdvanceCandidates>();
				}
				if (type == "query_advance_summon_type")
				{
					return do_decode.operator()<driver::response::QueryAdvanceSummonType>();
				}
				if (type == "check_advance_summon_type")
				{
					return do_decode.operator()<driver::response::CheckAdvanceSummonType>();
				}
				if (type == "select_advance_tributes")
				{
					return do_decode.operator()<driver::response::SelectAdvanceTributes>();
				}
				if (type == "query_advance_summon_location")
				{
					return do_decode.operator()<driver::response::QueryAdvanceSummonLocation>();
				}
				if (type == "query_flip_summonable_cards")
				{
					return do_decode.operator()<driver::response::QueryFlipSummonableCards>();
				}
				if (type == "query_dual_summonable_cards")
				{
					return do_decode.operator()<driver::response::QueryDualSummonableCards>();
				}
				if (type == "query_extra_deck_summonable_cards")
				{
					return do_decode.operator()<driver::response::QueryExtraDeckSummonableCards>();
				}
				if (type == "query_extra_deck_summon_material_sources")
				{
					return do_decode.operator()<driver::response::QueryExtraDeckSummonMaterialSources>();
				}
				if (type == "query_extra_deck_summon_materials")
				{
					return do_decode.operator()<driver::response::QueryExtraDeckSummonMaterials>();
				}
				if (type == "query_extra_deck_summon_location")
				{
					return do_decode.operator()<driver::response::QueryExtraDeckSummonLocation>();
				}
				if (type == "query_pendulum_summonable_cards")
				{
					return do_decode.operator()<driver::response::QueryPendulumSummonableCards>();
				}
				if (type == "query_special_summonable_cards")
				{
					return do_decode.operator()<driver::response::QuerySpecialSummonableCards>();
				}
				if (type == "query_special_summon_location")
				{
					return do_decode.operator()<driver::response::QuerySpecialSummonLocation>();
				}
				if (type == "query_special_summon_placements")
				{
					return do_decode.operator()<driver::response::QuerySpecialSummonPlacements>();
				}
				if (type == "query_set_spell_trap_cards")
				{
					return do_decode.operator()<driver::response::QuerySetSpellTrapCards>();
				}
				if (type == "query_spell_trap_set_location")
				{
					return do_decode.operator()<driver::response::QuerySpellTrapSetLocation>();
				}
				if (type == "query_activate_pendulum_cards")
				{
					return do_decode.operator()<driver::response::QueryActivatePendulumCards>();
				}
				if (type == "query_activate_pendulum_side")
				{
					return do_decode.operator()<driver::response::QueryActivatePendulumSide>();
				}
				if (type == "query_activatable_effects")
				{
					return do_decode.operator()<driver::response::QueryActivatableEffects>();
				}
				if (type == "query_effect_targets")
				{
					return do_decode.operator()<driver::response::QueryEffectTargets>();
				}
				if (type == "select_effect_activation")
				{
					return do_decode.operator()<driver::response::SelectEffectActivation>();
				}
				if (type == "select_effect_targets")
				{
					return do_decode.operator()<driver::response::SelectEffectTargets>();
				}
				if (type == "query_chainable_effects")
				{
					return do_decode.operator()<driver::response::QueryChainableEffects>();
				}
				if (type == "select_chain")
				{
					return do_decode.operator()<driver::response::SelectChain>();
				}
				if (type == "query_attackable_cards")
				{
					return do_decode.operator()<driver::response::QueryAttackableCards>();
				}
				if (type == "check_direct_attackable")
				{
					return do_decode.operator()<driver::response::CheckDirectAttackable>();
				}
				if (type == "query_attack_targets")
				{
					return do_decode.operator()<driver::response::QueryAttackTargets>();
				}
				if (type == "select_attack")
				{
					return do_decode.operator()<driver::response::SelectAttack>();
				}
				if (type == "query_phase_transitions")
				{
					return do_decode.operator()<driver::response::QueryPhaseTransitions>();
				}
				if (type == "select_phase_transition")
				{
					return do_decode.operator()<driver::response::SelectPhaseTransition>();
				}
				if (type == "cancel_selection")
				{
					return do_decode.operator()<driver::response::CancelSelection>();
				}
				if (type == "confirm_selection")
				{
					return do_decode.operator()<driver::response::ConfirmSelection>();
				}
				if (type == "pass_response")
				{
					return do_decode.operator()<driver::response::PassResponse>();
				}

				// 未知type直接失败
				return std::nullopt;
			}

			// 没有type按照index匹配
			return ValueCodec<codec_detail::Wrap<driver::response::Response>>::decode(json);
		}

		[[nodiscard]] static auto encode(const driver::response::Response& data) noexcept -> nlohmann::json
		{
			auto object = ValueCodec<codec_detail::Wrap<driver::response::Response>>::encode(data);

			switch (data.index())
			{
				case utility::variant_index_v<driver::response::QueryIdleCommand, driver::response::Response>:
				{
					object["type"] = "query_idle_command";
					break;
				}
				case utility::variant_index_v<driver::response::SubmitAction, driver::response::Response>:
				{
					object["type"] = "submit_action";
					break;
				}
				case utility::variant_index_v<driver::response::SelectCards, driver::response::Response>:
				{
					object["type"] = "select_cards";
					break;
				}
				case utility::variant_index_v<driver::response::SelectYesNo, driver::response::Response>:
				{
					object["type"] = "select_yes_no";
					break;
				}
				case utility::variant_index_v<driver::response::OrderCards, driver::response::Response>:
				{
					object["type"] = "order_cards";
					break;
				}
				case utility::variant_index_v<driver::response::SelectMonsterZoneLocation, driver::response::Response>:
				{
					object["type"] = "select_monster_zone_location";
					break;
				}
				case utility::variant_index_v<driver::response::SelectAnyMonsterZoneLocation, driver::response::Response>:
				{
					object["type"] = "select_any_monster_zone_location";
					break;
				}
				case utility::variant_index_v<driver::response::SelectMonsterPosition, driver::response::Response>:
				{
					object["type"] = "select_monster_position";
					break;
				}
				case utility::variant_index_v<driver::response::SelectSpellTrapLocation, driver::response::Response>:
				{
					object["type"] = "select_spell_trap_location";
					break;
				}
				case utility::variant_index_v<driver::response::QueryPositionChangeableCards, driver::response::Response>:
				{
					object["type"] = "query_position_changeable_cards";
					break;
				}
				case utility::variant_index_v<driver::response::QueryPositionChangeablePositions, driver::response::Response>:
				{
					object["type"] = "query_position_changeable_positions";
					break;
				}
				case utility::variant_index_v<driver::response::QueryNormalSummonableCards, driver::response::Response>:
				{
					object["type"] = "query_normal_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::response::QueryNormalSummonType, driver::response::Response>:
				{
					object["type"] = "query_normal_summon_type";
					break;
				}
				case utility::variant_index_v<driver::response::CheckNormalSummonType, driver::response::Response>:
				{
					object["type"] = "check_normal_summon_type";
					break;
				}
				case utility::variant_index_v<driver::response::QueryNormalSummonLocation, driver::response::Response>:
				{
					object["type"] = "query_normal_summon_location";
					break;
				}
				case utility::variant_index_v<driver::response::QueryNormalSummonPlacement, driver::response::Response>:
				{
					object["type"] = "query_normal_summon_placement";
					break;
				}
				case utility::variant_index_v<driver::response::QueryAdvanceSummonableCards, driver::response::Response>:
				{
					object["type"] = "query_advance_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::response::QueryAdvanceCandidates, driver::response::Response>:
				{
					object["type"] = "query_advance_candidates";
					break;
				}
				case utility::variant_index_v<driver::response::QueryAdvanceSummonType, driver::response::Response>:
				{
					object["type"] = "query_advance_summon_type";
					break;
				}
				case utility::variant_index_v<driver::response::CheckAdvanceSummonType, driver::response::Response>:
				{
					object["type"] = "check_advance_summon_type";
					break;
				}
				case utility::variant_index_v<driver::response::SelectAdvanceTributes, driver::response::Response>:
				{
					object["type"] = "select_advance_tributes";
					break;
				}
				case utility::variant_index_v<driver::response::QueryAdvanceSummonLocation, driver::response::Response>:
				{
					object["type"] = "query_advance_summon_location";
					break;
				}
				case utility::variant_index_v<driver::response::QueryFlipSummonableCards, driver::response::Response>:
				{
					object["type"] = "query_flip_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::response::QueryDualSummonableCards, driver::response::Response>:
				{
					object["type"] = "query_dual_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::response::QueryExtraDeckSummonableCards, driver::response::Response>:
				{
					object["type"] = "query_extra_deck_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::response::QueryExtraDeckSummonMaterialSources, driver::response::Response>:
				{
					object["type"] = "query_extra_deck_summon_material_sources";
					break;
				}
				case utility::variant_index_v<driver::response::QueryExtraDeckSummonMaterials, driver::response::Response>:
				{
					object["type"] = "query_extra_deck_summon_materials";
					break;
				}
				case utility::variant_index_v<driver::response::QueryExtraDeckSummonLocation, driver::response::Response>:
				{
					object["type"] = "query_extra_deck_summon_location";
					break;
				}
				case utility::variant_index_v<driver::response::QueryPendulumSummonableCards, driver::response::Response>:
				{
					object["type"] = "query_pendulum_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::response::QuerySpecialSummonableCards, driver::response::Response>:
				{
					object["type"] = "query_special_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::response::QuerySpecialSummonLocation, driver::response::Response>:
				{
					object["type"] = "query_special_summon_location";
					break;
				}
				case utility::variant_index_v<driver::response::QuerySpecialSummonPlacements, driver::response::Response>:
				{
					object["type"] = "query_special_summon_placements";
					break;
				}
				case utility::variant_index_v<driver::response::QuerySetSpellTrapCards, driver::response::Response>:
				{
					object["type"] = "query_set_spell_trap_cards";
					break;
				}
				case utility::variant_index_v<driver::response::QuerySpellTrapSetLocation, driver::response::Response>:
				{
					object["type"] = "query_spell_trap_set_location";
					break;
				}
				case utility::variant_index_v<driver::response::QueryActivatePendulumCards, driver::response::Response>:
				{
					object["type"] = "query_activate_pendulum_cards";
					break;
				}
				case utility::variant_index_v<driver::response::QueryActivatePendulumSide, driver::response::Response>:
				{
					object["type"] = "query_activate_pendulum_side";
					break;
				}
				case utility::variant_index_v<driver::response::QueryActivatableEffects, driver::response::Response>:
				{
					object["type"] = "query_activatable_effects";
					break;
				}
				case utility::variant_index_v<driver::response::QueryEffectTargets, driver::response::Response>:
				{
					object["type"] = "query_effect_targets";
					break;
				}
				case utility::variant_index_v<driver::response::SelectEffectActivation, driver::response::Response>:
				{
					object["type"] = "select_effect_activation";
					break;
				}
				case utility::variant_index_v<driver::response::SelectEffectTargets, driver::response::Response>:
				{
					object["type"] = "select_effect_targets";
					break;
				}
				case utility::variant_index_v<driver::response::QueryChainableEffects, driver::response::Response>:
				{
					object["type"] = "query_chainable_effects";
					break;
				}
				case utility::variant_index_v<driver::response::SelectChain, driver::response::Response>:
				{
					object["type"] = "select_chain";
					break;
				}
				case utility::variant_index_v<driver::response::QueryAttackableCards, driver::response::Response>:
				{
					object["type"] = "query_attackable_cards";
					break;
				}
				case utility::variant_index_v<driver::response::CheckDirectAttackable, driver::response::Response>:
				{
					object["type"] = "check_direct_attackable";
					break;
				}
				case utility::variant_index_v<driver::response::QueryAttackTargets, driver::response::Response>:
				{
					object["type"] = "query_attack_targets";
					break;
				}
				case utility::variant_index_v<driver::response::SelectAttack, driver::response::Response>:
				{
					object["type"] = "select_attack";
					break;
				}
				case utility::variant_index_v<driver::response::QueryPhaseTransitions, driver::response::Response>:
				{
					object["type"] = "query_phase_transitions";
					break;
				}
				case utility::variant_index_v<driver::response::SelectPhaseTransition, driver::response::Response>:
				{
					object["type"] = "select_phase_transition";
					break;
				}
				case utility::variant_index_v<driver::response::CancelSelection, driver::response::Response>:
				{
					object["type"] = "cancel_selection";
					break;
				}
				case utility::variant_index_v<driver::response::ConfirmSelection, driver::response::Response>:
				{
					object["type"] = "confirm_selection";
					break;
				}
				case utility::variant_index_v<driver::response::PassResponse, driver::response::Response>:
				{
					object["type"] = "pass_response";
					break;
				}
				default: {}
			}

			return object;
		}
	};

	// ==============================================================================================
	// MESSAGE
	// ==============================================================================================

	// 卡组
	template<>
	class ValueCodec<Deck>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// filename
			if (!json.contains("filename") || !json.at("filename").is_string())
			{
				return false;
			}

			// name
			if (!json.contains("name") || !json.at("name").is_string())
			{
				return false;
			}

			// main
			// if (!json.contains("main") || !ValueCodec<std::vector<domain::CardCode>>::valid(json.at("main")))
			if (!json.contains("main") || !json.at("main").is_array())
			{
				return false;
			}

			// extra
			// if (json.contains("extra") && !ValueCodec<std::vector<domain::CardCode>>::valid(json.at("extra")))
			if (json.contains("extra") && !json.at("extra").is_array())
			{
				return false;
			}

			// side
			// if (json.contains("side") && !ValueCodec<std::vector<domain::CardCode>>::valid(json.at("side")))
			if (json.contains("side") && !json.at("side").is_array())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<Deck>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto filename_it = json.find("filename");
			auto filename = filename_it.value().get<nlohmann::json::string_t>();
			const auto name_it = json.find("name");
			auto name = name_it.value().get<nlohmann::json::string_t>();
			const auto main_it = json.find("main");
			auto main = ValueCodec<std::vector<domain::CardCode>>::decode(main_it.value());
			if (!main.has_value())
			{
				return std::nullopt;
			}
			std::vector<domain::CardCode> extra;
			if (const auto extra_it = json.find("extra");
				extra_it != json.end())
			{
				auto extra_result = ValueCodec<std::vector<domain::CardCode>>::decode(extra_it.value());
				if (!extra_result.has_value())
				{
					return std::nullopt;
				}

				extra = *std::move(extra_result);
			}
			std::vector<domain::CardCode> side;
			if (const auto side_it = json.find("side");
				side_it != json.end())
			{
				auto side_result = ValueCodec<std::vector<domain::CardCode>>::decode(side_it.value());
				if (!side_result.has_value())
				{
					return std::nullopt;
				}

				side = *std::move(side_result);
			}

			return Deck
			{
					.filename = std::move(filename),
					.name = std::move(name),
					.main_deck = *std::move(main),
					.extra_deck = std::move(extra),
					.side_deck = std::move(side),
			};
		}

		[[nodiscard]] static auto encode(const Deck& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["filename"] = data.filename;
			object["name"] = data.name;
			object["main"] = ValueCodec<std::vector<domain::CardCode>>::encode(data.main_deck);
			object["extra"] = ValueCodec<std::vector<domain::CardCode>>::encode(data.extra_deck);
			object["side"] = ValueCodec<std::vector<domain::CardCode>>::encode(data.side_deck);

			return object;
		}
	};

	// 加入房间
	template<>
	class ValueCodec<message::Join>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// seat -- 可缺省/可为null,表示自动分配
			if (json.contains("seat") && !ValueCodec<std::optional<domain::Player>>::valid(json.at("seat")))
			{
				return false;
			}

			// session
			if (json.contains("session") && !json.at("session").is_string())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::Join>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			std::optional<domain::Player> seat;
			if (const auto seat_it = json.find("seat"); seat_it != json.end())
			{
				auto seat_result = ValueCodec<std::optional<domain::Player>>::decode(seat_it.value());
				if (!seat_result.has_value())
				{
					return std::nullopt;
				}
				seat = *seat_result;
			}

			std::string session;
			if (const auto session_it = json.find("session"); session_it != json.end())
			{
				session = session_it.value().get<nlohmann::json::string_t>();
			}

			return message::Join{.seat = seat, .session = std::move(session)};
		}

		[[nodiscard]] static auto encode(const message::Join& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["seat"] = ValueCodec<std::optional<domain::Player>>::encode(data.seat);
			object["session"] = data.session;

			return object;
		}
	};

	// 选择卡组
	template<>
	class ValueCodec<message::ChooseDeck>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// deck
			if (!json.contains("deck") || !json.at("deck").is_string())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::ChooseDeck>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto deck_it = json.find("deck");
			auto deck = deck_it.value().get<nlohmann::json::string_t>();

			return message::ChooseDeck{.deck = std::move(deck)};
		}

		[[nodiscard]] static auto encode(const message::ChooseDeck& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["deck"] = data.deck;

			return object;
		}
	};

	// 投降
	template<>
	class ValueCodec<message::Surrender>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::Surrender>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			return message::Surrender{};
		}

		[[nodiscard]] static auto encode(const message::Surrender& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			return object;
		}
	};

	// 房间同步请求
	template<>
	class ValueCodec<message::Sync>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_object();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::Sync>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			return message::Sync{};
		}

		[[nodiscard]] static auto encode(const message::Sync& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			return object;
		}
	};

	// 决斗回应
	template<>
	class ValueCodec<message::Response>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			if (!json.contains("request_id") || !json.at("request_id").is_number_unsigned())
			{
				return false;
			}

			if (!json.contains("payload") || !ValueCodec<driver::response::Response>::valid(json.at("payload")))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::Response>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto request_id = json.at("request_id").get<nlohmann::json::number_unsigned_t>();
			auto payload = ValueCodec<driver::response::Response>::decode(json.at("payload"));
			if (!payload.has_value())
			{
				return std::nullopt;
			}

			return message::Response{.request_id = request_id, .payload = *std::move(payload)};
		}

		[[nodiscard]] static auto encode(const message::Response& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["request_id"] = data.request_id;
			object["payload"] = ValueCodec<driver::response::Response>::encode(data.payload);

			return object;
		}
	};

	// 座位分配成功
	template<>
	class ValueCodec<message::Joined>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			if (!json.contains("seat") || !ValueCodec<domain::Player>::valid(json.at("seat")))
			{
				return false;
			}

			if (json.contains("session") && !json.at("session").is_string())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::Joined>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto seat = ValueCodec<domain::Player>::decode(json.at("seat"));
			if (!seat.has_value())
			{
				return std::nullopt;
			}

			std::string session;
			if (const auto session_it = json.find("session"); session_it != json.end())
			{
				session = session_it.value().get<nlohmann::json::string_t>();
			}

			return message::Joined{.seat = *seat, .session = std::move(session)};
		}

		[[nodiscard]] static auto encode(const message::Joined& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["seat"] = ValueCodec<domain::Player>::encode(data.seat);
			object["session"] = data.session;

			return object;
		}
	};

	template<>
	class ValueCodec<message::LobbyState::Seat>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// empty => 未选择卡组
			if (!json.contains("deck_name") || !json.at("deck_name").is_string())
			{
				return false;
			}

			// ready = 已选择卡组 + 已准备
			if (!json.contains("ready") || !json.at("ready").is_boolean())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::LobbyState::Seat>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto deck_name_it = json.find("deck_name");
			auto deck_name = deck_name_it.value().get<nlohmann::json::string_t>();
			const auto ready_it = json.find("ready");
			const auto ready = ready_it.value().get<nlohmann::json::boolean_t>();

			return message::LobbyState::Seat{.deck_name = std::move(deck_name), .ready = ready};
		}

		[[nodiscard]] static auto encode(const message::LobbyState::Seat& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["deck_name"] = data.deck_name;
			object["ready"] = data.ready;

			return object;
		}
	};

	// 大厅状态
	template<>
	class ValueCodec<message::LobbyState>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// decks
			// if (!json.contains("decks") || !ValueCodec<std::vector<Deck>>::valid(json.at("decks")))
			if (!json.contains("decks") || !json.at("decks").is_array())
			{
				return false;
			}

			// seats
			// if (!json.contains("seats") || !ValueCodec<message::LobbyState::seats_type>::valid(json.at("seats")))
			if (!json.contains("seats") || !json.at("seats").is_array())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::LobbyState>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto decks_it = json.find("decks");
			auto decks = ValueCodec<std::vector<Deck>>::decode(decks_it.value());
			if (!decks.has_value())
			{
				return std::nullopt;
			}
			const auto seats_it = json.find("seats");
			auto seats = ValueCodec<message::LobbyState::seats_type>::decode(seats_it.value());
			if (!seats.has_value())
			{
				return std::nullopt;
			}

			return message::LobbyState{.decks = *std::move(decks), .seats = *std::move(seats)};
		}

		[[nodiscard]] static auto encode(const message::LobbyState& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["decks"] = ValueCodec<std::vector<Deck>>::encode(data.decks);
			object["seats"] = ValueCodec<std::array<message::LobbyState::Seat, message::LobbyState::seats_size>>::encode(data.seats);

			return object;
		}
	};

	template<>
	class ValueCodec<message::RoomSync::Seat>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// empty => 未选择卡组
			if (!json.contains("deck_name") || !json.at("deck_name").is_string())
			{
				return false;
			}

			// ready = 已选择卡组 + 已准备
			if (!json.contains("ready") || !json.at("ready").is_boolean())
			{
				return false;
			}

			// joined = 已有玩家加入
			if (!json.contains("joined") || !json.at("joined").is_boolean())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::RoomSync::Seat>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto deck_name_it = json.find("deck_name");
			auto deck_name = deck_name_it.value().get<nlohmann::json::string_t>();
			const auto ready_it = json.find("ready");
			const auto ready = ready_it.value().get<nlohmann::json::boolean_t>();
			const auto joined_it = json.find("joined");
			const auto joined = joined_it.value().get<nlohmann::json::boolean_t>();

			return message::RoomSync::Seat{.deck_name = std::move(deck_name), .ready = ready, .joined = joined};
		}

		[[nodiscard]] static auto encode(const message::RoomSync::Seat& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["deck_name"] = data.deck_name;
			object["ready"] = data.ready;
			object["joined"] = data.joined;

			return object;
		}
	};

	// 房间同步
	template<>
	class ValueCodec<message::RoomSync>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			if (json.contains("started") && !json.at("started").is_boolean())
			{
				return false;
			}
			if (json.contains("finished") && !json.at("finished").is_boolean())
			{
				return false;
			}

			// seats
			if (!json.contains("seats") || !json.at("seats").is_array())
			{
				return false;
			}

			if (json.contains("lobby") && !ValueCodec<message::LobbyState>::valid(json.at("lobby")))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::RoomSync>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			bool started = false;
			if (const auto started_it = json.find("started"); started_it != json.end())
			{
				started = started_it.value().get<nlohmann::json::boolean_t>();
			}
			bool finished = false;
			if (const auto finished_it = json.find("finished"); finished_it != json.end())
			{
				finished = finished_it.value().get<nlohmann::json::boolean_t>();
			}

			const auto seats_it = json.find("seats");
			auto seats = ValueCodec<message::RoomSync::seats_type>::decode(seats_it.value());
			if (!seats.has_value())
			{
				return std::nullopt;
			}

			std::optional<message::LobbyState> lobby;
			if (const auto lobby_it = json.find("lobby"); lobby_it != json.end())
			{
				auto lobby_result = ValueCodec<message::LobbyState>::decode(lobby_it.value());
				if (!lobby_result.has_value())
				{
					return std::nullopt;
				}
				lobby = *std::move(lobby_result);
			}

			return message::RoomSync{.started = started, .finished = finished, .seats = *std::move(seats), .lobby = std::move(lobby)};
		}

		[[nodiscard]] static auto encode(const message::RoomSync& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["started"] = data.started;
			object["finished"] = data.finished;
			object["seats"] = ValueCodec<message::RoomSync::seats_type>::encode(data.seats);
			if (data.lobby.has_value())
			{
				object["lobby"] = ValueCodec<message::LobbyState>::encode(*data.lobby);
			}

			return object;
		}
	};

	// 决斗请求
	template<>
	class ValueCodec<message::Request>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			if (!json.contains("request_id") || !json.at("request_id").is_number_unsigned())
			{
				return false;
			}

			if (!json.contains("payload") || !ValueCodec<driver::request::Request>::valid(json.at("payload")))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::Request>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto request_id = json.at("request_id").get<nlohmann::json::number_unsigned_t>();
			auto payload = ValueCodec<driver::request::Request>::decode(json.at("payload"));
			if (!payload.has_value())
			{
				return std::nullopt;
			}

			return message::Request{.request_id = request_id, .payload = *std::move(payload)};
		}

		[[nodiscard]] static auto encode(const message::Request& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["request_id"] = data.request_id;
			object["payload"] = ValueCodec<driver::request::Request>::encode(data.payload);

			return object;
		}
	};


	// 日志
	template<>
	class ValueCodec<message::Log>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			if (!json.contains("sequence") || !json.at("sequence").is_number_unsigned())
			{
				return false;
			}

			// entry
			if (!json.contains("entry") || !json.at("entry").is_object())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::Log>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto sequence = json.at("sequence").get<nlohmann::json::number_unsigned_t>();
			auto entry = json.at("entry");

			return message::Log{.sequence = sequence, .entry = std::move(entry)};
		}

		[[nodiscard]] static auto encode(const message::Log& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["sequence"] = data.sequence;
			object["entry"] = data.entry;

			return object;
		}
	};

	// 错误
	template<>
	class ValueCodec<message::Error>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			if (!json.contains("message") || !json.at("message").is_string())
			{
				return false;
			}

			if (json.contains("source") && !json.at("source").is_string())
			{
				return false;
			}

			if (json.contains("request_id") && !json.at("request_id").is_number_unsigned())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::Error>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			message::Error error;
			error.message = json.at("message").get<nlohmann::json::string_t>();
			if (const auto source_it = json.find("source"); source_it != json.end())
			{
				error.source = source_it.value().get<nlohmann::json::string_t>();
			}
			if (const auto request_id_it = json.find("request_id"); request_id_it != json.end())
			{
				error.request_id = request_id_it.value().get<nlohmann::json::number_unsigned_t>();
			}

			return error;
		}

		[[nodiscard]] static auto encode(const message::Error& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["message"] = data.message;
			if (data.source.has_value())
			{
				object["source"] = *data.source;
			}
			if (data.request_id.has_value())
			{
				object["request_id"] = *data.request_id;
			}

			return object;
		}
	};


	template<>
	class ValueCodec<message::Message>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			if (!json.is_object())
			{
				return false;
			}

			// type相比于variant_index更方便校验
			if (json.contains("type") && !json.at("type").is_string())
			{
				return false;
			}

			if (!ValueCodec<codec_detail::Wrap<message::Message>>::valid(json))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<message::Message>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			// 如果有type则按照type匹配
			if (const auto type_it = json.find("type");
				type_it != json.end())
			{
				const auto do_decode = [&]<typename M>() noexcept -> std::optional<message::Message>
				{
					if (const auto r = ValueCodec<M>::decode(json);
						r.has_value())
					{
						return message::Message{*std::move(r)};
					}

					return std::nullopt;
				};

				const auto& type = type_it.value().get_ref<const nlohmann::json::string_t&>();

				if (type == "join")
				{
					return do_decode.operator()<message::Join>();
				}
				if (type == "choose_deck")
				{
					return do_decode.operator()<message::ChooseDeck>();
				}
				if (type == "surrender")
				{
					return do_decode.operator()<message::Surrender>();
				}
				if (type == "sync")
				{
					return do_decode.operator()<message::Sync>();
				}
				if (type == "response")
				{
					return do_decode.operator()<message::Response>();
				}
				if (type == "joined")
				{
					return do_decode.operator()<message::Joined>();
				}
				if (type == "lobby")
				{
					return do_decode.operator()<message::LobbyState>();
				}
				if (type == "room_sync")
				{
					return do_decode.operator()<message::RoomSync>();
				}
				if (type == "request")
				{
					return do_decode.operator()<message::Request>();
				}
				if (type == "log")
				{
					return do_decode.operator()<message::Log>();
				}
				if (type == "error")
				{
					return do_decode.operator()<message::Error>();
				}

				// 未知type直接失败
				return std::nullopt;
			}

			// 没有type按照index匹配
			return ValueCodec<codec_detail::Wrap<message::Message>>::decode(json);
		}

		[[nodiscard]] static auto encode(const message::Message& data) noexcept -> nlohmann::json
		{
			auto object = ValueCodec<codec_detail::Wrap<message::Message>>::encode(data);

			switch (data.index())
			{
				case utility::variant_index_v<message::Join, message::Message>:
				{
					object["type"] = "join";
					break;
				}
				case utility::variant_index_v<message::ChooseDeck, message::Message>:
				{
					object["type"] = "choose_deck";
					break;
				}
				case utility::variant_index_v<message::Surrender, message::Message>:
				{
					object["type"] = "surrender";
					break;
				}
				case utility::variant_index_v<message::Sync, message::Message>:
				{
					object["type"] = "sync";
					break;
				}
				case utility::variant_index_v<message::Response, message::Message>:
				{
					object["type"] = "response";
					break;
				}
				case utility::variant_index_v<message::Joined, message::Message>:
				{
					object["type"] = "joined";
					break;
				}
				case utility::variant_index_v<message::LobbyState, message::Message>:
				{
					object["type"] = "lobby";
					break;
				}
				case utility::variant_index_v<message::RoomSync, message::Message>:
				{
					object["type"] = "room_sync";
					break;
				}
				case utility::variant_index_v<message::Request, message::Message>:
				{
					object["type"] = "request";
					break;
				}
				case utility::variant_index_v<message::Log, message::Message>:
				{
					object["type"] = "log";
					break;
				}
				case utility::variant_index_v<message::Error, message::Message>:
				{
					object["type"] = "error";
					break;
				}
				default: {}
			}

			return object;
		}
	};

	// ==============================================================================================
	// 
	// ==============================================================================================

	template<typename T>
	[[nodiscard]] auto valid(const nlohmann::json& json) noexcept -> bool
	{
		return ValueCodec<T>::valid(json);
	}

	template<typename T>
	[[nodiscard]] auto decode(const nlohmann::json& json) noexcept -> std::optional<T>
	{
		return ValueCodec<T>::decode(json);
	}

	template<typename T>
	[[nodiscard]] auto encode(const T& data) noexcept -> nlohmann::json
	{
		return ValueCodec<T>::encode(data);
	}

	template<typename T>
	class FieldCodec
	{
	public:
		[[nodiscard]] static auto read(const std::string_view key, const nlohmann::json& json) noexcept -> std::optional<T>
		{
			const auto it = json.find(key);
			if (it == json.end())
			{
				return std::nullopt;
			}

			return ValueCodec<T>::decode(it.value());
		}

		static auto write(const std::string_view key, const T& data, nlohmann::json& json) noexcept -> void
		{
			json[key] = ValueCodec<T>::encode(data);
		}
	};
}

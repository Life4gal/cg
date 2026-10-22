#pragma once

#include <cg/domain/card.hpp>
#include <cg/domain/player.hpp>
#include <cg/domain/field.hpp>
#include <cg/domain/summon.hpp>
#include <cg/domain/phase.hpp>
#include <cg/domain/selection_context.hpp>
#include <cg/domain/reject_reason.hpp>

#include <cg/codec/codec.hpp>

namespace cg::codec
{
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
	// domain::Location / domain::AnyFieldLocation / domain::AnyMonsterLocation 处理方式完全相同,一并处理
		requires(
			std::is_same_v<T, domain::Location> ||
			std::is_same_v<T, domain::AnyFieldLocation> ||
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

			if (!ValueCodec<VariantWrap<T>>::valid(json))
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
				else if constexpr (std::is_same_v<T, domain::AnyFieldLocation>)
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
			return ValueCodec<VariantWrap<T>>::decode(json);
		}

		[[nodiscard]] static auto encode(const T& any) noexcept -> nlohmann::json
		{
			auto object = ValueCodec<VariantWrap<T>>::encode(any);

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
			else if constexpr (std::is_same_v<T, domain::AnyFieldLocation>)
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
	// domain::Placement / domain::AnyFieldPlacement / domain::AnyMonsterPlacement 处理方式完全相同,一并处理
		requires(
			std::is_same_v<T, domain::Placement> ||
			std::is_same_v<T, domain::AnyFieldPlacement> ||
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

			if (!ValueCodec<VariantWrap<T>>::valid(json))
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
				else if constexpr (std::is_same_v<T, domain::AnyFieldPlacement>)
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
			return ValueCodec<VariantWrap<T>>::decode(json);
		}

		[[nodiscard]] static auto encode(const T& any) noexcept -> nlohmann::json
		{
			auto object = ValueCodec<VariantWrap<T>>::encode(any);

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
			else if constexpr (std::is_same_v<T, domain::AnyFieldPlacement>)
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
}

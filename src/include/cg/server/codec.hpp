#pragma once

#include <algorithm>
#include <expected>

#include <cg/utility/enum.hpp>
#include <cg/utility/variant.hpp>

#include <cg/driver/request.hpp>
#include <cg/driver/response.hpp>

#include <nlohmann/json.hpp>

namespace cg::server
{
	template<typename T>
	class ValueCodec;

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
			return std::ranges::all_of(json, &ValueCodec<T>::valid);
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

	template<typename T>
	// domain::Location / domain::FieldLocation / domain::AnyMonsterLocation
	// domain::Placement / domain::FieldPlacement / domain::AnyMonsterPlacement
	// driver::Action
	// driver::Request
	// 处理方式完全相同,一并处理
		requires(
			// Location
			std::is_same_v<T, domain::Location> ||
			std::is_same_v<T, domain::FieldLocation> ||
			std::is_same_v<T, domain::AnyMonsterLocation> ||
			// Placement
			std::is_same_v<T, domain::Placement> ||
			std::is_same_v<T, domain::FieldPlacement> ||
			std::is_same_v<T, domain::AnyMonsterPlacement> ||
			// Action
			std::is_same_v<T, driver::Action> ||
			// Request
			std::is_same_v<T, driver::Request>
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

			// kind
			if (!json.contains("variant_kind") || !json.at("variant_kind").is_number_unsigned())
			{
				return false;
			}

			// variant?
			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<T>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto kind_it = json.find("variant_kind");
			const auto kind = kind_it.value().get<nlohmann::json::number_unsigned_t>();

			// 下标超出变体数量,直接失败
			if (kind >= utility::variant_size_v<T>)
			{
				return std::nullopt;
			}

			std::optional<T> result;
			[&]<std::size_t... Index>(std::index_sequence<Index...>) noexcept -> void
			{
				const auto process = [&]<std::size_t I>() noexcept -> bool
				{
					if (I != kind)
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

			object["variant_kind"] = any.index();

			return object;
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

			const auto column_it = json.find("index");
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

	template<>
	class ValueCodec<driver::AdvancePhaseAction>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_object();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::AdvancePhaseAction>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			return driver::AdvancePhaseAction{};
		}

		[[nodiscard]] static auto encode([[maybe_unused]] const driver::AdvancePhaseAction& data) noexcept -> nlohmann::json
		{
			return nlohmann::json::object();
		}
	};

	template<>
	class ValueCodec<driver::NormalSummonAction>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::NormalSummonAction>
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

			return driver::NormalSummonAction{.card = *card, .placement = *placement};
		}

		[[nodiscard]] static auto encode(const driver::NormalSummonAction& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["placement"] = ValueCodec<domain::MonsterPlacement>::encode(data.placement);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::AdvanceSummonAction>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::AdvanceSummonAction>
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

			return driver::AdvanceSummonAction{.card = *card, .materials = *std::move(materials), .placement = *placement};
		}

		[[nodiscard]] static auto encode(const driver::AdvanceSummonAction& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["materials"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.materials);
			object["placement"] = ValueCodec<domain::MonsterPlacement>::encode(data.placement);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::FlipSummonAction>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::FlipSummonAction>
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

			return driver::FlipSummonAction{.card = *card};
		}

		[[nodiscard]] static auto encode(const driver::FlipSummonAction& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::DualSummonAction>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::DualSummonAction>
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

			return driver::DualSummonAction{.card = *card};
		}

		[[nodiscard]] static auto encode(const driver::DualSummonAction& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::ExtraDeckSummonAction>
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
			if (!json.contains("method") || !json.at("method").is_string())
			{
				return false;
			}

			// material_source
			if (!json.contains("material_source") || !json.at("material_source").is_string())
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::ExtraDeckSummonAction>
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
			const auto& method_string = method_it.value().get_ref<const nlohmann::json::string_t&>();
			const auto method = utility::from_string<domain::SummonMethod>(method_string);
			if (!method.has_value())
			{
				return std::nullopt;
			}
			const auto material_source_it = json.find("material_source");
			const auto& material_source_string = material_source_it.value().get_ref<const nlohmann::json::string_t&>();
			const auto material_source = utility::from_mask_string<domain::SummonMaterialSource>(material_source_string);
			if (!material_source.has_value())
			{
				return std::nullopt;
			}
			const auto placement_it = json.find("placement");
			const auto placement = ValueCodec<domain::AnyMonsterPlacement>::decode(placement_it.value());
			if (!placement.has_value())
			{
				return std::nullopt;
			}

			return driver::ExtraDeckSummonAction{.card = *card, .method = *method, .material_source = *material_source, .placement = *placement};
		}

		[[nodiscard]] static auto encode(const driver::ExtraDeckSummonAction& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["method"] = utility::to_string(data.method);
			object["material_source"] = utility::to_string(data.material_source);
			object["placement"] = ValueCodec<domain::AnyMonsterPlacement>::encode(data.placement);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::SpecialSummonAction>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::SpecialSummonAction>
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

			return driver::SpecialSummonAction{.card = *card, .placement = *placement, .controller = *controller};
		}

		[[nodiscard]] static auto encode(const driver::SpecialSummonAction& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["placement"] = ValueCodec<domain::MonsterPlacement>::encode(data.placement);
			object["controller"] = ValueCodec<domain::Player>::encode(data.controller);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::PendulumSummonAction>
	{
	public:
		[[nodiscard]] static auto valid(const nlohmann::json& json) noexcept -> bool
		{
			return json.is_object();
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::PendulumSummonAction>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			// TODO

			return driver::PendulumSummonAction{};
		}

		[[nodiscard]] static auto encode([[maybe_unused]] const driver::PendulumSummonAction& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			// todo

			return object;
		}
	};

	template<>
	class ValueCodec<driver::AttackDeclareAction>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::AttackDeclareAction>
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
				return driver::AttackDeclareAction{.attacker = *attacker, .target = std::nullopt};
			}
			const auto target = ValueCodec<state::CardInstanceId>::decode(target_it.value());
			if (!target.has_value())
			{
				return std::nullopt;
			}

			return driver::AttackDeclareAction{.attacker = *attacker, .target = *target};
		}

		[[nodiscard]] static auto encode(const driver::AttackDeclareAction& data) noexcept -> nlohmann::json
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
	class ValueCodec<driver::ActiveEffectAction>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::ActiveEffectAction>
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
			if (effect.has_value())
			{
				return std::nullopt;
			}

			return driver::ActiveEffectAction{.card = *card, .effect = *effect};
		}

		[[nodiscard]] static auto encode(const driver::ActiveEffectAction& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["effect"] = ValueCodec<state::EffectId>::encode(data.effect);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::ActivatePendulumAction>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::ActivatePendulumAction>
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

			return driver::ActivatePendulumAction{.card = *card, .side = *side};
		}

		[[nodiscard]] static auto encode(const driver::ActivatePendulumAction& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["side"] = ValueCodec<domain::PendulumSide>::encode(data.side);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::SetSpellTrapAction>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::SetSpellTrapAction>
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

			return driver::SetSpellTrapAction{.card = *card, .location = *location};
		}

		[[nodiscard]] static auto encode(const driver::SetSpellTrapAction& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["card"] = ValueCodec<state::CardInstanceId>::encode(data.card);
			object["location"] = ValueCodec<domain::SpellTrapLocation>::encode(data.location);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::IdleCommandRequest>
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

			// actions
			// if (!json.contains("actions") || !ValueCodec<std::vector<driver::Action>>::valid(json.at("actions")))
			if (!json.contains("actions"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::IdleCommandRequest>
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
			const auto actions_it = json.find("actions");
			auto actions = ValueCodec<std::vector<driver::Action>>::decode(actions_it.value());
			if (!actions.has_value())
			{
				return std::nullopt;
			}

			return driver::IdleCommandRequest{.player = *player, .actions = *std::move(actions)};
		}

		[[nodiscard]] static auto encode(const driver::IdleCommandRequest& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["actions"] = ValueCodec<std::vector<driver::Action>>::encode(data.actions);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::SelectCardRequest>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::SelectCardRequest>
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

			return driver::SelectCardRequest{.player = *player, .candidates = *std::move(candidates), .min = min, .max = max};
		}

		[[nodiscard]] static auto encode(const driver::SelectCardRequest& data) noexcept -> nlohmann::json
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
	class ValueCodec<driver::SelectYesNoRequest>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::SelectYesNoRequest>
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

			return driver::SelectYesNoRequest{.player = *player};
		}

		[[nodiscard]] static auto encode(const driver::SelectYesNoRequest& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::SelectNumberRequest>
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

			// min
			if (!json.contains("min") || !json.at("min").is_number_integer())
			{
				return false;
			}

			// max
			if (!json.contains("max") || !json.at("max").is_number_integer())
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::SelectNumberRequest>
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
			const auto min_it = json.find("min");
			const auto min = min_it.value().get<driver::SelectNumberRequest::value_type>();
			const auto max_it = json.find("max");
			const auto max = max_it.value().get<driver::SelectNumberRequest::value_type>();

			return driver::SelectNumberRequest{.player = *player, .min = min, .max = max};
		}

		[[nodiscard]] static auto encode(const driver::SelectNumberRequest& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["min"] = data.min;
			object["max"] = data.max;

			return object;
		}
	};

	template<>
	class ValueCodec<driver::SelectCardNameRequest>
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
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::SelectCardNameRequest>
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
			std::vector<std::string> candidates;
			try
			{
				candidates = candidates_it.value().get<std::vector<std::string>>();
			}
			catch ([[maybe_unused]] const nlohmann::json::parse_error& error)
			{
				return std::nullopt;
			}

			return driver::SelectCardNameRequest{.player = *player, .candidates = std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::SelectCardNameRequest& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["candidates"] = data.candidates;

			return object;
		}
	};

	template<>
	class ValueCodec<driver::SelectMonsterLocationRequest>
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
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::MonsterLocation>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::SelectMonsterLocationRequest>
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
			auto candidates = ValueCodec<std::vector<domain::MonsterLocation>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::SelectMonsterLocationRequest{.player = *player, .candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::SelectMonsterLocationRequest& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["candidates"] = ValueCodec<std::vector<domain::MonsterLocation>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::SelectMonsterLocationExRequest>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::SelectMonsterLocationExRequest>
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

			return driver::SelectMonsterLocationExRequest{.player = *player, .candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::SelectMonsterLocationExRequest& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["candidates"] = ValueCodec<std::vector<domain::AnyMonsterLocation>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::SelectMonsterPositionRequest>
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
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::Position>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::SelectMonsterPositionRequest>
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
			auto candidates = ValueCodec<std::vector<domain::Position>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::SelectMonsterPositionRequest{.player = *player, .candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::SelectMonsterPositionRequest& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["candidates"] = ValueCodec<std::vector<domain::Position>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::SelectSpellTrapLocationRequest>
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
			// if (!json.contains("candidates") || !ValueCodec<std::vector<domain::SpellTrapLocation>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::SelectSpellTrapLocationRequest>
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
			auto candidates = ValueCodec<std::vector<domain::SpellTrapLocation>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::SelectSpellTrapLocationRequest{.player = *player, .candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::SelectSpellTrapLocationRequest& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["candidates"] = ValueCodec<std::vector<domain::SpellTrapLocation>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::SelectActivateEffectRequest>
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
			// if (!json.contains("candidates") || !ValueCodec<std::vector<state::EffectId>>::valid(json.at("candidates")))
			if (!json.contains("candidates"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::SelectActivateEffectRequest>
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
			auto candidates = ValueCodec<std::vector<state::EffectId>>::decode(candidates_it.value());
			if (!candidates.has_value())
			{
				return std::nullopt;
			}

			return driver::SelectActivateEffectRequest{.player = *player, .candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::SelectActivateEffectRequest& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["player"] = ValueCodec<domain::Player>::encode(data.player);
			object["candidates"] = ValueCodec<std::vector<state::EffectId>>::encode(data.candidates);

			return object;
		}
	};

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

	[[nodiscard]] auto encode_request(const driver::Request& request) noexcept -> nlohmann::json;

	[[nodiscard]] auto decode_response(const driver::Request& request, const nlohmann::json& message) noexcept -> std::expected<driver::Response, std::string>;
}

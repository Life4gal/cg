#pragma once

#include <cg/driver/action.hpp>

#include <cg/codec/domain.hpp>
#include <cg/codec/state.hpp>

namespace cg::codec
{
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
	class ValueCodec<driver::action::TributeSummon>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::TributeSummon>
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

			return driver::action::TributeSummon{.card = *card, .materials = *std::move(materials), .placement = *placement};
		}

		[[nodiscard]] static auto encode(const driver::action::TributeSummon& data) noexcept -> nlohmann::json
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
	class ValueCodec<driver::action::GeminiSummon>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::action::GeminiSummon>
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

			return driver::action::GeminiSummon{.card = *card};
		}

		[[nodiscard]] static auto encode(const driver::action::GeminiSummon& data) noexcept -> nlohmann::json
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

			if (!ValueCodec<VariantWrap<driver::action::Action>>::valid(json))
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
				if (type == "tribute_summon")
				{
					return do_decode.operator()<driver::action::TributeSummon>();
				}
				if (type == "flip_summon")
				{
					return do_decode.operator()<driver::action::FlipSummon>();
				}
				if (type == "gemini_summon")
				{
					return do_decode.operator()<driver::action::GeminiSummon>();
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
			return ValueCodec<VariantWrap<driver::action::Action>>::decode(json);
		}

		[[nodiscard]] static auto encode(const driver::action::Action& data) noexcept -> nlohmann::json
		{
			auto object = ValueCodec<VariantWrap<driver::action::Action>>::encode(data);

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
				case utility::variant_index_v<driver::action::TributeSummon, driver::action::Action>:
				{
					object["type"] = "tribute_summon";
					break;
				}
				case utility::variant_index_v<driver::action::FlipSummon, driver::action::Action>:
				{
					object["type"] = "flip_summon";
					break;
				}
				case utility::variant_index_v<driver::action::GeminiSummon, driver::action::Action>:
				{
					object["type"] = "gemini_summon";
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
}

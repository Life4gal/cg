#pragma once

#include <cg/driver/response.hpp>

#include <cg/codec/action.hpp>

namespace cg::codec
{
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
	class ValueCodec<driver::response::QueryTributeSummonableCards>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryTributeSummonableCards>
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

			return driver::response::QueryTributeSummonableCards{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryTributeSummonableCards& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryTributeSummonTributeCandidates>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryTributeSummonTributeCandidates>
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

			return driver::response::QueryTributeSummonTributeCandidates{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryTributeSummonTributeCandidates& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<state::CardInstanceId>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::QueryTributeSummonType>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryTributeSummonType>
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

			return driver::response::QueryTributeSummonType{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryTributeSummonType& data) noexcept -> nlohmann::json
		{
			auto object = nlohmann::json::object();

			object["candidates"] = ValueCodec<std::vector<domain::NormalSummonType>>::encode(data.candidates);

			return object;
		}
	};

	template<>
	class ValueCodec<driver::response::CheckTributeSummonType>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::CheckTributeSummonType>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::CheckTributeSummonType{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::CheckTributeSummonType{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::CheckTributeSummonType& data) noexcept -> nlohmann::json
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
	class ValueCodec<driver::response::SelectTributeSummonTributes>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::SelectTributeSummonTributes>
		{
			if (!valid(json))
			{
				return std::nullopt;
			}

			const auto reject_it = json.find("reject");
			if (reject_it == json.end())
			{
				return driver::response::SelectTributeSummonTributes{.reject = std::nullopt};
			}
			const auto reject = ValueCodec<domain::RejectReason>::decode(reject_it.value());
			if (!reject.has_value())
			{
				return std::nullopt;
			}

			return driver::response::SelectTributeSummonTributes{.reject = *reject};
		}

		[[nodiscard]] static auto encode(const driver::response::SelectTributeSummonTributes& data) noexcept -> nlohmann::json
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
	class ValueCodec<driver::response::QueryTributeSummonLocation>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryTributeSummonLocation>
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

			return driver::response::QueryTributeSummonLocation{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryTributeSummonLocation& data) noexcept -> nlohmann::json
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
	class ValueCodec<driver::response::QueryGeminiSummonableCards>
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

		[[nodiscard]] static auto decode(const nlohmann::json& json) noexcept -> std::optional<driver::response::QueryGeminiSummonableCards>
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

			return driver::response::QueryGeminiSummonableCards{.candidates = *std::move(candidates)};
		}

		[[nodiscard]] static auto encode(const driver::response::QueryGeminiSummonableCards& data) noexcept -> nlohmann::json
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
			std::map<state::CardInstanceId, std::vector<dsl::EffectId>> candidates;
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
				auto effects = ValueCodec<std::vector<dsl::EffectId>>::decode(value);
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

				candidates[std::move(key)] = ValueCodec<std::vector<dsl::EffectId>>::encode(effects);
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
			std::map<state::CardInstanceId, std::vector<dsl::EffectId>> candidates;
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
				auto effects = ValueCodec<std::vector<dsl::EffectId>>::decode(value);
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

				candidates[std::move(key)] = ValueCodec<std::vector<dsl::EffectId>>::encode(effects);
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

			if (!ValueCodec<VariantWrap<driver::response::Response>>::valid(json))
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
				if (type == "query_tribute_summonable_cards")
				{
					return do_decode.operator()<driver::response::QueryTributeSummonableCards>();
				}
				if (type == "query_tribute_summon_tribute_candidates")
				{
					return do_decode.operator()<driver::response::QueryTributeSummonTributeCandidates>();
				}
				if (type == "query_tribute_summon_type")
				{
					return do_decode.operator()<driver::response::QueryTributeSummonType>();
				}
				if (type == "check_tribute_summon_type")
				{
					return do_decode.operator()<driver::response::CheckTributeSummonType>();
				}
				if (type == "select_tribute_summon_tributes")
				{
					return do_decode.operator()<driver::response::SelectTributeSummonTributes>();
				}
				if (type == "query_tribute_summon_location")
				{
					return do_decode.operator()<driver::response::QueryTributeSummonLocation>();
				}
				if (type == "query_flip_summonable_cards")
				{
					return do_decode.operator()<driver::response::QueryFlipSummonableCards>();
				}
				if (type == "query_gemini_summonable_cards")
				{
					return do_decode.operator()<driver::response::QueryGeminiSummonableCards>();
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
			return ValueCodec<VariantWrap<driver::response::Response>>::decode(json);
		}

		[[nodiscard]] static auto encode(const driver::response::Response& data) noexcept -> nlohmann::json
		{
			auto object = ValueCodec<VariantWrap<driver::response::Response>>::encode(data);

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
				case utility::variant_index_v<driver::response::QueryTributeSummonableCards, driver::response::Response>:
				{
					object["type"] = "query_tribute_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::response::QueryTributeSummonTributeCandidates, driver::response::Response>:
				{
					object["type"] = "query_tribute_summon_tribute_candidates";
					break;
				}
				case utility::variant_index_v<driver::response::QueryTributeSummonType, driver::response::Response>:
				{
					object["type"] = "query_tribute_summon_type";
					break;
				}
				case utility::variant_index_v<driver::response::CheckTributeSummonType, driver::response::Response>:
				{
					object["type"] = "check_tribute_summon_type";
					break;
				}
				case utility::variant_index_v<driver::response::SelectTributeSummonTributes, driver::response::Response>:
				{
					object["type"] = "select_tribute_summon_tributes";
					break;
				}
				case utility::variant_index_v<driver::response::QueryTributeSummonLocation, driver::response::Response>:
				{
					object["type"] = "query_tribute_summon_location";
					break;
				}
				case utility::variant_index_v<driver::response::QueryFlipSummonableCards, driver::response::Response>:
				{
					object["type"] = "query_flip_summonable_cards";
					break;
				}
				case utility::variant_index_v<driver::response::QueryGeminiSummonableCards, driver::response::Response>:
				{
					object["type"] = "query_gemini_summonable_cards";
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
}

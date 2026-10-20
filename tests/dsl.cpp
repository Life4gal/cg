#include <fstream>

#include <cg/dsl/script.hpp>
#include <cg/dsl/types.hpp>
#include <cg/dsl/vocabulary.hpp>
#include <cg/dsl/json/reader.hpp>

#include <nlohmann/json.hpp>
#include <doctest/doctest.h>

namespace
{
	using namespace cg;

	class ReadResult
	{
	public:
		std::optional<dsl::CardScript> script;
		dsl::Diagnostics diagnostics;
	};

	[[nodiscard]] auto read_card(const nlohmann::json& json) -> ReadResult
	{
		const dsl::TypeRegistry types{};
		const dsl::Vocabulary vocabulary{types};
		dsl::json::Reader reader{types, vocabulary};

		auto script = reader.read_card_script(json, "<test>");

		return {.script = std::move(script), .diagnostics = reader.diagnostics()};
	}

	[[nodiscard]] auto make_card(const std::string_view condition, const std::string_view effect) -> nlohmann::json
	{
		const auto text = std::format(
			R"({{"identity":{{"kind":"SPELL","subtypes":["NORMAL"]}},"effects":[{{"type":"activated","activation":{{"mode":"activation"}},"condition":{},"effect":{}}}]}})",
			condition,
			effect
		);

		return nlohmann::json::parse(text);
	}

	[[nodiscard]] auto filter_with(const std::string_view filter) -> std::string
	{
		return std::format(R"({{"call":"exists_matching","args":{{"players":["OPPONENT"],"zones":["MONSTER"],"min":1,"filter":{}}}}})", filter);
	}

	[[nodiscard]] auto contains_message(const dsl::Diagnostics& diagnostics, const std::string_view text) -> bool
	{
		return std::ranges::any_of(
			diagnostics,
			[&](const dsl::Diagnostic& diagnostic) noexcept -> bool
			{
				return diagnostic.message.find(text) != std::string::npos;
			}
		);
	}

	constexpr std::string_view default_effect{R"([{"call":"destroy","args":{"cards":{"call":"find_cards","args":{"players":["OPPONENT"],"zones":["MONSTER"]}},"reason":"EFFECT"}}])"};
}

TEST_CASE("dsl::Vocabulary is consistent")
{
	const auto types = dsl::TypeRegistry{};
	const auto vocabulary = dsl::Vocabulary{types};

	CHECK_FALSE(vocabulary.empty());
	CHECK(vocabulary.validate(types).empty());
}

TEST_CASE("dsl::json::Reader reads bundled cards")
{
	const auto directory = std::filesystem::path{CG_CARDS_DIR};
	REQUIRE(std::filesystem::is_directory(directory));

	auto count = 0;
	for (const auto& entry: std::filesystem::directory_iterator{directory})
	{
		if (!entry.is_regular_file() || entry.path().extension() != ".json")
		{
			continue;
		}

		count += 1;

		std::ifstream stream{entry.path(), std::ios::binary};
		REQUIRE(stream.good());

		std::stringstream buffer{};
		buffer << stream.rdbuf();

		const auto json = nlohmann::json::parse(buffer.str());
		const auto result = read_card(json);

		INFO(entry.path().filename().string());
		for (const auto& [path, message]: result.diagnostics)
		{
			INFO(path << ": " << message);
		}

		CHECK(result.script.has_value());
	}

	CHECK(count > 0);
}

TEST_CASE("dsl::json::Reader enforces element scope")
{
	SUBCASE("`$` outside a scoped parameter")
	{
		const auto card = make_card(R"({"call":"is_card_type","args":{"card":"$","card_type":"SPELL"}})", default_effect);
		const auto result = read_card(card);

		CHECK_FALSE(result.script.has_value());
		CHECK(contains_message(result.diagnostics, "only valid inside a scoped parameter"));
	}

	SUBCASE("`$` inside a filter")
	{
		const auto card = make_card(filter_with(R"({"call":"is_card_type","args":{"card":"$","card_type":"SPELL"}})"), default_effect);
		const auto result = read_card(card);

		CHECK(result.script.has_value());
	}

	SUBCASE("`$` with a mismatched type")
	{
		const auto card = make_card(filter_with(R"({"call":"is_card_type","args":{"card":"$","card_type":"$"}})"), default_effect);
		const auto result = read_card(card);

		CHECK_FALSE(result.script.has_value());
		CHECK(contains_message(result.diagnostics, "candidate of type `card`, but `card_type` is expected"));
	}

	SUBCASE("`$` as the filter itself")
	{
		const auto card = make_card(filter_with(R"("$")"), default_effect);
		const auto result = read_card(card);

		CHECK_FALSE(result.script.has_value());
		CHECK(contains_message(result.diagnostics, "candidate of type `card`, but `bool` is expected"));
	}

	SUBCASE("nested predicates bind innermost")
	{
		const auto card = make_card(
			filter_with(R"({"call":"exists_matching","args":{"players":["OPPONENT"],"zones":["MONSTER"],"min":1,"filter":{"call":"is_card_type","args":{"card":"$","card_type":"MONSTER"}}}})"),
			default_effect
		);
		const auto result = read_card(card);

		CHECK(result.script.has_value());
	}
}

TEST_CASE("dsl::speed_of matches the official classification")
{
	using domain::CardType;
	using domain::CardSubtype;
	using domain::EffectSpeed;
	using dsl::ActivationKind;
	using dsl::CardActivation;
	using dsl::Ignition;
	using dsl::Quick;
	using dsl::Trigger;

	CHECK(dsl::speed_of(CardType::MONSTER, {}, ActivationKind{Ignition{}}) == EffectSpeed::NORMAL);
	CHECK(dsl::speed_of(CardType::MONSTER, {}, ActivationKind{Trigger{}}) == EffectSpeed::NORMAL);
	CHECK(dsl::speed_of(CardType::MONSTER, {}, ActivationKind{Quick{}}) == EffectSpeed::QUICK);
	CHECK(dsl::speed_of(CardType::MONSTER, {}, ActivationKind{CardActivation{}}) == EffectSpeed::NONE);

	CHECK(dsl::speed_of(CardType::SPELL, {}, ActivationKind{CardActivation{}}) == EffectSpeed::NORMAL);
	CHECK(dsl::speed_of(CardType::SPELL, {CardSubtype::QUICK_PLAY}, ActivationKind{CardActivation{}}) == EffectSpeed::QUICK);

	CHECK(dsl::speed_of(CardType::TRAP, {}, ActivationKind{CardActivation{}}) == EffectSpeed::QUICK);
	CHECK(dsl::speed_of(CardType::TRAP, {CardSubtype::COUNTER}, ActivationKind{CardActivation{}}) == EffectSpeed::COUNTER);

	CHECK(domain::is_fast_effect(EffectSpeed::QUICK));
	CHECK(domain::is_fast_effect(EffectSpeed::COUNTER));
	CHECK_FALSE(domain::is_fast_effect(EffectSpeed::NORMAL));
	CHECK_FALSE(domain::is_fast_effect(EffectSpeed::NONE));
}

TEST_CASE("dsl::json::Reader enforces statement phases")
{
	const auto make_activated = [](const std::string_view cost, const std::string_view target, const std::string_view effect) noexcept -> nlohmann::json
	{
		const auto card = std::format(
			R"({{"identity":{{"kind":"SPELL","subtypes":["NORMAL"]}},"effects":[{{"type":"activated","activation":{{"mode":"activation"}},"cost":{},"target":{},"effect":{}}}]}})",
			cost,
			target,
			effect
		);

		return nlohmann::json::parse(card);
	};

	constexpr std::string_view destroy_call{R"([{"call":"destroy","args":{"cards":{"call":"find_cards","args":{"players":["OPPONENT"],"zones":["MONSTER"]}},"reason":"EFFECT"}}])"};
	constexpr std::string_view target_call{R"([{"call":"pick_target","args":{"players":["OPPONENT"],"zones":["MONSTER"],"min":1,"max":1}}])"};

	SUBCASE("resolve action in cost")
	{
		const auto result = read_card(make_activated(destroy_call, "[]", "[]"));

		CHECK_FALSE(result.script.has_value());
		CHECK(contains_message(result.diagnostics, "not usable in the `COST` phase"));
	}

	SUBCASE("targeting in cost")
	{
		const auto result = read_card(make_activated(target_call, "[]", "[]"));

		CHECK_FALSE(result.script.has_value());
		CHECK(contains_message(result.diagnostics, "not usable in the `COST` phase"));
	}

	SUBCASE("resolve action in target")
	{
		const auto result = read_card(make_activated("[]", destroy_call, "[]"));

		CHECK_FALSE(result.script.has_value());
		CHECK(contains_message(result.diagnostics, "not usable in the `TARGET` phase"));
	}

	SUBCASE("targeting in effect")
	{
		const auto result = read_card(make_activated("[]", "[]", target_call));

		CHECK_FALSE(result.script.has_value());
		CHECK(contains_message(result.diagnostics, "not usable in the `RESOLVE` phase"));
	}

	SUBCASE("resolve action in effect")
	{
		const auto result = read_card(make_activated("[]", "[]", destroy_call));

		CHECK(result.script.has_value());
	}
}

TEST_CASE("dsl::json::Reader reads player scopes")
{
	SUBCASE("both players' monster zones")
	{
		const auto card = make_card(R"({"call":"exists_matching","args":{"players":["SELF","OPPONENT"],"zones":["MONSTER","EXTRA_MONSTER"],"min":1}})", default_effect);
		const auto result = read_card(card);

		CHECK(result.script.has_value());
	}

	SUBCASE("old scalar player key is rejected")
	{
		const auto card = make_card(R"({"call":"exists_matching","args":{"player":"OPPONENT","zones":["MONSTER"],"min":1}})", default_effect);
		const auto result = read_card(card);

		CHECK_FALSE(result.script.has_value());
		CHECK(contains_message(result.diagnostics, "unknown parameter `player`"));
	}

	SUBCASE("pick_target accepts a scope and a filter")
	{
		const auto card =
				R"({"identity":{"kind":"SPELL","subtypes":["NORMAL"]},"effects":[{"type":"activated","activation":{"mode":"activation"},"target":[{"call":"pick_target","args":{"players":["SELF","OPPONENT"],"zones":["MONSTER"],"min":1,"max":1,"filter":{"call":"is_card_type","args":{"card":"$","card_type":"MONSTER"}}},"bind":"target"}],"effect":[]}]})";
		const auto result = read_card(nlohmann::json::parse(card));

		CHECK(result.script.has_value());
	}
}

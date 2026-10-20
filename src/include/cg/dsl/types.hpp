#pragma once

#include <vector>
#include <optional>

#include <cg/utility/enum.hpp>

#include <cg/dsl/id.hpp>

namespace cg::dsl
{
	// 值类别
	enum class ValueClass : std::uint8_t
	{
		// ====================
		// 基础类别
		// ====================
		VOID,
		BOOL,
		INT,
		CARD,
		GROUP,

		// ====================
		// 开放名域(如系列/字段名)
		// ====================
		SYMBOL,

		// ====================
		// 枚举值(domain中的定义)
		// ====================
		ENUM,
		ENUM_SET,
	};

	class TypeRegistry
	{
	public:
		using symbol_underlying_type = std::uint64_t;

		using symbol_resolver_type = auto(*)(std::string_view name) noexcept -> std::optional<symbol_underlying_type>;
		using symbol_name_resolver_type = auto(*)(symbol_underlying_type symbol) noexcept -> std::string_view;

		class SymbolResolver
		{
		public:
			symbol_resolver_type symbol;
			symbol_name_resolver_type symbol_name;
		};

		constexpr static SymbolResolver none_symbol_resolver{.symbol = nullptr, .symbol_name = nullptr};

		template<typename Symbol, typename SymbolName>
			requires(
				std::is_invocable_r_v<std::optional<symbol_underlying_type>, Symbol, std::string_view> &&
				std::is_invocable_r_v<std::string_view, SymbolName, symbol_underlying_type>
			)
		[[nodiscard]] static auto make_resolver(Symbol&& symbol_resolver, SymbolName&& symbol_name_resolver) noexcept -> SymbolResolver
		{
			return {.symbol = std::forward<Symbol>(symbol_resolver), .symbol_name = std::forward<SymbolName>(symbol_name_resolver)};
		}

		template<auto Symbol, auto SymbolName>
			requires requires
			{
				{ Symbol(std::string_view{}) } -> std::same_as<std::optional<symbol_underlying_type>>;
				{ SymbolName(symbol_underlying_type{}) } -> std::same_as<std::string_view>;
			}
		[[nodiscard]] static auto make_resolver() noexcept -> SymbolResolver
		{
			return TypeRegistry::make_resolver(Symbol, SymbolName);
		}

		template<typename EnumType>
			requires std::is_scoped_enum_v<EnumType>
		[[nodiscard]] static auto make_resolver() noexcept -> SymbolResolver
		{
			const auto symbol_resolver = [](const std::string_view name) noexcept -> std::optional<symbol_underlying_type>
			{
				// FIXME(OPT): 支持纠错,例如"enum"=>"ENUM"
				const auto symbol = utility::from_string<EnumType>(name);
				if (!symbol.has_value())
				{
					return std::nullopt;
				}

				return std::to_underlying(*symbol);
			};
			const auto symbol_name_resolver = [](const symbol_underlying_type symbol) noexcept -> std::string_view
			{
				const auto v = static_cast<std::underlying_type_t<EnumType>>(symbol);
				const auto e = static_cast<EnumType>(v);
				return utility::to_string(e);
			};

			return TypeRegistry::make_resolver(symbol_resolver, symbol_name_resolver);
		}

		class Def
		{
		public:
			TypeId id;
			std::string_view name;
			ValueClass value_class;
			SymbolResolver symbol_resolver;
		};

	private:
		std::vector<Def> types_;

		[[nodiscard]] auto generate_id() const noexcept -> TypeId;

		auto do_register(std::string_view name, ValueClass value_class, SymbolResolver symbol_resolver) noexcept -> void;

	public:
		TypeRegistry() noexcept;

		[[nodiscard]] auto contains(TypeId id) const noexcept -> bool;
		[[nodiscard]] auto contains(std::string_view name) const noexcept -> bool;

		[[nodiscard]] auto find(TypeId id) const noexcept -> const Def*;
		[[nodiscard]] auto find(std::string_view name) const noexcept -> const Def*;
	};
}

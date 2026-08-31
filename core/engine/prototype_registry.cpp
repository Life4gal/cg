#include <core/engine/prototype_registry.hpp>

#include <unordered_map>

namespace
{
	using namespace cg;

	std::unordered_map<domain::CardCode, engine::Prototype> g_registered_prototype;
}

namespace cg::engine
{
	auto PrototypeRegistry::register_prototype(const Prototype& prototype) noexcept -> void
	{
		g_registered_prototype[prototype.code] = prototype;
	}

	auto PrototypeRegistry::registered(const domain::CardCode code) noexcept -> bool
	{
		return g_registered_prototype.contains(code);
	}

	auto PrototypeRegistry::find_prototype(const domain::CardCode code) noexcept -> const Prototype*
	{
		const auto it = g_registered_prototype.find(code);

		if (it != g_registered_prototype.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	auto PrototypeRegistry::size() noexcept -> std::size_t
	{
		return g_registered_prototype.size();
	}

	auto PrototypeRegistry::clear() noexcept -> void
	{
		g_registered_prototype.clear();
	}
}

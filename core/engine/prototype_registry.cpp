#include <core/engine/prototype_registry.hpp>

namespace cg::engine
{
	auto PrototypeRegistry::register_prototype(const Prototype& prototype) noexcept -> const Prototype&
	{
		auto [it, inserted] = registered_prototype_.insert_or_assign(prototype.code(), prototype);

		return it->second;
	}

	auto PrototypeRegistry::registered(const domain::CardCode code) const noexcept -> bool
	{
		return registered_prototype_.contains(code);
	}

	auto PrototypeRegistry::find_prototype(const domain::CardCode code) const noexcept -> const Prototype*
	{
		const auto it = registered_prototype_.find(code);

		if (it != registered_prototype_.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	auto PrototypeRegistry::size() const noexcept -> std::size_t
	{
		return registered_prototype_.size();
	}

	auto PrototypeRegistry::clear() noexcept -> void
	{
		registered_prototype_.clear();
	}
}

#include <cg/dsl/archetype.hpp>

#include <cg/utility/hash.hpp>

namespace cg::dsl
{
	std::unordered_map<std::uint64_t, std::string> ArchetypeRegistry::registry_ = {};

	auto ArchetypeRegistry::do_hash(const std::string_view text) noexcept -> std::uint64_t
	{
		return utility::hash<std::string_view>::operator()(text);
	}

	auto ArchetypeRegistry::id_of(const std::string_view text) noexcept -> ArchetypeId
	{
		const auto hash = do_hash(text);

		if (!registry_.contains(hash))
		{
			registry_.emplace(hash, std::string{text});
		}

		return ArchetypeId{hash};
	}

	auto ArchetypeRegistry::name_of(const ArchetypeId id) noexcept -> std::string_view
	{
		const auto it = registry_.find(id.raw());
		if (it == registry_.end())
		{
			return {};
		}

		return it->second;
	}
}

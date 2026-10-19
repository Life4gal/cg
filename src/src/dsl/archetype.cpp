#include <cg/dsl/archetype.hpp>

namespace cg::dsl
{
	std::unordered_map<std::uint64_t, std::string> ArchetypeRegistry::registry_ = {};

	auto ArchetypeRegistry::do_hash(const std::string_view text) noexcept -> std::uint64_t
	{
		constexpr auto offset_basis = std::uint64_t{0xcbf29ce484222325ull};
		constexpr auto prime = std::uint64_t{0x100000001b3ull};

		auto hash = offset_basis;
		for (const auto c: text)
		{
			const auto byte = static_cast<unsigned char>(c);
			hash ^= byte;
			hash *= prime;
		}

		return hash;
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

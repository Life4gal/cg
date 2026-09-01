#include <core/script/host.hpp>

namespace cg::script
{
	auto Host::initialize(const std::string_view script_root) noexcept -> bool
	{
		//

		const auto result = do_initialize(script_root);

		//

		return result;
	}

	auto Host::bind_api(engine::Api& api) noexcept -> void
	{
		//

		do_bind_api(api);

		//
	}

	auto Host::load_prototype(const domain::CardCode code) noexcept -> std::optional<engine::Prototype>
	{
		//

		auto prototype = do_load_prototype(code);

		//

		return prototype;
	}
}

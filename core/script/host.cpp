#include <core/script/host.hpp>

namespace cg::script
{
	auto Host::bind_api(engine::Api& api) noexcept -> void
	{
		api_ = &api;
	}

	auto Host::load_prototype(const domain::CardCode code) noexcept -> std::optional<engine::Prototype>
	{
		//

		auto prototype = do_load_prototype(code);

		//

		return prototype;
	}
}

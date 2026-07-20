#include <core/bridge.hpp>

namespace cg::core
{
	Bridge::Bridge() noexcept = default;

	Bridge::Bridge(Bridge&&) noexcept = default;

	auto Bridge::operator=(Bridge&&) noexcept -> Bridge& = default;

	Bridge::~Bridge() noexcept = default;

	auto Bridge::load_script(const std::filesystem::path& script_path) noexcept -> bool
	{
		//

		return do_load_script(script_path);
	}

	auto Bridge::register_card(const Card& card) noexcept -> bool
	{
		//

		return do_register_card(card);
	}

	auto Bridge::unregister_card(const Card& card) noexcept -> bool
	{
		//

		return do_unregister_card(card);
	}

	auto Bridge::register_view(const View& view) noexcept -> bool
	{
		//

		return do_register_view(view);
	}

	auto Bridge::unregister_view(const View& view) noexcept -> bool
	{
		//

		return do_unregister_view(view);
	}

	auto Bridge::register_effect(const Effect& effect) noexcept -> bool
	{
		//

		return do_register_effect(effect);
	}

	auto Bridge::unregister_effect(const Effect& effect) noexcept -> bool
	{
		//

		return do_unregister_effect(effect);
	}
}

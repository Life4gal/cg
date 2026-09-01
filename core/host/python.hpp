#pragma once

#include <string_view>

#include <core/script/host.hpp>

namespace cg::host
{
	class PythonHost final : public script::Host
	{
	public:
		//

	private:
		auto do_initialize(std::string_view script_root) noexcept -> bool override;
		auto do_bind_api(engine::Api& api) noexcept -> void override;
		auto do_load_prototype(domain::CardCode code) noexcept -> std::optional<engine::Prototype> override;

	public:
		explicit PythonHost() noexcept;

		PythonHost(const PythonHost&) noexcept = delete;
		auto operator=(const PythonHost&) noexcept -> PythonHost& = delete;
		PythonHost(PythonHost&&) noexcept = default;
		auto operator=(PythonHost&&) noexcept -> PythonHost& = default;

		~PythonHost() noexcept override;
	};
}

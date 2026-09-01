#pragma once

#include <optional>
#include <string_view>

#include <core/domain/card.hpp>

#include <core/engine/prototype.hpp>

namespace cg::engine
{
	class Api;
}

namespace cg::script
{
	class Host
	{
	public:
		//

	private:
		virtual auto do_initialize(std::string_view script_root) noexcept -> bool = 0;
		virtual auto do_bind_api(engine::Api& api) noexcept -> void = 0;
		[[nodiscard]] virtual auto do_load_prototype(domain::CardCode code) noexcept -> std::optional<engine::Prototype> = 0;

	public:
		Host() noexcept = default;

		Host(const Host&) noexcept = delete;
		auto operator=(const Host&) noexcept -> Host& = delete;
		Host(Host&&) noexcept = default;
		auto operator=(Host&&) noexcept -> Host& = default;

		virtual ~Host() noexcept = default;

		// 初始化宿主
		auto initialize(std::string_view script_root) noexcept -> bool;

		// 绑定API接口
		auto bind_api(engine::Api& api) noexcept -> void;

		// 载入卡牌原型
		[[nodiscard]] auto load_prototype(domain::CardCode code) noexcept -> std::optional<engine::Prototype>;
	};
}

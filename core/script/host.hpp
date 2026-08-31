#pragma once

#include <optional>

#include <core/domain/card.hpp>

#include <core/engine/prototype.hpp>
#include <core/engine/api.hpp>

namespace cg::script
{
	class Host
	{
	public:
		//

	protected:
		engine::Api* api_;

	private:
		[[nodiscard]] virtual auto do_load_prototype(domain::CardCode code) noexcept -> std::optional<engine::Prototype> = 0;

	public:
		// 绑定Api接口
		auto bind_api(engine::Api& api) noexcept -> void;

		// 载入卡牌原型
		[[nodiscard]] auto load_prototype(domain::CardCode code) noexcept -> std::optional<engine::Prototype>;

		Host() noexcept = default;

		Host(const Host&) noexcept = delete;
		auto operator=(const Host&) noexcept -> Host& = delete;
		Host(Host&&) noexcept = default;
		auto operator=(Host&&) noexcept -> Host& = default;

		virtual ~Host() noexcept = default;
	};
}

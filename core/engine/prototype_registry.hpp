#pragma once

#include <core/engine/prototype.hpp>

namespace cg::engine
{
	class PrototypeRegistry
	{
	public:
		// 注册原型(如果已注册直接覆盖)
		static auto register_prototype(const Prototype& prototype) noexcept -> void;

		// 检查原型是否已注册
		[[nodiscard]] static auto registered(domain::CardCode code) noexcept -> bool;

		// 获取注册的原型,未注册返回nullptr
		[[nodiscard]] static auto find_prototype(domain::CardCode code) noexcept -> const Prototype*;

		// 获取已注册原型数量
		[[nodiscard]] static auto size() noexcept -> std::size_t;

		// 清空注册的原型
		static auto clear() noexcept -> void;
	};
}

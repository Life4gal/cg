#pragma once

#include <unordered_map>

#include <core/engine/prototype.hpp>

namespace cg::engine
{
	class PrototypeRegistry
	{
	public:
		//

	private:
		std::unordered_map<domain::CardCode, Prototype> registered_prototype_;

	public:
		// 注册原型(如果已注册直接覆盖)
		auto register_prototype(const Prototype& prototype) noexcept -> void;

		// 检查原型是否已注册
		[[nodiscard]] auto registered(domain::CardCode code) const noexcept -> bool;

		// 获取注册的原型,未注册返回nullptr
		[[nodiscard]] auto find_prototype(domain::CardCode code) const noexcept -> const Prototype*;

		// 获取已注册原型数量
		[[nodiscard]] auto size() const noexcept -> std::size_t;

		// 清空注册的原型
		auto clear() noexcept -> void;
	};
}

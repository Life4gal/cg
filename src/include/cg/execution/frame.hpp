#pragma once

#include <map>
#include <vector>

#include <cg/dsl/id.hpp>

#include <cg/execution/value.hpp>

#include <cg/state/id.hpp>

namespace cg::execution
{
	// 效果帧的求值状态(绑定/元素作用域)
	class Frame
	{
	public:
		using bindings_type = std::map<dsl::BindingId, Value>;
		using scopes_type = std::vector<state::CardInstanceId>;

		// 效果帧绑定
		bindings_type bindings;
		// 元素作用域栈($)
		scopes_type element_scopes;
	};
}

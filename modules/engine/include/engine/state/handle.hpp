#pragma once

#include <engine/opacity.hpp>

namespace cg::engine
{
	// 表达式句柄
	class ExpressionHandle : public Opacity<class ExpressionTag> {};

	// 效果程序句柄
	class ProgramHandle : public Opacity<class ProgramTag> {};
}

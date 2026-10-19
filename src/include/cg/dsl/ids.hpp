#pragma once

#include <cstdint>

#include <cg/utility/opacity.hpp>

namespace cg::dsl
{
	// 词汇表原语ID
	class PrimitiveId : public utility::Opacity<class TagPrimitive>
	{
	public:
		using Opacity::Opacity;
	};

	// 词汇表类型ID
	class TypeId : public utility::Opacity<class TagType>
	{
	public:
		using Opacity::Opacity;
	};

	// 效果帧内绑定ID
	class BindingId : public utility::Opacity<class TagBinding>
	{
	public:
		using Opacity::Opacity;
	};

	// 系列/字段ID
	// FNV-1a 64位哈希值 => hash("字段名称")
	class ArchetypeId : public utility::Opacity<class TagArchetype, std::uint64_t>
	{
	public:
		using Opacity::Opacity;
	};

	// 效果ID
	class EffectId : public utility::Opacity<class TagEffect>
	{
	public:
		using Opacity::Opacity;
	};
}

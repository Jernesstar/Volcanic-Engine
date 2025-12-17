#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace Magma::Graphics {

enum class BufferDataType {
	Int, Float, Vec2, Vec3, Vec4, Mat2, Mat3, Mat4,
	UVec4
};

struct BufferElement {
	const std::string Name;
	const BufferDataType Type;
	const u32 Size;
	const u32 Count;

	BufferElement(const std::string& name, BufferDataType type)
		: Name(name), Type(type),
		Size(CalcSize(type)), Count(CalcCount(type)) { }

private:
	static u32 CalcSize(BufferDataType type) {
		switch(type) {
			case BufferDataType::Int:	return 4;
			case BufferDataType::Float: return 4;
			case BufferDataType::Vec2:	return 4 * 2;
			case BufferDataType::Vec3:	return 4 * 3;
			case BufferDataType::Vec4:	return 4 * 4;
			case BufferDataType::UVec4:	return 4;
			case BufferDataType::Mat2:	return 4 * 2 * 2;
			case BufferDataType::Mat3:	return 4 * 3 * 3;
			case BufferDataType::Mat4:	return 4 * 4 * 4;
		}

		return 0;
	}
	static u32 CalcCount(BufferDataType type) {
		switch(type) {
			case BufferDataType::Int:	return 1;
			case BufferDataType::Float: return 1;
			case BufferDataType::Vec2:	return 2;
			case BufferDataType::Vec3:	return 3;
			case BufferDataType::Vec4:	return 4;
			case BufferDataType::UVec4:	return 4;
			case BufferDataType::Mat2:	return 2; // 2 * Vec2
			case BufferDataType::Mat3:	return 3; // 3 * Vec3
			case BufferDataType::Mat4:	return 4; // 4 * Vec4
		}

		return 0;
	}
};

class BufferLayout {
public:
	const List<BufferElement> Elements;
	const u32 Stride;
	const bool Instanced;

public:
	BufferLayout(const std::initializer_list<BufferElement>& elements,
				 bool instanced = false)
		: Elements(elements), Stride(CalcStride(elements)),
		Instanced(instanced) { }

	List<BufferElement>::const_iterator begin() const {
		return Elements.begin();
	}
	List<BufferElement>::const_iterator end() const {
		return Elements.end();
	}

private:
	u32 CalcStride(const List<BufferElement>& elements) {
		u32 stride = 0;
		for(auto& element : elements)
			stride += element.Size;
		return stride;
	}
};

}
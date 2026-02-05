#pragma once

#include <glad/glad.h>

#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/Buffer.h>

#include "Platform/BufferLayout.h"

using namespace Magma;

namespace OpenGL {

class VertexBuffer {
public:
	const Graphics::BufferLayout Layout;
	const u32 Count;
	const u32 Size;

public:
	VertexBuffer(const Graphics::BufferLayout& layout,
				 u32 count, bool dynamic = false, const void* data = nullptr)
		: Layout(layout), Count(count), Size(count * layout.Stride)
	{
		glCreateBuffers(1, &m_BufferID);
		glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
		glBufferData(GL_ARRAY_BUFFER, Size, data,
					 dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	template<typename T>
	VertexBuffer(const Graphics::BufferLayout& layout, Buffer<T> buffer)
		: Layout(layout), Count(buffer.GetCount()), Size(buffer.GetSize())
	{
		VOLCANICORE_ASSERT(layout.Stride == sizeof(T));

		glCreateBuffers(1, &m_BufferID);
		glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
		glBufferData(GL_ARRAY_BUFFER, Size, buffer.Get(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	template<typename T, std::size_t TCount>
	VertexBuffer(const Graphics::BufferLayout& layout,
				 const T (&vertices)[TCount])
		: Layout(layout), Count(TCount), Size(TCount * layout.Stride)
	{
		glCreateBuffers(1, &m_BufferID);
		glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
		glBufferData(GL_ARRAY_BUFFER, Size, vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	~VertexBuffer() {
		glDeleteBuffers(1, &m_BufferID);
	}

	void Bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
	}
	void Unbind() const {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void SetData(const void* data, u32 count = 0, u32 offset = 0) {
		glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
		glBufferSubData(GL_ARRAY_BUFFER, offset * Layout.Stride,
						(count == 0) ? Size : count * Layout.Stride, data);
	}

	template<typename T>
	void SetData(const Buffer<T>& buffer, u32 offset = 0) {
		SetData(buffer.Get(), buffer.GetCount(), offset);
	}

	template<typename T, std::size_t TCount>
	void SetData(const T (&vertices)[TCount]) {
		SetData(vertices, TCount);
	}

private:
	u32 m_BufferID;
};

}
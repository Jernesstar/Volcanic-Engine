#pragma once

#include <glad/glad.h>

#include <VolcaniCore/Core/Buffer.h>

namespace OpenGL {

class IndexBuffer {
public:
	const u32 Count;
	const u32 Size;

public:
	IndexBuffer(u32 count, bool dynamic = false, const u32* indices = nullptr)
		: Count(count), Size(count * sizeof(u32))
	{
		glCreateBuffers(1, &m_BufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, Size, indices,
					 dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	IndexBuffer(const Buffer<u32>& buffer, bool dynamic = false)
		: Count(buffer.GetCount()), Size(buffer.GetSize())
	{
		glCreateBuffers(1, &m_BufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, Size, buffer.Get(),
					 dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	template<std::size_t TCount>
	IndexBuffer(const u32 (&indices)[TCount])
		: Count(TCount), Size(TCount * sizeof(u32))
	{
		glCreateBuffers(1, &m_BufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, Size, indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	~IndexBuffer() {
		glDeleteBuffers(1, &m_BufferID);
	}

	void Bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID);
	}
	void Unbind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void SetData(const void* data, u32 count = 0, u32 offset = 0) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof(u32),
						(count == 0) ? Size : count * sizeof(u32), data);
		
	}

	void SetData(const Buffer<u32>& buffer, u32 offset = 0) {
		SetData(buffer.Get(), buffer.GetCount(), offset);
	}

	template<std::size_t TCount>
	void SetData(const u32 (&indices)[TCount]) {
		SetData(indices, TCount);
	}

private:
	u32 m_BufferID;
};

}
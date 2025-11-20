#pragma once

#include <glad/glad.h>

#include "Platform/StorageBuffer.h"

using namespace VolcaniCore;
using namespace Magma;

namespace OpenGL {

class StorageBuffer : public Graphics::StorageBuffer {
public:
	StorageBuffer(const Graphics::StorageBufferSpec& spec)
		: Graphics::StorageBuffer(spec)
	{
		glCreateBuffers(1, &m_BufferID);
		glNamedBufferStorage(m_BufferID, spec.Layout.Stride * spec.Count,
							 nullptr, GL_DYNAMIC_STORAGE_BIT);
	}

	~StorageBuffer() {
		glDeleteBuffers(1, &m_BufferID);
	}

	void SetData(const void* data, uint64_t count = 1,
				 uint64_t offset = 0) override
	{
		glNamedBufferSubData(m_BufferID, offset * Spec.Layout.Stride,
							 count * Spec.Layout.Stride, data);
	}

	void Bind(uint32_t binding) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_BufferID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_BufferID);
	}

private:
	uint32_t m_BufferID;
};

}
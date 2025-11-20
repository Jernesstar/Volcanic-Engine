#pragma once

#include <glad/glad.h>

#include <VolcaniCore/Core/Assert.h>

#include "Platform/UniformBuffer.h"

using namespace Magma;

namespace OpenGL {

class UniformBuffer : public Graphics::UniformBuffer {
public:
	UniformBuffer(const Graphics::UniformBufferSpec& spec)
		: Graphics::UniformBuffer(spec)
	{
		glCreateBuffers(1, &m_BufferID);
		glNamedBufferData(m_BufferID, Spec.Layout.Stride * Spec.Count,
						  nullptr, GL_DYNAMIC_DRAW);
	}

	~UniformBuffer() {
		glDeleteBuffers(1, &m_BufferID);
	}

	void SetData(const void* data, uint64_t count = 1,
				 uint64_t offset = 0) override
	{
		glNamedBufferSubData(m_BufferID,
							 offset * Spec.Layout.Stride,
							 count * Spec.Layout.Stride, data);
	}

	void Bind(uint32_t binding) {
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_BufferID);
	}

private:
	uint32_t m_BufferID;
};

}
#pragma once

#include <vector>

#include <VolcaniCore/Core/Defines.h>

#include "Platform/BufferLayout.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"

using namespace Magma;

namespace OpenGL {

class VertexArray {
public:
	VertexArray() {
		glCreateVertexArrays(1, &m_VertexArrayID);
	}
	VertexArray(Ref<VertexBuffer> vertexBuffer,
				Ref<IndexBuffer> indexBuffer = nullptr)
	{
		glCreateVertexArrays(1, &m_VertexArrayID);
		AddVertexBuffer(vertexBuffer);
		SetIndexBuffer(indexBuffer);
	}
	VertexArray(std::initializer_list<Ref<VertexBuffer>> vertexBuffers,
				Ref<IndexBuffer> indexBuffer = nullptr)
	{
		glCreateVertexArrays(1, &m_VertexArrayID);
		SetIndexBuffer(indexBuffer);

		for(auto buffer : vertexBuffers)
			AddVertexBuffer(buffer);
	}
	~VertexArray() {
		glDeleteVertexArrays(1, &m_VertexArrayID);
	}

	void Bind() const { glBindVertexArray(m_VertexArrayID); }
	void Unbind() const { glBindVertexArray(0); }

	void SetIndexBuffer(Ref<IndexBuffer> buffer) {
		if(!buffer)
			return;

		m_IndexBuffer = buffer;
		Bind();
		buffer->Bind();
		Unbind();
	}

	void AddVertexBuffer(Ref<VertexBuffer> buffer) {
		VOLCANICORE_ASSERT(buffer, "Vertex buffer was null");
		m_VertexBuffers.Add(buffer);

		Bind();
		buffer->Bind();

		auto& layout = buffer->Layout;
		u64 stride = layout.Stride;
		u64 offset = 0;

		for(auto& element : layout) {
			u64 count = element.Count;
			bool normalize = element.Normalize ? GL_TRUE : GL_FALSE;

			switch(element.Type) {
				case Graphics::BufferDataType::Int:
				{
					glEnableVertexAttribArray(m_BufferIndex);
					glVertexAttribIPointer(
						m_BufferIndex++, count, GL_INT, stride, (void*)offset);
					break;
				}
				case Graphics::BufferDataType::Float:
				case Graphics::BufferDataType::Vec2:
				case Graphics::BufferDataType::Vec3:
				case Graphics::BufferDataType::Vec4:
				{
					glEnableVertexAttribArray(m_BufferIndex);
					auto type = normalize ? GL_UNSIGNED_BYTE : GL_FLOAT;
					glVertexAttribPointer(m_BufferIndex, count, type,
										  normalize, stride, (void*)offset);
					if(layout.Instanced)
						glVertexAttribDivisor(m_BufferIndex, 1);

					m_BufferIndex++;
					break;
				}
				case Graphics::BufferDataType::Mat2:
				case Graphics::BufferDataType::Mat3:
				case Graphics::BufferDataType::Mat4:
				{
					bool instanced = layout.Instanced;
					u64 vecSize = sizeof(float) * count;

					for(u64 i = 0; i < count; i++) {
						glEnableVertexAttribArray(m_BufferIndex);
						glVertexAttribPointer(
							m_BufferIndex, count, GL_FLOAT, normalize, stride,
							(void*)(offset + (vecSize * i)));

						glVertexAttribDivisor(m_BufferIndex++,
											  (uint32_t)instanced);
					}
					break;
				}
			}

			offset += element.Size;
		}

		Unbind();
	}

	bool HasIndexBuffer() const { return bool(m_IndexBuffer); }
	Ref<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }

	Ref<VertexBuffer> GetVertexBuffer(uint32_t index = 0) {
		return m_VertexBuffers[index];
	}

private:
	uint32_t m_VertexArrayID;
	uint32_t m_BufferIndex = 0;

	List<Ref<VertexBuffer>> m_VertexBuffers;
	Ref<IndexBuffer> m_IndexBuffer;
};

}
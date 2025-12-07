#include "Renderer.h"

#include <VolcaniCore/Core/Assert.h>

#include <glad/glad.h>

#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "UniformBuffer.h"
#include "StorageBuffer.h"

using namespace Magma::Graphics;

namespace OpenGL {

class DrawBuffer : public Graphics::DrawBuffer {
public:
	Ref<VertexArray> Array;
	Buffer<uint32_t> Indices;
	Buffer<void> Vertices;
	Buffer<void> Instances;

public:
	DrawBuffer(const DrawBufferSpec& spec)
		: Graphics::DrawBuffer(spec)
	{
		Array = CreateRef<VertexArray>();
		if(spec.IndexCount) {
			Array->SetIndexBuffer(
				CreateRef<IndexBuffer>(spec.IndexCount, true));
			Indices = Buffer<uint32_t>(spec.IndexCount);
		}
		if(spec.VertexCount) {
			Array->AddVertexBuffer(
				CreateRef<VertexBuffer>(spec.VertexLayout, spec.VertexCount));
			Vertices
				= Buffer<void>(spec.VertexLayout.Stride, spec.VertexCount);
		}
		if(spec.InstanceCount) {
			Array->AddVertexBuffer(
				CreateRef<VertexBuffer>(
					spec.InstanceLayout, spec.InstanceCount));
			Instances
				= Buffer<void>(spec.InstanceLayout.Stride, spec.InstanceCount);
		}
	}
	~DrawBuffer() = default;

	void Add(DrawBufferIndex index, const void* data, u64 count) override {
		switch(index) {
			case DrawBufferIndex::Index: {
				Indices.Add(data, count);
				if(!Spec.DynamicIndices)
					Array->GetIndexBuffer()->SetData(Indices);
				break;
			}
			case DrawBufferIndex::Vertex: {
				Vertices.Add(data, count);
				if(!Spec.DynamicVertices)
					Array->GetVertexBuffer(0)->SetData(Vertices);
				break;
			}
			case DrawBufferIndex::Instance: {
				Instances.Add(data, count);
				if(!Spec.DynamicInstances) {
					u32 idx = Spec.VertexCount != 0;
					Array->GetVertexBuffer(idx)->SetData(Instances);
				}
				break;
			}
		}
	}

	void Clear() override {
		Indices.Clear();
		Vertices.Clear();
		Instances.Clear();
	}

	u64 GetIndexCount() const override { return Indices.GetCount(); }
	u64 GetVertexCount() const override { return Vertices.GetCount(); }
	u64 GetInstanceCount() const override { return Instances.GetCount(); }
};

static List<DrawBuffer*> s_Buffers;
static List<DrawPass> s_Passes;
static List<DrawCommand> s_Commands;
static List<DrawCall> s_Calls;

Renderer::Renderer() {
	int success = gladLoadGL();
	VOLCANICORE_ASSERT(success, "Glad could not load OpenGL");
	VOLCANICORE_LOG_INFO(
		"Successfully loaded OpenGL\n"
		"\tVersion: %s\n"
		"\tGPU: %s", glGetString(GL_VERSION), glGetString(GL_RENDERER));
}

void Renderer::Init() {
	glEnable(GL_MULTISAMPLE);		// Smooth edges
	glEnable(GL_FRAMEBUFFER_SRGB);	// Gamma correction
}

void Renderer::Close() {
	for(auto buffer : s_Buffers)
		delete buffer;

	s_Buffers.Clear();
}

void Renderer::BeginFrame() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void SetUniforms(DrawCommand& cmd) {
	auto& uniforms = cmd.Uniforms;
	auto shader = cmd.Pass->Pipeline;

	for(auto& [name, data] : uniforms.IntUniforms)
		shader->SetInt(name, data);
	for(auto& [name, data] : uniforms.FloatUniforms)
		shader->SetFloat(name, data);
	for(auto& [name, slot] : uniforms.TextureUniforms) {
		if(!slot.Sampler)
			continue;
		slot.Sampler->As<OpenGL::Texture>()->Bind(slot.Index);
		shader->SetInt(name, slot.Index);
	}
	for(auto& [name, data] : uniforms.Vec2Uniforms)
		shader->SetVec2(name, data);
	for(auto& [name, data] : uniforms.Vec3Uniforms)
		shader->SetVec3(name, data);
	for(auto& [name, data] : uniforms.Vec4Uniforms)
		shader->SetVec4(name, data);
	for(auto& [name, data] : uniforms.Mat2Uniforms)
		shader->SetMat2(name, data);
	for(auto& [name, data] : uniforms.Mat3Uniforms)
		shader->SetMat3(name, data);
	for(auto& [name, data] : uniforms.Mat4Uniforms)
		shader->SetMat4(name, data);

	for(auto& [buffer, name, binding] : uniforms.UniformBuffers) {
		if(!buffer)
			continue;
		if(name != "")
			shader->SetUniformBuffer(name, binding);
		buffer->As<OpenGL::UniformBuffer>()->Bind(binding);
	}

	for(auto& [buffer, name, binding] : uniforms.StorageBuffers) {
		if(!buffer)
			continue;
		if(name != "")
			shader->SetStorageBuffer(name, binding);
		buffer->As<OpenGL::StorageBuffer>()->Bind(binding);
	}
}

static void SetOptions(DrawCommand& cmd) {
	if(cmd.Scissor) {
		glEnable(GL_SCISSOR_TEST);
		glScissor(cmd.ScissorX, cmd.ScissorY, cmd.ScissorW, cmd.ScissorH);
	}
	else
		glDisable(GL_SCISSOR_TEST);

	if(cmd.Viewport)
		glViewport(cmd.ViewportX, cmd.ViewportY, cmd.ViewportW, cmd.ViewportH);
	if(cmd.Clear) {
		glClearColor(cmd.ClearColor.r, cmd.ClearColor.g, cmd.ClearColor.b, cmd.ClearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	if(cmd.DepthTesting == DepthTestingMode::On)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if(cmd.Blending == BlendingMode::Off)
		glDisable(GL_BLEND);
	else
		glEnable(GL_BLEND);
	if(cmd.Blending == BlendingMode::Greatest)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else if(cmd.Blending == BlendingMode::Additive) {
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_ADD);
	}

	if(cmd.Culling == CullingMode::Off)
		glDisable(GL_CULL_FACE);
	else {
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
	}
	if(cmd.Culling == CullingMode::Front)
		glCullFace(GL_FRONT);
	else if(cmd.Culling == CullingMode::Back)
		glCullFace(GL_BACK);
}
struct DrawArraysIndirectCommand {
	u32 Count;
	u32 InstanceCount;
	u32 First;
	u32 BaseInstance;
};

struct DrawElementsIndirectCommand{
	u32 Count;
	u32 InstanceCount;
	u32 FirstIndex;
	i32 BaseVertex;
	u32 BaseInstance;
};

static void SubmitDrawCall(DrawCommand& cmd, DrawCall& call) {
	uint32_t primitive;
	switch(call.Primitive) {
		case DrawPrimitive::Point:
			primitive = GL_POINTS;
			break;
		case DrawPrimitive::Line:
			primitive = GL_LINES;
			break;
		case DrawPrimitive::Triangle:
			primitive = GL_TRIANGLES;
			break;
	}

	if(call.Partition == DrawPartition::Single) {
		if(call.IndexCount == 0)
			glDrawArrays(primitive, cmd.VerticesIndex + call.VertexOffset,
						 call.VertexCount);
		else
			glDrawElementsBaseVertex(
				primitive, call.IndexCount, GL_UNSIGNED_INT,
				(void*)(sizeof(u32) * (cmd.IndicesIndex + call.IndexOffset)),
				cmd.VerticesIndex + call.VertexOffset);
	}
	else if(call.Partition == DrawPartition::Instanced) {
		if(call.IndexCount == 0)
			glDrawArraysInstancedBaseInstance(
				primitive, cmd.VerticesIndex + call.VertexOffset,
				call.VertexCount, call.InstanceCount, call.InstanceOffset);
		else
			glDrawElementsInstancedBaseVertexBaseInstance(
				primitive, call.IndexCount, GL_UNSIGNED_INT,
				(void*)(sizeof(u32) * (cmd.IndicesIndex + call.IndexOffset)),
				call.InstanceCount, cmd.VerticesIndex + call.VertexOffset,
				call.InstanceOffset);
	}
	else if(call.Partition == DrawPartition::MultiDraw) {
		// if(call.IndexCount == 0)
		// 	glMultiDrawArraysIndirect();
		// else
		// 	glMultiDrawElementsIndirect();

		// Add indirect objects. When full, make draw call and clear
	}
}

void Renderer::EndFrame() {
	if(!s_Commands)
		return;

	for(auto& buffer : s_Buffers) {
		if(buffer->Spec.DynamicIndices && buffer->Spec.IndexCount)
			buffer->Array->GetIndexBuffer()->SetData(buffer->Indices);
		if(buffer->Spec.DynamicVertices && buffer->Spec.VertexCount)
			buffer->Array->GetVertexBuffer(0)->SetData(buffer->Vertices);
		if(buffer->Spec.DynamicInstances && buffer->Spec.InstanceCount) {
			u32 idx = buffer->Spec.VertexCount != 0;
			buffer->Array->GetVertexBuffer(idx)->SetData(buffer->Instances);
		}
	}

	for(auto& cmd : s_Commands) {
		SetOptions(cmd);

		auto shader = cmd.Pass->Pipeline;
		if(shader) {
			shader->As<OpenGL::Shader>()->Bind();
			SetUniforms(cmd);
		}
		else
			glUseProgram(0);

		auto out = cmd.Pass->Output;
		if(out) {
			out->As<OpenGL::Framebuffer>()->Bind();
		}
		else
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		auto buffer = cmd.Pass->Buffer;
		if(buffer) {
			buffer->As<OpenGL::DrawBuffer>()->Array->Bind();
		}
		else
			glBindVertexArray(0);

		for(auto& call : cmd.DrawCalls)
			SubmitDrawCall(cmd, call);
	}

	s_Commands.Clear();
}

Graphics::DrawBuffer* Renderer::NewBuffer(const Graphics::DrawBufferSpec& s) {
	auto buffer = new DrawBuffer(s);
	s_Buffers.Add(buffer);
	return buffer;
}

Graphics::DrawPass* Renderer::NewPass(Graphics::DrawBuffer* buffer) {
	// VOLCANICORE_ASSERT(buffer);
	return &s_Passes.Emplace(buffer);
}

Graphics::DrawCommand* Renderer::NewCommand(Graphics::DrawPass* pass) {
	// VOLCANICORE_ASSERT(pass);
	return &s_Commands.Emplace(pass);
}

}
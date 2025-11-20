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
	u32 MaxIndexCount;
	u32 MaxVertexCount;
	u32 MaxInstanceCount;

public:
	DrawBuffer(const DrawBufferSpec& spec)
		: Graphics::DrawBuffer(spec),
			Indices(spec.IndexCount),
			Vertices(spec.VertexCount, spec.Vertex.Stride),
			Instances(spec.InstanceCount, spec.Instance.Stride)
	{
		Array = CreateRef<VertexArray>();
	}
	~DrawBuffer() = default;

	void SetData(DrawBufferIndex index, Buffer<void> data) override {
		switch(index) {
			case DrawBufferIndex::Vertex:
				Vertices.Set(data.Get(), data.GetCount());
				break;
			case DrawBufferIndex::Index:
				Indices.Set(data.Get(), data.GetCount());
				break;
			case DrawBufferIndex::Instance:
				Instances.Set(data.Get(), data.GetCount());
				break;
		}
	}

	void Clear() override {
		Indices.Clear();
		Vertices.Clear();
		Instances.Clear();
	}
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

}

void Renderer::BeginFrame() {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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
	if(cmd.Viewport)
		glViewport(cmd.ViewportX, cmd.ViewportY, cmd.ViewportW, cmd.ViewportH);
	if(cmd.Clear) {
		glClearColor(cmd.ClearColor.r, cmd.ClearColor.g, cmd.ClearColor.b, cmd.ClearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	if(cmd.DepthTesting == DepthTestingMode::On)
		glEnable(GL_DEPTH_TEST);
	else if(cmd.DepthTesting == DepthTestingMode::Off)
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

	if(call.Partition == DrawPartition::MultiDraw) {
		// if(call.IndexCount == 0)
		// 	glMultiDrawArraysIndirect();
		// else
		// 	glMultiDrawElementsIndirect();

		// Add indirect objects. When full, make draw call and clear
		return;
	}

	if(call.Partition == DrawPartition::Single) {
		if(call.IndexCount == 0)
			glDrawArrays(primitive, cmd.VerticesIndex + call.VertexOffset,
						 call.VertexCount);
		else
			glDrawElementsBaseVertex(
				primitive, call.IndexCount, GL_UNSIGNED_INT,
				(void*)(sizeof(uint32_t) * (cmd.IndicesIndex + call.IndexOffset)),
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
				(void*)(sizeof(uint32_t) * (cmd.IndicesIndex + call.IndexOffset)),
				call.InstanceCount, cmd.VerticesIndex + call.VertexOffset,
				call.InstanceOffset);
	}
}

void Renderer::EndFrame() {
	if(!s_Commands)
		return;

	for(auto& buffer : s_Buffers) {
		if(buffer->Indices.GetCount() && buffer->MaxIndexCount)
			buffer->Array->GetIndexBuffer()->SetData(buffer->Indices);
		if(buffer->Vertices.GetCount() && buffer->MaxVertexCount)
			buffer->Array->GetVertexBuffer(0)->SetData(buffer->Vertices);
		if(buffer->Instances.GetCount() && buffer->MaxInstanceCount) {
			// If the vertex buffer is empty, the instance buffer will be the
			// first buffer in the array
			u32 idx = buffer->MaxVertexCount != 0;
			buffer->Array->GetVertexBuffer(idx)->SetData(buffer->Instances);
		}
	}

	for(auto& cmd : s_Commands) {
		SetOptions(cmd);
		SetUniforms(cmd);

		for(auto& call : cmd.DrawCalls)
			SubmitDrawCall(cmd, call);

		if(cmd.Scissor)
			glDisable(GL_SCISSOR_TEST);
	}

	s_Commands.Clear();
	s_Passes.Clear();
}

Graphics::DrawBuffer* Renderer::NewBuffer(const Graphics::DrawBufferSpec& s) {
	auto buffer = new DrawBuffer(s);
	s_Buffers.Add(buffer);
	return buffer;
}

Graphics::DrawPass* Renderer::NewPass(Graphics::DrawBuffer* buffer) {
	return &s_Passes.Emplace(buffer);
}

Graphics::DrawCommand* Renderer::NewCommand(Graphics::DrawPass* pass) {
	return &s_Commands.Emplace(pass);
}

}
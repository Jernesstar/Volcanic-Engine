#include "Renderer.h"

#include <VolcaniCore/Core/Assert.h>

#include <glad/glad.h>

#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "UniformBuffer.h"
#include "StorageBuffer.h"

using namespace VolcanicEngine::Graphics;

namespace OpenGL {

static List<DrawBuffer*> s_Buffers;
static List<DrawPass> s_Passes;
static List<DrawCommand> s_Commands;

static Ref<VertexArray> s_EmptyVAO;

// TODO(Add): Use stream buffer
class DrawBuffer : public Graphics::DrawBuffer {
public:
	Ref<VertexArray> Array;
	Buffer<u32> Indices;
	Buffer<void> Vertices;
	Buffer<void> Instances;
	u32 IndicesCount = 0;
	u32 VerticesCount = 0;
	u32 InstancesCount = 0;

public:
	DrawBuffer(const DrawBufferSpec& spec)
		: Graphics::DrawBuffer(spec)
	{
		Array = CreateRef<VertexArray>();
		if(spec.IndexCount) {
			Array->SetIndexBuffer(
				CreateRef<IndexBuffer>(spec.IndexCount, spec.DynamicIndices));
			if(spec.DynamicIndices)
				Indices = Buffer<u32>(spec.IndexCount);
		}
		if(spec.VertexCount) {
			Array->AddVertexBuffer(
				CreateRef<VertexBuffer>(
					spec.VertexLayout, spec.VertexCount, spec.DynamicVertices));
			if(spec.DynamicVertices)
				Vertices
					= Buffer<void>(spec.VertexLayout.Stride, spec.VertexCount);
		}
		if(spec.InstanceCount) {
			Array->AddVertexBuffer(
				CreateRef<VertexBuffer>(
					spec.InstanceLayout, spec.InstanceCount,
					spec.DynamicInstances));
			if(spec.DynamicInstances)
				Instances =
					Buffer<void>(
						spec.InstanceLayout.Stride, spec.InstanceCount);
		}
	}
	~DrawBuffer() {
		Clear();
		auto [found, i] = s_Buffers.Find([&](auto& b) { return b == this; });
		if(found)
			s_Buffers.Pop(i);
		else
			VOLCANICORE_ASSERT(false, "Draw buffer not found for deletion");
	}

	void Add(DrawBufferIndex index, const void* data, u32 count) override {
		switch(index) {
			case DrawBufferIndex::E_Index: {
				if(!Spec.DynamicIndices)
					Array->GetIndexBuffer()->SetData(data, count, IndicesCount);
				else
					Indices.Add(data, count);

				IndicesCount += count;
				break;
			}
			case DrawBufferIndex::E_Vertex: {
				if(!Spec.DynamicVertices)
					Array->GetVertexBuffer(0)->SetData(data, count, VerticesCount);
				else
					Vertices.Add(data, count);

				VerticesCount += count;
				break;
			}
			case DrawBufferIndex::E_Instance: {
				if(!Spec.DynamicInstances) {
					u32 idx = Spec.VertexCount != 0;
					Array->GetVertexBuffer(idx)->SetData(data, count, InstancesCount);
				}
				else
					Instances.Add(data, count);

				InstancesCount += count;
				break;
			}
		}
	}

	void Clear() override {
		if(Spec.DynamicIndices)
			Indices.Clear();
		if(Spec.DynamicVertices)
			Vertices.Clear();
		if(Spec.DynamicInstances)
			Instances.Clear();

		IndicesCount = 0;
		VerticesCount = 0;
		InstancesCount = 0;
	}

	void SendData() {
		if(Spec.DynamicIndices && Spec.IndexCount)
			Array->GetIndexBuffer()->SetData(Indices);
		if(Spec.DynamicVertices && Spec.VertexCount)
			Array->GetVertexBuffer(0)->SetData(Vertices);
		if(Spec.DynamicInstances && Spec.InstanceCount) {
			u32 idx = Spec.VertexCount != 0;
			Array->GetVertexBuffer(idx)->SetData(Instances);
		}
	}

	u32 GetIndexCount() const override { return IndicesCount; }
	u32 GetVertexCount() const override { return VerticesCount; }
	u32 GetInstanceCount() const override { return InstancesCount; }
};

Renderer::Renderer() {
	int success = gladLoadGL();
	VOLCANICORE_ASSERT(success, "Glad could not load OpenGL");
	// printf(
	// 	"Successfully loaded OpenGL\n \
	// 	\tVersion: %s\n \
	// 	\tGPU: %s", glGetString(GL_VERSION), glGetString(GL_RENDERER));
}

void Renderer::Init() {
	glEnable(GL_MULTISAMPLE);		// Smooth edges
	glEnable(GL_FRAMEBUFFER_SRGB);	// Gamma correction

	s_EmptyVAO = CreateRef<VertexArray>();
	s_Passes.Allocate(64);
	s_Commands.Allocate(64);
}

void Renderer::Close() {
	s_Buffers.Clear();
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

	for(auto& [name, slot] : uniforms.TextureUniforms) {
		if(!slot.Sampler)
			continue;

		slot.Sampler->As<OpenGL::Texture>()->Bind(slot.Binding);
		shader->SetInt(name, slot.Binding);
	}
	for(auto& [name, slot] : uniforms.AttachmentUniforms) {
		if(!slot.Sampler)
			continue;

		slot.Sampler->As<OpenGL::Attachment>()->Bind(slot.Binding);
		shader->SetInt(name, slot.Binding);
	}
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
	u32 primitive;
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

	for(auto& buffer : s_Buffers)
		buffer->As<OpenGL::DrawBuffer>()->SendData();

	for(auto& cmd : s_Commands) {
		SetOptions(cmd);

		if(cmd.Pass && cmd.Pass->Pipeline) {
			cmd.Pass->Pipeline->As<OpenGL::Shader>()->Bind();
			SetUniforms(cmd);
		}
		else
			glUseProgram(0);

		if(cmd.Pass && cmd.Pass->Output)
			cmd.Pass->Output->As<OpenGL::Framebuffer>()->Bind();
		else
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if(cmd.Pass && cmd.Pass->Output) {
			uint32_t i = 0;
			for(auto& [target, idx] : cmd.Outputs)
				cmd.Pass->Output->Attach(target, idx, i++);
		}

		if(cmd.Pass && cmd.Pass->Buffer)
			cmd.Pass->Buffer->As<OpenGL::DrawBuffer>()->Array->Bind();
		else
			s_EmptyVAO->Bind();

		if(cmd.Pass && cmd.ComputeX && cmd.ComputeY && cmd.ComputeZ)
			cmd.Pass->Pipeline->As<OpenGL::Shader>()
				->Compute(cmd.ComputeX, cmd.ComputeY, cmd.ComputeZ);

		for(auto& call : cmd.DrawCalls)
			SubmitDrawCall(cmd, call);
	}

	s_Commands.Clear();
	s_Passes.Clear();
}

Graphics::DrawBuffer* Renderer::NewBuffer(const Graphics::DrawBufferSpec& s) {
	auto* buffer = new DrawBuffer(s);
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
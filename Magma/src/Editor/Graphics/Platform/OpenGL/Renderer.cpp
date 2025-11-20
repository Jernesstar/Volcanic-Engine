#include "Renderer.h"

#include <VolcaniCore/Core/Assert.h>

#include <glad/glad.h>

#include "VertexArray.h"

namespace OpenGL {

struct DrawBuffer {
	Ref<VertexArray> Array;
	Buffer<uint32_t> Indices;
	Buffer<void> Vertices;
	Buffer<void> Instances;
};

static List<DrawBuffer> s_Buffers;
static List<DrawPass> s_Passes;
static List<DrawCommand> s_Commands;
static List<DrawCall> s_Calls;

Renderer::Renderer()
	: RendererAPI(RendererAPI::Backend::OpenGL)
{
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

void Renderer::EndFrame() {

	for(auto& cmd : s_Commands) {
		if(cmd.Scissor) {
			glEnable(GL_SCISSOR_TEST);
			glScissor(cmd.ScissorX, cmd.ScissorY, cmd.ScissorW, cmd.ScissorH);
		}
		if(cmd.Clear) {
			glClearColor(cmd.ClearColor.r, cmd.ClearColor.g, cmd.ClearColor.b, cmd.ClearColor.a);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		// Draw calls ...

		if(cmd.Scissor)
			glDisable(GL_SCISSOR_TEST);
	}

	s_Commands.Clear();
	s_Passes.Clear();
}

DrawBufferID Renderer::NewBuffer(const DrawBufferSpec& spec) {
	auto& buffer =
		s_Buffers.Emplace(nullptr,
			Buffer<uint32_t>(spec.IndexCount),
			Buffer<void>(spec.VertexCount, spec.Vertex.Stride),
			Buffer<void>(spec.InstanceCount, spec.Instance.Stride));

	buffer.Array = CreateRef<VertexArray>();

	if(spec.VertexCount)
		buffer.Array->AddVertexBuffer(
			CreateRef<VertexBuffer>(spec.Vertex, spec.VertexCount));
	if(spec.IndexCount)
		buffer.Array->SetIndexBuffer(CreateRef<IndexBuffer>(spec.IndexCount));
	if(spec.InstanceCount)
		buffer.Array->AddVertexBuffer(
			CreateRef<VertexBuffer>(spec.Instance, spec.InstanceCount));

	return s_Buffers.Count();
}

void Renderer::SetBufferData(DrawBufferID id, DrawBufferIndex index, Buffer<void> data) {
	auto* buffer = s_Buffers.At(id);

	switch(index) {
		case DrawBufferIndex::Vertex:
			buffer->Vertices.Set(data.Get(), data.GetCount());
			break;
		case DrawBufferIndex::Index:
			buffer->Indices.Set(data.Get(), data.GetCount());
			break;
		case DrawBufferIndex::Instance:
			buffer->Instances.Set(data.Get(), data.GetCount());
			break;
	}
}

DrawPass* Renderer::NewPass(DrawBufferID id) {
	return &s_Passes.Emplace();
}

DrawCommand* Renderer::NewCommand(DrawPass* pass) {
	return &s_Commands.Emplace();
}

DrawCall* Renderer::NewCall(DrawCommand* cmd) {
	return &s_Calls.Emplace();
}

}
#include "Renderer.h"

#include <VolcaniCore/Core/Assert.h>

#include <glad/glad.h>

#include "VertexArray.h"

namespace OpenGL {

struct DrawBuffer {
	Ref<VertexArray> Array;
};

struct DrawPass {

};

struct DrawCommand {
	bool Clear = false;
	Vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	bool Scissor = false;
	f32 ScissorX = 0, ScissorY = 0, ScissorW = 0, ScissorH = 0;
};

static List<DrawBuffer> s_Buffers;
static List<DrawPass> s_Passes;
static List<DrawCommand> s_Commands;

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

}

void Renderer::SetBufferData(DrawBufferID id, DrawBufferIndex index, Buffer<void> data) {

}

DrawPassID Renderer::NewPass(const DrawPassSpec& spec) {

}

DrawCommandID Renderer::NewCommand(const DrawCommandSpec& spec) {
	auto& cmd = s_Commands.Emplace();

	cmd.Clear = spec.Clear;
	cmd.ClearColor = spec.ClearColor;
	cmd.Scissor = spec.Scissor;
	cmd.ScissorX = spec.ScissorX;
	cmd.ScissorY = spec.ScissorY;
	cmd.ScissorW = spec.ScissorW;
	cmd.ScissorH = spec.ScissorH;

	return s_Commands.Count();
}

}
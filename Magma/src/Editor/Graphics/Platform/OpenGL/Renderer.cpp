#include "Renderer.h"

#include <VolcaniCore/Core/Assert.h>

#include <glad/glad.h>

namespace OpenGL {

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
	glEnable(GL_MULTISAMPLE);				// Smooth edges
	glEnable(GL_FRAMEBUFFER_SRGB);			// Gamma correction
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // Smooth cubemap edges

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void Renderer::Close() {

}

void Renderer::StartFrame() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame() {

}

}
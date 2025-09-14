#include "Igneous.h"

// #include <Magma/Graphics/Renderer.h>
#include <Magma/Graphics/RendererAPI.h>
#include <Magma/Graphics/ShaderLibrary.h>

using namespace Magma::Graphics;

namespace Lava {

void Igneous::Init() {
	RendererAPI::Create(RendererAPI::Backend::OpenGL);
	// Renderer::Init();
}

void Igneous::Shutdown() {
	// Renderer::Close();
	RendererAPI::Shutdown();
}

void Igneous::BeginFrame() {
	RendererAPI::Get()->StartFrame();
	// Renderer::BeginFrame();
	// Renderer::Clear();
}

void Igneous::EndFrame() {
	// Renderer::EndFrame();
	RendererAPI::Get()->EndFrame();
}

void Igneous::OnUpdate(TimeStep ts) {
	// float fps = (1.0f / (float)ts) * 1000.0f;
	// Renderer::GetFrame().Info.FPS = fps;

}

void Igneous::OnEvent(uint32_t event) {
	// float fps = (1.0f / (float)ts) * 1000.0f;
	// Renderer::GetFrame().Info.FPS = fps;

}

}


#include "Igneous.h"

#include <Magma/Graphics/Renderer.h>
#include <Magma/Graphics/RendererAPI.h>
#include <Magma/Graphics/ShaderLibrary.h>

using namespace Magma::Graphics;

namespace Igneous {

void Init() {
	RendererAPI::Create(RendererAPI::Backend::OpenGL);
	Renderer::Init();
}

void Close() {
	Renderer::Close();
	RendererAPI::Shutdown();
}

void BeginFrame() {
	RendererAPI::Get()->StartFrame();
	Renderer::BeginFrame();
	Renderer::Clear();
}

void Update(TimeStep ts) {
	float fps = (1.0f / (float)ts) * 1000.0f;
	Renderer::GetFrame().Info.FPS = fps;

}

void EndFrame() {
	Renderer::EndFrame();
	RendererAPI::Get()->EndFrame();
}

void RegisterInterface() {
	
}

}


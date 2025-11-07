#include "Renderer.h"

#include "Platform/RendererAPI.h"

namespace Magma::Graphics {

void Renderer::Init() {
#if defined(VOLCANIC_APPLE)
	RendererAPI::Create(RendererAPI::Backend::Metal);
#else
	RendererAPI::Create(RendererAPI::Backend::OpenGL);
#endif
}

void Renderer::Close() {
	RendererAPI::Shutdown();
}

void Renderer::StartFrame() {
	RendererAPI::Get()->StartFrame();

}

void Renderer::EndFrame() {

	RendererAPI::Get()->EndFrame();
}

}
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

void Renderer::BeginFrame() {
	RendererAPI::Get()->BeginFrame();

}

void Renderer::EndFrame() {

	RendererAPI::Get()->EndFrame();
}

void Renderer::DrawQuad(const Quad& quad) {
	auto cmd =
		RendererAPI::Get()->NewCommand({
			.Clear = true,
			.ClearColor = {
				quad.Color.r,
				quad.Color.g,
				quad.Color.b,
				quad.Color.a
			},
			.Scissor = true,
			.ScissorX = quad.PosX,
			.ScissorY = quad.PosY,
			.ScissorW = quad.Width,
			.ScissorH = quad.Height
		});
}

}
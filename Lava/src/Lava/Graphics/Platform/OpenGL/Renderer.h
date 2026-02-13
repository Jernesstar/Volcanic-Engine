#pragma once

#include "RendererAPI.h"

using namespace Lava;

namespace OpenGL {

class Renderer : public Graphics::RendererAPI {
public:
	Renderer();
	~Renderer() = default;

	Graphics::DrawBuffer* NewBuffer(const Graphics::DrawBufferSpec&) override;
	Graphics::DrawPass* NewPass(Graphics::DrawBuffer*) override;
	Graphics::DrawCommand* NewCommand(Graphics::DrawPass*) override;

private:
	void Init() override;
	void Close() override;
	void BeginFrame() override;
	void EndFrame() override;
};

}
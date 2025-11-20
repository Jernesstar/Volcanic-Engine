#pragma once

#include "RendererAPI.h"

using namespace Magma::Graphics;

namespace OpenGL {

class Renderer : public RendererAPI {
public:
	Renderer();
	~Renderer() = default;

	DrawBufferID NewBuffer(const DrawBufferSpec&) override;
	void SetBufferData(DrawBufferID, DrawBufferIndex, Buffer<void>) override;

	DrawPass* NewPass(DrawBufferID) override;
	DrawCommand* NewCommand(DrawPass*) override;
	DrawCall* NewCall(DrawCommand*) override;

private:
	void Init() override;
	void Close() override;
	void BeginFrame() override;
	void EndFrame() override;
};

}
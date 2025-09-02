#pragma once

#include <Graphics/RendererAPI.h>

using namespace Magma::Graphics;

namespace Magma::OpenGL {

class Renderer : public Graphics::RendererAPI {
public:
	Renderer();
	~Renderer() = default;

	void StartFrame() override;
	void EndFrame() override;
	DebugInfo GetDebugInfo() override;

	DrawBuffer* NewDrawBuffer(DrawBufferSpecification& specs,
							  void* data = nullptr) override;
	DrawBuffer* GetDrawBuffer(DrawBufferSpecification& specs) override;
	void SetBufferData(DrawBuffer* buffer, uint8_t bufferIndex,
		const void* data, uint64_t count, uint64_t offset = 0) override;
	void ReleaseBuffer(DrawBuffer* buffer) override;

	DrawPass* NewDrawPass(DrawBuffer* buffer,
		Ref<Graphics::ShaderPipeline> pipeline = nullptr,
		Ref<Graphics::Framebuffer> framebuffer = nullptr) override;

	DrawCommand* NewDrawCommand(DrawPass* pass) override;

private:
	void Init() override;
	void Close() override;
};

}
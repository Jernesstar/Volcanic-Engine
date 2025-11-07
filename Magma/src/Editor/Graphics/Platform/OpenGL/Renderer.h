#pragma once

#include "RendererAPI.h"

using namespace Magma::Graphics;

namespace OpenGL {

class Renderer : public RendererAPI {
public:
	Renderer();
	~Renderer() = default;

	void StartFrame() override;
	void EndFrame() override;

private:
	void Init() override;
	void Close() override;
};

}
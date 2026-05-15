#pragma once

#include <Engine/Scene/Scene.h>

#include "Renderer.h"

namespace VolcanicEngine {

enum class PipelineStage {
	PreDepthPrepass,
	PostDepthPrepass,
	PreGeometry,
	PostGeometry,
	PreShadows,
	PostShadows,
	PreSkybox,
	PostSkybox,
	PreTransparency,
	PostTransparency,
	PrePostProcess,
	PostPostProcess,
	PreUI,
	PostUI,
};

class RenderPipeline {
public:
	bool Render3D = true;
	bool Render2D = true;
	bool RenderCanvas = true;

public:
	RenderPipeline() = default;
	virtual ~RenderPipeline() = default;

	virtual void OnInit() = 0;
	virtual void OnRender(Scene* scene) = 0;
	virtual void OnResize(u32, u32) = 0;

	virtual Ref<Framebuffer> GetOutput() const = 0;
};

}
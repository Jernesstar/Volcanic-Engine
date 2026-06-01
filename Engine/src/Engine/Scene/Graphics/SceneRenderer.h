#pragma once

#include <VolcaniCore/Core/Defines.h>

#include "RenderPipeline.h"

using namespace VolcaniCore;

namespace VolcanicEngine {

class Scene;

class SceneRenderer {
public:
	SceneRenderer() = default;
	~SceneRenderer() = default;

	void OnSceneLoad();
	void OnSceneClose();

	void Render(Scene* scene, TimeStep ts);

	void SetPipeline(Ref<RenderPipeline> pipeline);
	void UseDefaultPipeline();
	void UseDefaultPipeline(u32 renderW, u32 renderH,
		u32 outputW = 1920, u32 outputH = 1080);

	Ref<RenderPipeline> GetPipeline() const { return m_Pipeline; }
	Ref<Framebuffer> GetOutput() const;

private:
	Ref<RenderPipeline> m_Pipeline;
};

}
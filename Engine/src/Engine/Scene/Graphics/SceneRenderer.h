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

	void Update(TimeStep ts);
	void Render(Scene* scene);

	void SetPipeline(Ref<RenderPipeline> pipeline);
	void UseDefaultPipeline();

	Ref<RenderPipeline> GetPipeline() const { return m_Pipeline; }
	Ref<Framebuffer> GetOutput() const;

private:
	Ref<RenderPipeline> m_Pipeline;
};

}
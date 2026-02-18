#pragma once

#include <VolcaniCore/Core/Buffer.h>
#include <Engine/Graphics/RenderPass.h>
#include <Engine/Graphics/Camera.h>
#include <Engine/Graphics/CameraController.h>

#include "ECS/Entity.h"

using namespace VolcaniCore;

using namespace VolcanicEngine::ECS;

namespace VolcanicEngine {

class Scene;

class SceneRenderer {
public:
	SceneRenderer() = default;
	virtual ~SceneRenderer() = default;

	virtual void Update(TimeStep ts) = 0;

	virtual void Begin() = 0;
	virtual void SubmitCamera(const Entity& entity) = 0;
	virtual void SubmitSkybox(const Entity& entity) = 0;
	virtual void SubmitLight(const Entity& entity) = 0;
	virtual void SubmitParticles(const Entity& entity) = 0;
	virtual void SubmitMesh(const Entity& entity) = 0;
	virtual void Render() = 0;

	Ref<Framebuffer> GetOutput() const { return m_Output; }

protected:
	Ref<Framebuffer> m_Output;
};

}
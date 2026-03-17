#pragma once

#include <VolcaniCore/Core/TimeUtils.h>
#include <VolcaniCore/Core/Buffer.h>

#include "Graphics/RenderPass.h"
#include "Graphics/Camera.h"
#include "ECS/Entity.h"

using namespace VolcaniCore;
using namespace VolcanicEngine::ECS;
using namespace VolcanicEngine::Graphics;

namespace VolcanicEngine {

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

	virtual void SubmitLayout(const Entity& entity) = 0;
	virtual void SubmitImage(const Entity& entity) = 0;
	virtual void SubmitText(const Entity& entity) = 0;
	virtual void SubmitButton(const Entity& entity) = 0;

	virtual void Render() = 0;

	Ref<Framebuffer> GetOutput() const { return m_Output; }

protected:
	Ref<Framebuffer> m_Output;
};

class RuntimeSceneRenderer : public SceneRenderer {
public:
	RuntimeSceneRenderer();
	~RuntimeSceneRenderer();

	void Update(TimeStep ts);

	void Begin();
	void SubmitCamera(const Entity& entity) override;
	void SubmitSkybox(const Entity& entity) override;
	void SubmitLight(const Entity& entity) override;
	void SubmitParticles(const Entity& entity) override;
	void SubmitMesh(const Entity& entity) override;

	void SubmitLayout(const Entity& entity)  override;
	void SubmitImage(const Entity& entity)   override;
	void SubmitText(const Entity& entity)    override;
	void SubmitButton(const Entity& entity)  override;

	void Render();

	void OnSceneLoad();
	void OnSceneClose();

private:
	void InitMips();
	void Downsample();
	void Upsample();
	void Composite();

	// Resolves the pixel-space rect for an entity, taking anchor and
	// parent bounds into account.
	Vec4 ResolveRect(const Entity& entity);

private:
	bool HasDirectionalLight = false;
	u32 SpotlightCount = 0;
	u32 PointLightCount = 0;
};

}
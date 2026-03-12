#pragma once

#include <VolcaniCore/Core/TimeUtils.h>
#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Defines.h>

#include "Graphics/Platform/Framebuffer.h"
#include "Graphics/Platform/Texture.h"
#include "ECS/Entity.h"

#include "Component.h"

using namespace VolcaniCore;
using namespace VolcanicEngine::ECS;
using namespace VolcanicEngine::Graphics;

namespace VolcanicEngine {

class CanvasRenderer {
public:
	CanvasRenderer() = default;
	virtual ~CanvasRenderer() = default;

	virtual void Update(TimeStep ts) = 0;

	virtual void Begin()                          = 0;
	virtual void SubmitLayout(const Entity& entity)  = 0;
	virtual void SubmitImage(const Entity& entity)   = 0;
	virtual void SubmitText(const Entity& entity)    = 0;
	virtual void SubmitButton(const Entity& entity)  = 0;
	virtual void Render()                         = 0;

	Ref<Framebuffer> GetOutput() const { return m_Output; }

protected:
	Ref<Framebuffer> m_Output;
};

class RuntimeCanvasRenderer : public CanvasRenderer {
public:
	RuntimeCanvasRenderer();
	~RuntimeCanvasRenderer();

	void Update(TimeStep ts) override;

	void Begin()                                   override;
	void SubmitLayout(const Entity& entity)  override;
	void SubmitImage(const Entity& entity)   override;
	void SubmitText(const Entity& entity)    override;
	void SubmitButton(const Entity& entity)  override;
	void Render()                                  override;

private:
	// Resolves the pixel-space rect for an entity, taking anchor and
	// parent bounds into account.
	Vec4 ResolveRect(const Entity& entity);

private:
	Vec2 m_ViewportSize = { 0.0f, 0.0f };
};

}
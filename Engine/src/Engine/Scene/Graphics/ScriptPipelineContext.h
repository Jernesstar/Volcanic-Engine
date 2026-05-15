#pragma once

#include <Engine/Scene/Scene.h>

#include "ScriptRenderPass.h"
#include "ScriptFramebuffer.h"
#include "DefaultRenderPipeline.h"

namespace VolcanicEngine {

class ScriptPipelineContext {
public:
	static ScriptPipelineContext* Factory(
		DefaultRenderPipeline* pipeline, Scene* scene)
	{
		return new ScriptPipelineContext(pipeline, scene);
	}

	void AddRef()  { m_refCount++; }
	void Release() { if(--m_refCount == 0) delete this; }

	// Read the pipeline's named buffers
	ScriptFramebuffer* GetBuffer(const std::string& name) {
		return ScriptFramebuffer::Wrap(m_pipeline->GetBuffer(name));
	}

	// Insert an extra pass at this stage
	void AddPass(ScriptRenderPass* pass) {
		Renderer::StartPass(pass->GetPass());
		// caller draws into it, then calls End()
	}

	Scene* GetScene() { return m_scene; }

private:
	int m_refCount = 1;
	DefaultRenderPipeline* m_pipeline;
	Scene* m_scene;
};

}
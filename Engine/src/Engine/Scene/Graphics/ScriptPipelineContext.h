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

public:
	ScriptPipelineContext(DefaultRenderPipeline* pipeline, Scene* scene)
	: m_Pipeline(pipeline), m_Scene(scene) { }

	~ScriptPipelineContext() { RestoreOutput(); }

	void AddRef() { m_RefCount++; }
	void Release() { if(--m_RefCount == 0) delete this; }

	Scene* GetScene() { return m_Scene; }

	ScriptFramebuffer* GetBuffer(const std::string& name) {
		return ScriptFramebuffer::Wrap(m_Pipeline->GetBuffer(name));
	}

	void RedirectOutput(ScriptFramebuffer* fb) {
		m_RedirectedBuffer = fb;
		if(fb) fb->AddRef();
	}

	void RestoreOutput() {
		if(m_RedirectedBuffer) {
			m_RedirectedBuffer->Release();
			m_RedirectedBuffer = nullptr;
		}
	}

	bool HasRedirection() const { return m_RedirectedBuffer != nullptr; }
	ScriptFramebuffer* GetRedirectedBuffer() const { return m_RedirectedBuffer; }

	void SuppressBlit() { m_SuppressBlit = true; }
	bool IsBlitSuppressed() const { return m_SuppressBlit; }

	void SetBloomThreshold(f32 t) { m_Pipeline->m_BloomThreshold = t; }
	void SetFilterRadius(f32 r) { m_Pipeline->m_FilterRadius = r; }
	void SetBloomRadius(int r) { m_Pipeline->m_FilterRadius = (f32)r / 1000.0f; }
	f32 GetBloomThreshold() const { return m_Pipeline->m_BloomThreshold; }
	f32 GetFilterRadius() const { return m_Pipeline->m_FilterRadius; }

	void SetSubPixelOffset(Vec2 offset) {
		m_SubPixelOffset = offset;
		m_HasOffset = true;
	}

	Vec2 GetSubPixelOffset() const {
		return m_SubPixelOffset;
	}
	bool HasSubPixelOffset() const {
		return m_HasOffset;
	}

	// ── Pass insertion ────────────────────────────────────────────────────────

	void AddPass(ScriptRenderPass* pass) {
		if(pass)
			Renderer::StartPass(pass->GetPass());
	}

private:
	int m_RefCount = 1;

	DefaultRenderPipeline* m_Pipeline;
	Scene* m_Scene;

	ScriptFramebuffer* m_RedirectedBuffer = nullptr;
	bool m_SuppressBlit = false;

	glm::vec2 m_SubPixelOffset = { 0.0f, 0.0f };
	bool m_HasOffset = false;
};

}
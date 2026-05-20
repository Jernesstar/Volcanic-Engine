#pragma once

#include <Engine/Graphics/Platform/RendererAPI.h>
#include <Engine/Graphics/Platform/Framebuffer.h>

using namespace VolcanicEngine::Graphics;

namespace VolcanicEngine {

class ScriptFramebuffer {
public:
	static ScriptFramebuffer* Factory() {
		return new ScriptFramebuffer();
	}

	static ScriptFramebuffer* Wrap(Ref<Framebuffer> fb) {
		return new ScriptFramebuffer(fb);
	}

	ScriptFramebuffer(Ref<Framebuffer> fb = nullptr) {
		m_Framebuffer = fb;
	}

	void AddRef() { m_RefCount++; }
	void Release() { if(--m_RefCount == 0) delete this; }

	void AddColorAttachment(u32 w, u32 h) {
		m_Spec.Attachments.Add({ AttachmentTarget::Color, w, h });
	}
	void AddDepthAttachment(u32 w, u32 h) {
		m_Spec.Attachments.Add({ AttachmentTarget::Depth, w, h });
	}

	Ref<Framebuffer> Resolve() {
		if(!m_Framebuffer)
			m_Framebuffer = RendererAPI::Get()->CreateFramebuffer(m_Spec);
		return m_Framebuffer;
	}

	// ScriptTexture* GetColor(u32 idx = 0);
	// ScriptTexture* GetDepth();

private:
	u32 m_RefCount = 1;
	FramebufferSpec m_Spec;
	Ref<Framebuffer> m_Framebuffer;
};

}
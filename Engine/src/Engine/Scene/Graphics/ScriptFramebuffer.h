#pragma once

#include <Engine/Graphics/Platform/RendererAPI.h>
#include <Engine/Graphics/Platform/Framebuffer.h>

using namespace VolcanicEngine::Graphics;

namespace VolcanicEngine {

class ScriptFramebuffer {
public:
	static ScriptFramebuffer* Factory(u32 w = 0, u32 h = 0) {
		return new ScriptFramebuffer(nullptr, w, h);
	}

	static ScriptFramebuffer* Wrap(Ref<Framebuffer> fb) {
		return new ScriptFramebuffer(fb);
	}

	void AddRef() { m_RefCount++; }
	void Release() { if(--m_RefCount == 0) delete this; }

	ScriptFramebuffer(Ref<Framebuffer> fb = nullptr, u32 w = 0, u32 h = 0)
		: m_Framebuffer(fb), m_DefaultWidth(w), m_DefaultHeight(h) { }

	// Zero-arg versions use the dimensions from the constructor
	void AddColorAttachment() {
		m_Spec.Attachments.Add({ AttachmentTarget::Color, m_DefaultWidth, m_DefaultHeight });
	}
	void AddDepthAttachment() {
		m_Spec.Attachments.Add({ AttachmentTarget::Depth, m_DefaultWidth, m_DefaultHeight });
	}
	// Explicit overrides still available
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

	ScriptTexture* GetColor(u32 idx = 0) {
		return ScriptTexture::WrapAttachment(m_Framebuffer,
			AttachmentTarget::Color, idx);
		}

	ScriptTexture* GetDepth() {
		return ScriptTexture::WrapAttachment(m_Framebuffer,
			AttachmentTarget::Depth);
	}

private:
	u32 m_RefCount = 1;
	u32 m_DefaultWidth = 0;
	u32 m_DefaultHeight = 0;
	FramebufferSpec  m_Spec;
	Ref<Framebuffer> m_Framebuffer;
};

}
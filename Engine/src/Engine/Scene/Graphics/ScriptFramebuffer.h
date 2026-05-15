#pragma once

#include "Platform/Framebuffer.h"

using namespace VolcanicEngine::Graphics;

namespace VolcanicEngine {

class ScriptFramebuffer {
public:
	static ScriptFramebuffer* Factory() {
		return new ScriptFramebuffer();
	}

	ScriptFramebuffer() {

	}

	void AddRef() { m_RefCount++; }
	void Release() { if(--m_RefCount == 0) delete this; }

	void AddColorAttachment(TextureFormat fmt) {
		// m_Spec.Attachments.Add({ AttachmentType::Color, fmt });
	}
	void AddDepthAttachment() {
		// m_Spec.Attachments.Add(
		// 	{ AttachmentType::Depth, TextureFormat::Depth24Stencil8 });
	}

	Ref<Framebuffer> Resolve() {
		// if(!m_Framebuffer)
		// 	m_Framebuffer = Framebuffer::Create(m_Spec);
		// return m_Framebuffer;
	}

	// ScriptTexture* GetColor(u32 idx = 0);
	// ScriptTexture* GetDepth();

private:
	u32 m_RefCount = 1;
	FramebufferSpec m_Spec;
	Ref<Framebuffer> m_Framebuffer;
};

}
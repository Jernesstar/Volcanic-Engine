#pragma once

#include "Platform/Texture.h"
#include "Platform/Framebuffer.h"

using namespace VolcanicEngine::Graphics;

namespace VolcanicEngine {

class ScriptTexture {
public:
	// Create a blank texture (script-owned)
	static ScriptTexture* Factory(u32 width, u32 height) {
		return new ScriptTexture(width, height);
	}

	// Wrap an existing engine texture (not script-owned, no-delete)
	static ScriptTexture* Wrap(Ref<Texture> texture) {
		if(!texture)
			return nullptr;
		return new ScriptTexture(texture);
	}

	// Wrap a framebuffer color attachment by index
	static ScriptTexture* WrapAttachment(Ref<Framebuffer> fb,
		AttachmentTarget target = AttachmentTarget::Color, u32 idx = 0)
	{
		if(!fb || !fb->Has(target))
			return nullptr;
		return new ScriptTexture(fb->Get(target, idx));
	}

public:
	void AddRef() { m_RefCount++; }
	void Release() {
		if(--m_RefCount == 0)
			delete this;
	}

	u32 GetWidth() const {
		if(m_Texture)
			return m_Texture->Spec.Width;
		return 0;
	}

	u32 GetHeight() const {
		if(m_Texture)
			return m_Texture->Spec.Height;
		return 0;
	}

	// Consume the next available binding slot (auto-increments per frame)
	u32 NextSlot() { return m_SlotCounter++; }

	void ResetSlot() { m_SlotCounter = 0; }

	Ref<Texture> GetTexture() const { return m_Texture; }
	Ref<Attachment> GetAttachment() const { return m_Attachment; }

	// Returns true if backed by a raw Texture rather than a Framebuffer attachment
	bool IsRawTexture() const { return m_Texture != nullptr; }

private:
	// Script-created blank texture
	ScriptTexture(u32 width, u32 height) {
		m_Texture = RendererAPI::Get()->CreateTexture({ width, height });
	}

	// Wrap an existing Texture
	explicit ScriptTexture(Ref<Texture> texture)
		: m_Texture(texture) { }

	// Wrap a Framebuffer attachment
	explicit ScriptTexture(Ref<Attachment> attachment)
		: m_Attachment(attachment) { }

private:
	u32 m_RefCount = 1;
	u32 m_SlotCounter = 0;

	Ref<Texture> m_Texture;
	Ref<Attachment> m_Attachment;
};

}
#pragma once

#include <glad/glad.h>

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

#include <Platform/Framebuffer.h>

#include "Texture.h"

using namespace VolcaniCore;
using namespace Magma;

namespace OpenGL {

class Framebuffer;

class Attachment : public Graphics::Attachment {
public:
	enum class Type { Texture, RenderBuffer };

public:
	Attachment(Graphics::AttachmentTarget target, Attachment::Type type,
			   u32 width = 0, u32 height = 0, u32 id = 0)
		: Graphics::Attachment(target),
			m_Type(type), m_Width(width), m_Height(height), m_RendererID(id)
	{
		
	}
	~Attachment() {
		if(m_Type == Attachment::Type::Texture)
			glDeleteTextures(1, &m_RendererID);
		else if(m_Type == Attachment::Type::RenderBuffer)
			glDeleteRenderbuffers(1, &m_RendererID);
	}

	void Bind(u32 slot = 0) const {
		if(this->GetType() == Attachment::Type::Texture)
			glBindTextureUnit(slot, m_RendererID);
		else if(this->GetType() == Attachment::Type::RenderBuffer)
			glBindRenderbuffer(GL_RENDERBUFFER, m_RendererID);
	}

	Attachment::Type GetType() const { return m_Type; }
	u32 GetWidth() const { return m_Width; }
	u32 GetHeight() const { return m_Height; }
	u32 GetRendererID() const { return m_RendererID; };

private:
	Attachment::Type m_Type;
	u32 m_Width, m_Height;
	u32 m_RendererID;

	friend class Framebuffer;
};

class Framebuffer : public Graphics::Framebuffer {
public:
	Framebuffer(const Graphics::FramebufferSpec& spec)
		: Graphics::Framebuffer(spec)
	{
		glGenFramebuffers(1, &m_BufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);

		m_Attachments.Add(
			CreateRef<Attachment>(Graphics::AttachmentTarget::Color,
								  Attachment::Type::Texture)
		);
		m_Attachments.Add(
			CreateRef<Attachment>(Graphics::AttachmentTarget::Depth,
								  Attachment::Type::Texture)
		);

		CreateColorAttachment();
		CreateDepthAttachment();

		uint32_t atts[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, atts);

		VOLCANICORE_ASSERT(
			glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// // glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);
		// glFramebufferTexture2D(GL_FRAMEBUFFER, type + dst, GL_TEXTURE_2D, id, 0);
		// // glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	~Framebuffer() {
		m_Attachments.Clear();
		glDeleteBuffers(1, &m_BufferID);
	}

	void Bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID); }
	void Unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

	bool Has(Graphics::AttachmentTarget t) const override {
		for(auto& att : m_Attachments) {
			if(att->Target == t)
				return true;
		}

		return false;
	}

	void Add(Graphics::AttachmentTarget t, Ref<Graphics::Attachment> att) override {

	}

	void Attach(Graphics::AttachmentTarget t, u32 idx, u32 dst) override {

	}

	Ref<Graphics::Attachment> Get(Graphics::AttachmentTarget t, u32 idx = 0) const override {

		return nullptr;
	}

	Ref<Attachment> GetAttachment(Graphics::AttachmentTarget t, u32 idx = 0) {
		u32 i = 0;
		for(auto att : m_Attachments) {
			if(att->Target == t && i++ == idx)
				return att;
		}

		return nullptr;
	}

private:
	void CreateColorAttachment(u32 index = 0) {

	}

	void CreateDepthAttachment() {

	}

	void CreateStencilAttachment() {

	}

	void CreateDepthStencilAttachment() {

	}

	List<Ref<Attachment>> m_Attachments;
	u32 m_BufferID;
};

}
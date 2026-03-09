#pragma once

#include <glad/glad.h>

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

#include <Platform/Framebuffer.h>

#include "Texture.h"

using namespace VolcaniCore;
using namespace VolcanicEngine;

namespace OpenGL {

class Framebuffer;

class Attachment : public Graphics::Attachment {
public:
	enum class Type { Texture, RenderBuffer };

public:
	Attachment(const Graphics::AttachmentSpec& spec, Attachment::Type type,
			   u32 index = 0)
		: Graphics::Attachment(spec), m_Type(type)
	{
		Create(spec, index);
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
	u32 GetWidth() const { return Spec.Width; }
	u32 GetHeight() const { return Spec.Height; }
	u32 GetRendererID() const { return m_RendererID; };

private:
	Attachment::Type m_Type;
	u32 m_RendererID;

private:
	void Create(const Graphics::AttachmentSpec& spec, u32 index) {
		uint32_t width  = spec.Width;
		uint32_t height = spec.Height;

		u32 internalFormat;
		u32 filter;
		if(spec.Target == Graphics::AttachmentTarget::Color) {
			internalFormat = GL_RGBA8;
			filter = GL_LINEAR;
		}
		else if(spec.Target == Graphics::AttachmentTarget::Depth) {
			internalFormat = GL_DEPTH_COMPONENT32F;
			filter = GL_LINEAR;
		}
		else if(spec.Target == Graphics::AttachmentTarget::Stencil) {
			internalFormat = GL_STENCIL_INDEX8;
			filter = GL_LINEAR;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat,
						   spec.Width, spec.Height);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, filter);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, filter);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		u32 type;
		if(spec.Target == Graphics::AttachmentTarget::Color) {
			if(index > 0)
				return;

			type = GL_COLOR_ATTACHMENT0;
		}
		else if(spec.Target == Graphics::AttachmentTarget::Depth)
			type = GL_DEPTH_ATTACHMENT;
		else if(spec.Target == Graphics::AttachmentTarget::Stencil)
			type = GL_STENCIL_ATTACHMENT;

		glFramebufferTexture2D(GL_FRAMEBUFFER, type + index,
								GL_TEXTURE_2D, m_RendererID, 0);
	}
};

class Framebuffer : public Graphics::Framebuffer {
public:
	Framebuffer(const Graphics::FramebufferSpec& spec)
		: Graphics::Framebuffer(spec)
	{
		glGenFramebuffers(1, &m_BufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);

		u32 colorCount = 0;
		for(const auto& attSpec : spec.Attachments) {
			auto att =
				CreateRef<Attachment>(attSpec, Attachment::Type::Texture,
									  m_Attachments.Count());
			m_Attachments.Add(att);
			if(attSpec.Target == Graphics::AttachmentTarget::Color)
				colorCount++;
		}

		if(colorCount == 0) {
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}
		else {
			u32* atts = new u32[colorCount];
			for(u32 i = 0; i < colorCount; i++)
				atts[i] = GL_COLOR_ATTACHMENT0 + i;
			glDrawBuffers(1, atts);
			delete[] atts;
		}

		if(spec.EnableRead) {
			auto att = GetAttachment(Graphics::AttachmentTarget::Color, 0);
			glGenBuffers(2, m_PixelBuffers);
			for(int i = 0; i < 2; i++) {
				glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PixelBuffers[i]);
				glBufferData(GL_PIXEL_PACK_BUFFER,
							 att->GetWidth() * att->GetHeight() * 4,
							 nullptr, GL_STREAM_READ);
			}
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		}

		VOLCANICORE_ASSERT(
			glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
			"[OpenGL]: Framebuffer is not complete!");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	~Framebuffer() {
		m_Attachments.Clear();
		glDeleteBuffers(1, &m_BufferID);
	}

	void Bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID); }
	void Unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

	bool Has(Graphics::AttachmentTarget t) const override {
		for(auto& att : m_Attachments) {
			if(att->Spec.Target == t)
				return true;
		}

		return false;
	}

	void Attach(Graphics::AttachmentTarget t, u32 idx, u32 dst) override {
		u32 type;
		switch(t) {
			case Graphics::AttachmentTarget::Color:
				type = GL_COLOR_ATTACHMENT0;
				break;
			case Graphics::AttachmentTarget::Depth:
				type = GL_DEPTH_ATTACHMENT;
				idx = 0;
				break;
			case Graphics::AttachmentTarget::Stencil:
				type = GL_STENCIL_ATTACHMENT;
				idx = 0;
				break;
		}

		auto att = GetAttachment(t, idx);
		u32 id = att->GetRendererID();

		glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, type + dst, GL_TEXTURE_2D, id, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	Buffer<u8> GetPixels() override {
		auto att = GetAttachment(Graphics::AttachmentTarget::Color, 0);
		auto width = att->GetWidth();
		auto height = att->GetHeight();
		int size = width * height * 4;
		int readIndex  =  m_PixelIndex;
		int writeIndex = !m_PixelIndex;

		// Kick off async readback into writeIndex PBO
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_BufferID);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PixelBuffers[writeIndex]);
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		// Map readIndex PBO — GPU finished this one last frame
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PixelBuffers[readIndex]);
		auto* ptr = (u8*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

		Buffer<u8> buf(size);
		if(ptr)
			buf.Add(ptr, size);
		else
			Log::Error("Could not retrieve pixel data!");

		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		m_PixelIndex = writeIndex;

		return buf;
	}

	Ref<Graphics::Attachment> Get(Graphics::AttachmentTarget t, u32 idx = 0) const override {
		return GetAttachment(t, idx);
	}

	Ref<Attachment> GetAttachment(Graphics::AttachmentTarget t, u32 idx = 0) const {
		u32 i = 0;
		for(auto att : m_Attachments) {
			if(att->Spec.Target == t && i++ == idx)
				return att;
		}

		return nullptr;
	}

private:
	List<Ref<Attachment>> m_Attachments;
	u32 m_BufferID;
	u32 m_PixelBuffers[2];
	u32 m_PixelIndex = 0;
};

}
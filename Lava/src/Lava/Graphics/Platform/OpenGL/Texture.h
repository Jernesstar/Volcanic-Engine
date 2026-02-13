#pragma once

#include <glad/glad.h>

#include "Platform/Texture.h"

using namespace Lava;

namespace OpenGL {

static constexpr u32 GetType(Graphics::TextureType type) {
	switch(type) {
		case Graphics::TextureType::RGBA:
			return GL_RGBA;
		case Graphics::TextureType::Depth:
			return GL_DEPTH_COMPONENT;
	}
}

static constexpr u32 GetFormat(Graphics::TextureFormat format) {
	switch(format) {
		case Graphics::TextureFormat::Normal:
			return GL_RGBA8;
		case Graphics::TextureFormat::Float:
			return GL_RGBA16F;
		case Graphics::TextureFormat::Depth:
			return GL_DEPTH_COMPONENT32F;
	}

	return GL_RGBA8;
}

class Texture : public Graphics::Texture {
public:
	Texture(const Graphics::TextureSpec& spec)
		: Graphics::Texture(spec)
	{
		auto internalFormat = GetFormat(spec.Format);
		auto filter =
			spec.Sampling == Graphics::TextureSampling::Linear ?
													 GL_LINEAR : GL_NEAREST;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
		glTextureStorage2D(m_TextureID, 1, internalFormat,
						   spec.Width, spec.Height);
		glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, filter);
		glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, filter);
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	}

	~Texture() {
		glDeleteTextures(1, &m_TextureID);
	}

	void Bind(u32 slot = 0) {
		glBindTextureUnit(slot, m_TextureID);
	}

	void SetData(const void* data) override {
		glTextureSubImage2D(m_TextureID, 0, 0, 0, Spec.Width, Spec.Height,
							GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	u32 GetID() const { return m_TextureID; }

private:
	u32 m_TextureID;
};

}
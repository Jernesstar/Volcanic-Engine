#pragma once

#include <glad/glad.h>

#include "Platform/Texture.h"

using namespace Magma;

namespace OpenGL {

inline constexpr u32 GetType(Graphics::Texture::Type type) {
	switch(type) {
		case Graphics::Texture::Type::RGBA:
			return GL_RGBA;
		case Graphics::Texture::Type::Depth:
			return GL_DEPTH_COMPONENT;
	}
}

inline constexpr u32 GetFormat(Graphics::Texture::Format format) {
	switch(format) {
		case Graphics::Texture::Format::Normal:
			return GL_RGBA8;
		case Graphics::Texture::Format::Float:
			return GL_RGBA16F;
		case Graphics::Texture::Format::Depth:
			return GL_DEPTH_COMPONENT32F;
	}

	return GL_RGBA8;
}

class Texture : public Graphics::Texture {
public:
	Texture(u32 width, u32 height, Format format, Sampling sampling) {
		auto internalFormat = GetFormat(format);
		auto filter = sampling == Sampling::Linear ? GL_LINEAR : GL_NEAREST;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
		glTextureStorage2D(m_TextureID, 1, internalFormat, width, height);
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
		glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Width, m_Height,
							GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	u32 GetID() const { return m_TextureID; }

private:
	u32 m_TextureID;
};

}
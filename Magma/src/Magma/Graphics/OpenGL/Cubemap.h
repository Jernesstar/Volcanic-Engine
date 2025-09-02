#pragma once

#include "Graphics/Cubemap.h"

using namespace Magma::Graphics;

namespace Magma::OpenGL {

class Cubemap : public Magma::Graphics::Cubemap {
public:
	Cubemap(const List<ImageData>& faces);
	~Cubemap();

	void Bind() const;
	void Unbind() const;

private:
	uint32_t m_TextureID;
};

}
#include "Quad.h"

using namespace VolcaniCore;

namespace Lava::Graphics {

Ref<Quad> Quad::Create(Ref<Texture> texture) {
	return CreateRef<Quad>(texture);
}

Ref<Quad> Quad::Create(uint32_t width, uint32_t height, const glm::vec4& color)
{
	return CreateRef<Quad>(width, height, color);
}

}
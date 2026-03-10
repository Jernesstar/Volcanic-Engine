#pragma once

#include "Platform/RendererAPI.h"

namespace VolcanicEngine::Graphics {

struct Material {
	Ref<Shader> ShaderRef;
	DrawUniforms UniformData;
};

}
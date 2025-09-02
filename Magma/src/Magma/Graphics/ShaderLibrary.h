#pragma once

#include "Graphics/Shader.h"

using namespace VolcaniCore;

namespace Magma::Graphics {

class ShaderLibrary {
public:
	static void Add(const std::string& name, Ref<ShaderPipeline> shader);
	static Ref<ShaderPipeline> Get(const std::string& name);
	static void Clear();
};

}
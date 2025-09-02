#include "Shader.h"

#include "OpenGL/Shader.h"

#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/FileUtils.h>

#include "Graphics/RendererAPI.h"

using namespace VolcaniCore;

namespace Magma::Graphics {

Ref<ShaderPipeline> ShaderPipeline::Create(const List<Shader>& shaders) {
	RendererAPI::Backend backend = RendererAPI::Get()->GetBackend();

	switch(backend) {
		case RendererAPI::Backend::OpenGL:
			return CreateRef<OpenGL::ShaderProgram>(shaders);
	}

	return nullptr;
}

}
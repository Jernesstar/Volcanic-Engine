#include "RendererAPI.h"

#include <VolcaniCore/Core/Assert.h>

#include "Platform/OpenGL/Renderer.h"
#include "Platform/OpenGL/Framebuffer.h"
#include "Platform/OpenGL/Shader.h"
#include "Platform/OpenGL/StorageBuffer.h"
#include "Platform/OpenGL/UniformBuffer.h"

namespace Magma::Graphics {

void RendererAPI::Create(RendererAPI::Backend backend) {
	switch(backend) {
		case RendererAPI::Backend::OpenGL:
			s_Instance = CreateRef<OpenGL::Renderer>();
			break;
		// case RendererAPI::Backend::Metal:
		// 	s_Instance = CreateRef<Metal::Renderer>();
		// 	break;
		default:
			VOLCANICORE_ASSERT(false, "Unknown renderer API");
			return;
	}

	s_Instance->Init();
}

void RendererAPI::Shutdown() {
	s_Instance->Close();
	s_Instance.reset();
}

Ref<Framebuffer> RendererAPI::CreateFramebuffer(const FramebufferSpecification& spec) {
	// switch(GetBackend()) {
	// 	case RendererAPI::Backend::OpenGL:
	// 		return CreateRef<OpenGL::Framebuffer>(spec);
	// 	// case RendererAPI::Backend::Metal:
	// 	// 	return CreateRef<Metal::Framebuffer>(spec);
	// 	default:
	// 		VOLCANICORE_ASSERT(false, "Unknown renderer API");
	// 		return nullptr;
	// }
}

Ref<Shader> RendererAPI::CreateShader(const ShaderSpecification& spec) {
	// switch(GetBackend()) {
	// 	case RendererAPI::Backend::OpenGL:
	// 		return CreateRef<OpenGL::Shader>(spec);
	// 	// case RendererAPI::Backend::Metal:
	// 	// 	return CreateRef<Metal::Shader>(spec);
	// 	default:
	// 		VOLCANICORE_ASSERT(false, "Unknown renderer API");
	// 		return nullptr;
	// }
}

Ref<Texture> RendererAPI::CreateTexture(const TextureSpecification& spec) {
	// switch(GetBackend()) {
	// 	case RendererAPI::Backend::OpenGL:
	// 		return CreateRef<OpenGL::Texture>(spec);
	// 	// case RendererAPI::Backend::Metal:
	// 	// 	return CreateRef<Metal::Texture>(spec);
	// 	default:
	// 		VOLCANICORE_ASSERT(false, "Unknown renderer API");
	// 		return nullptr;
	// }
}

Ref<StorageBuffer> RendererAPI::CreateStorageBuffer(const StorageBufferSpecification& spec) {
	// switch(GetBackend()) {
	// 	case RendererAPI::Backend::OpenGL:
	// 		return CreateRef<OpenGL::StorageBuffer>(spec);
	// 	// case RendererAPI::Backend::Metal:
	// 	// 	return CreateRef<Metal::StorageBuffer>(spec);
	// 	default:
	// 		VOLCANICORE_ASSERT(false, "Unknown renderer API");
	// 		return nullptr;
	// }
}

Ref<UniformBuffer> RendererAPI::CreateUniformBuffer(const UniformBufferSpecification& spec) {
	// switch(GetBackend()) {
	// 	case RendererAPI::Backend::OpenGL:
	// 		return CreateRef<OpenGL::UniformBuffer>(spec);
	// 	// case RendererAPI::Backend::Metal:
	// 	// 	return CreateRef<Metal::UniformBuffer>(spec);
	// 	default:
	// 		VOLCANICORE_ASSERT(false, "Unknown renderer API");
	// 		return nullptr;
	// }
}

}
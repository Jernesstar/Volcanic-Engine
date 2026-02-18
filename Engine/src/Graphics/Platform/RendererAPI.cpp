#include "RendererAPI.h"

#include <VolcaniCore/Core/Assert.h>

#include "Platform/OpenGL/Renderer.h"
#include "Platform/OpenGL/Framebuffer.h"
#include "Platform/OpenGL/Shader.h"
#include "Platform/OpenGL/StorageBuffer.h"
#include "Platform/OpenGL/UniformBuffer.h"

namespace VolcanicEngine::Graphics {

void RendererAPI::Create(RendererBackend backend) {
	switch(backend) {
		case RendererBackend::OpenGL:
			s_Instance = CreateRef<OpenGL::Renderer>();
			break;
		// case RendererBackend::Metal:
		// 	s_Instance = CreateRef<Metal::Renderer>();
		// 	break;
		// case RendererBackend::DirectX:
		// 	s_Instance = CreateRef<DirectX::Renderer>();
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

Ref<Framebuffer> RendererAPI::CreateFramebuffer(const FramebufferSpec& spec) {
	switch(GetBackend()) {
		case RendererBackend::OpenGL:
			return CreateRef<OpenGL::Framebuffer>(spec);
		// case RendererBackend::Metal:
		// 	return CreateRef<Metal::Framebuffer>(spec);
		default:
			VOLCANICORE_ASSERT(false, "Unknown renderer API");
			return nullptr;
	}
}

Ref<Shader> RendererAPI::CreateShader(const ShaderSpec& spec) {
	switch(GetBackend()) {
		case RendererBackend::OpenGL:
			return CreateRef<OpenGL::Shader>(spec);
		// case RendererBackend::Metal:
		// 	return CreateRef<Metal::Shader>(spec);
		default:
			VOLCANICORE_ASSERT(false, "Unknown renderer API");
			return nullptr;
	}
}

Ref<Texture> RendererAPI::CreateTexture(const TextureSpec& spec) {
	switch(GetBackend()) {
		case RendererBackend::OpenGL:
			return CreateRef<OpenGL::Texture>(spec);
		// case RendererBackend::Metal:
		// 	return CreateRef<Metal::Texture>(spec);
		default:
			VOLCANICORE_ASSERT(false, "Unknown renderer API");
			return nullptr;
	}
}

Ref<UniformBuffer> RendererAPI::CreateUniformBuffer(const UniformBufferSpec& spec) {
	switch(GetBackend()) {
		case RendererBackend::OpenGL:
			return CreateRef<OpenGL::UniformBuffer>(spec);
		// case RendererBackend::Metal:
		// 	return CreateRef<Metal::UniformBuffer>(spec);
		default:
			VOLCANICORE_ASSERT(false, "Unknown renderer API");
			return nullptr;
	}
}

Ref<StorageBuffer> RendererAPI::CreateStorageBuffer(const StorageBufferSpec& spec) {
	switch(GetBackend()) {
		case RendererBackend::OpenGL:
			return CreateRef<OpenGL::StorageBuffer>(spec);
		// case RendererBackend::Metal:
		// 	return CreateRef<Metal::StorageBuffer>(spec);
		default:
			VOLCANICORE_ASSERT(false, "Unknown renderer API");
			return nullptr;
	}
}

}
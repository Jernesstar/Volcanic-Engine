#pragma once

#include <VolcaniCore/Core/Defines.h>

#include "Framebuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "StorageBuffer.h"
#include "UniformBuffer.h"

namespace Magma::Graphics {

class RendererAPI {
public:
	enum class Backend { OpenGL, Metal };

public:
	static void Create(RendererAPI::Backend backend);
	static void Shutdown();

	static Ref<RendererAPI> Get() { return s_Instance; }

public:
	RendererAPI(RendererAPI::Backend backend)
		: m_Backend(backend) { }
	virtual ~RendererAPI() = default;

	virtual void StartFrame() = 0;
	virtual void EndFrame() = 0;

	static RendererAPI::Backend GetBackend() {
		return s_Instance->m_Backend;
	}

	Ref<Framebuffer> CreateFramebuffer(const FramebufferSpecification& spec);
	Ref<Shader> CreateShader(const ShaderSpecification& spec);
	Ref<Texture> CreateTexture(const TextureSpecification& spec);
	Ref<StorageBuffer> CreateStorageBuffer(const StorageBufferSpecification& spec);
	Ref<UniformBuffer> CreateUniformBuffer(const UniformBufferSpecification& spec);

protected:
	const RendererAPI::Backend m_Backend;

protected:
	virtual void Init() = 0;
	virtual void Close() = 0;

private:
	inline static Ref<RendererAPI> s_Instance;
};

}
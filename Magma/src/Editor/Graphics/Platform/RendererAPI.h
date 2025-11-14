#pragma once

#include <VolcaniCore/Core/Defines.h>

#include "Framebuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "StorageBuffer.h"
#include "UniformBuffer.h"

namespace Magma::Graphics {

enum DrawBufferIndex : u8 { Vertex, Index, Instance };
typedef u8 DrawBufferID;
typedef u8 DrawPassID;
typedef u8 DrawCommandID;

struct DrawBufferSpec {
	u32 IndexCount;
	u32 VertexCount;
	u32 InstanceCount;
	BufferLayout Vertex;
	BufferLayout Instance;
};

struct DrawPassSpec {
	DrawBufferID Buffer;
	Ref<Framebuffer> Output;
	Ref<Shader> Pipeline;
};

struct DrawCommandSpec {
	DrawPassID Pass;

	bool Clear = false;
	Vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	bool Scissor = false;
	f32 ScissorX = 0.0f, ScissorY = 0.0f, ScissorW = 0.0f, ScissorH = 0.0f;
};

class RendererAPI {
public:
	enum class Backend { OpenGL, Metal };

public:
	virtual DrawBufferID NewBuffer(const DrawBufferSpec&) = 0;
	virtual void SetBufferData(DrawBufferID, DrawBufferIndex, Buffer<void> data) = 0;

	virtual DrawPassID NewPass(const DrawPassSpec&) = 0;
	virtual DrawCommandID NewCommand(const DrawCommandSpec&) = 0;

public:
	static Ref<RendererAPI> Get() { return s_Instance; }
	static RendererAPI::Backend GetBackend() { return Get()->m_Backend; }

	static Ref<Framebuffer> CreateFramebuffer(const FramebufferSpecification&);
	static Ref<Shader> CreateShader(const ShaderSpecification&);
	static Ref<Texture> CreateTexture(const TextureSpecification&);
	static Ref<StorageBuffer> CreateStorageBuffer(const StorageBufferSpecification&);
	static Ref<UniformBuffer> CreateUniformBuffer(const UniformBufferSpecification&);

protected:
	const RendererAPI::Backend m_Backend;

protected:
	RendererAPI(RendererAPI::Backend backend)
		: m_Backend(backend) { }
	virtual ~RendererAPI() = default;

	virtual void Init() = 0;
	virtual void Close() = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;

private:
	inline static Ref<RendererAPI> s_Instance;

private:
	static void Create(RendererAPI::Backend backend);
	static void Shutdown();

	friend class Renderer;
};

}
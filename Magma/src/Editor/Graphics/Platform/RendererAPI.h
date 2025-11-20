#pragma once

#include <VolcaniCore/Core/Defines.h>

#include "Framebuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "StorageBuffer.h"
#include "UniformBuffer.h"

namespace Magma::Graphics {

enum DrawBufferIndex : u8 {
	Vertex, Index, Instance
};

struct DrawBufferSpec {
	u32 IndexCount;
	u32 VertexCount;
	u32 InstanceCount;
	BufferLayout Vertex;
	BufferLayout Instance;
};

class DrawBuffer {
public:
	const DrawBufferSpec Spec;

public:
	DrawBuffer(const DrawBufferSpec& spec)
		: Spec(spec) { };
	virtual ~DrawBuffer() = default;

	virtual void SetData(DrawBufferIndex, Buffer<void> data) = 0;
	virtual void Clear() = 0;
};

struct DrawPass {
	DrawBuffer* Buffer;
	Ref<Framebuffer> Output;
	Ref<Shader> Pipeline;
};

struct TextureSlot {
	Ref<Texture> Sampler = nullptr;
	uint32_t Index = 0;
};

struct UniformSlot {
	Ref<UniformBuffer> Buffer = nullptr;
	std::string Name = "";
	uint32_t Binding = 0;
};

struct StorageSlot {
	Ref<StorageBuffer> Buffer = nullptr;
	std::string Name = "";
	uint32_t Binding = 0;
};

struct DrawUniforms {
	Map<std::string, i32> IntUniforms;
	Map<std::string, f32> FloatUniforms;
	Map<std::string, TextureSlot> TextureUniforms;

	Map<std::string, Vec2> Vec2Uniforms;
	Map<std::string, Vec3> Vec3Uniforms;
	Map<std::string, Vec4> Vec4Uniforms;

	Map<std::string, Mat2> Mat2Uniforms;
	Map<std::string, Mat3> Mat3Uniforms;
	Map<std::string, Mat4> Mat4Uniforms;

	List<UniformSlot> UniformBuffers;
	List<StorageSlot> StorageBuffers;

	operator bool () const {
		return UniformBuffers || StorageBuffers
		|| IntUniforms.size() || FloatUniforms.size() || TextureUniforms.size()
		|| Vec2Uniforms.size() || Vec3Uniforms.size() || Vec4Uniforms.size()
		|| Mat2Uniforms.size() || Mat3Uniforms.size() || Mat4Uniforms.size();
	}

	void SetInput(const std::string& name, i32 data) {
		IntUniforms[name] = data;
	}
	void SetInput(const std::string& name, f32 data) {
		FloatUniforms[name] = data;
	}
	void SetInput(const std::string& name, const TextureSlot& data) {
		TextureUniforms[name] = data;
	}
	void SetInput(const std::string& name, const Vec2& data) {
		Vec2Uniforms[name] = data;
	}
	void SetInput(const std::string& name, const Vec3& data) {
		Vec3Uniforms[name] = data;
	}
	void SetInput(const std::string& name, const Vec4& data) {
		Vec4Uniforms[name] = data;
	}
	void SetInput(const std::string& name, const Mat2& data) {
		Mat2Uniforms[name] = data;
	}
	void SetInput(const std::string& name, const Mat3& data) {
		Mat3Uniforms[name] = data;
	}
	void SetInput(const std::string& name, const Mat4& data) {
		Mat4Uniforms[name] = data;
	}
	void SetInput(const UniformSlot& data) {
		UniformBuffers.Add(data);
	}
	void SetInput(const StorageSlot& data) {
		StorageBuffers.Add(data);
	}
};

enum class DepthTestingMode { On, Off };
enum class BlendingMode { Off, Greatest, Additive };
enum class CullingMode { Off, Front, Back };

enum class DrawPrimitive { Point, Line, Triangle, Cubemap };
enum class DrawPartition { Single, Instanced, MultiDraw };

struct DrawCall;

// A draw command is a series of draw calls
// With a shared set of uniforms, usually utilized for
// a specific piece of geometry
struct DrawCommand {
	DrawPass* Pass;

	DrawUniforms Uniforms;
	List<DrawCall> DrawCalls;

	// The buffer domain of the draw command
	u32 VerticesIndex = 0;
	u32 IndicesIndex  = 0;
	u32 InstancesIndex = 0;

	bool Clear = false;
	Vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	bool Scissor = false;
	f32 ScissorX  = 0.0f, ScissorY  = 0.0f, ScissorW  = 0.0f, ScissorH  = 0.0f;

	bool Viewport = false;
	f32 ViewportX = 0.0f, ViewportY = 0.0f, ViewportW = 0.0f, ViewportH = 0.0f;

	bool Compute = false;
	u32 ComputeX = 0, ComputeY = 0, ComputeZ = 0;

	DepthTestingMode DepthTesting = DepthTestingMode::On;
	BlendingMode Blending = BlendingMode::Off;
	CullingMode Culling = CullingMode::Off;

	DrawCall* NewCall() {
		return &DrawCalls.Emplace();
	}
};

struct DrawCall {
	DrawCommand* Command;

	u32 IndexOffset    = 0;
	u32 IndexCount     = 0;
	u32 VertexOffset   = 0;
	u32 VertexCount    = 0;
	u32 InstanceOffset = 0;
	u32 InstanceCount  = 0;

	DrawPrimitive Primitive = DrawPrimitive::Triangle;
	DrawPartition Partition = DrawPartition::Single;
};

enum class RendererBackend { OpenGL, Metal, DirectX };

class RendererAPI {
public:
	static Ref<RendererAPI> Get() { return s_Instance; }
	static RendererBackend GetBackend() { return s_Backend; }

	static Ref<Framebuffer> CreateFramebuffer(const FramebufferSpec&);
	static Ref<Shader> CreateShader(const ShaderSpec&);
	static Ref<Texture> CreateTexture(const TextureSpec&);
	static Ref<UniformBuffer> CreateUniformBuffer(const UniformBufferSpec&);
	static Ref<StorageBuffer> CreateStorageBuffer(const StorageBufferSpec&);

protected:
	RendererAPI() = default;
	virtual ~RendererAPI() = default;

	virtual void Init() = 0;
	virtual void Close() = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;

	virtual DrawBuffer* NewBuffer(const DrawBufferSpec&) = 0;
	virtual DrawPass* NewPass(DrawBuffer*) = 0;
	virtual DrawCommand* NewCommand(DrawPass*) = 0;

private:
	inline static Ref<RendererAPI> s_Instance;
	inline static RendererBackend s_Backend;

private:
	static void Create(RendererBackend backend);
	static void Shutdown();

	friend class Renderer;
};

}
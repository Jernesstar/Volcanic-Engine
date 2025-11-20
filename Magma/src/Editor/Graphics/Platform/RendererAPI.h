#pragma once

#include <VolcaniCore/Core/Defines.h>

#include "Framebuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "StorageBuffer.h"
#include "UniformBuffer.h"

namespace Magma::Graphics {

enum DrawBufferIndex : u8 { Vertex, Index, Instance };
typedef u32 DrawBufferID;

struct DrawBufferSpec {
	u32 IndexCount;
	u32 VertexCount;
	u32 InstanceCount;
	BufferLayout Vertex;
	BufferLayout Instance;
};

struct DrawPass {
	DrawBufferID Buffer;
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

struct DrawCommand {
	DrawPass* Pass;

	DrawUniforms Uniforms;

	bool Clear = false;
	Vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	bool Scissor = false;
	f32 ScissorX  = 0.0f, ScissorY  = 0.0f, ScissorW  = 0.0f, ScissorH  = 0.0f;
	f32 ViewportX = 0.0f, ViewportY = 0.0f, ViewportW = 0.0f, ViewportH = 0.0f;
	u32 ComputeX = 0, ComputeY = 0, ComputeZ = 0;

	DepthTestingMode DepthTesting = DepthTestingMode::On;
	BlendingMode Blending = BlendingMode::Off;
	CullingMode Culling = CullingMode::Off;
};

enum class DrawPrimitive { Point, Line, Triangle, Cubemap };
enum class DrawPartition { Single, Instanced, MultiDraw };

struct DrawCall {
	DrawCommand* Command;

	u32 IndexOffset = 0;
	u32 IndexCount = 0;
	u32 VertexOffset = 0;
	u32 VertexCount = 0;
	u32 InstanceOffset = 0;
	u32 InstanceCount = 0;

	DrawPrimitive Primitive = DrawPrimitive::Triangle;
	DrawPartition Partition = DrawPartition::Single;
};

class RendererAPI {
public:
	enum class Backend { OpenGL, Metal };

public:
	virtual DrawBufferID NewBuffer(const DrawBufferSpec&) = 0;
	virtual void SetBufferData(DrawBufferID, DrawBufferIndex, Buffer<void> data) = 0;

	virtual DrawPass* NewPass(DrawBufferID) = 0;
	virtual DrawCommand* NewCommand(DrawPass*) = 0;
	virtual DrawCall* NewCall(DrawCommand*) = 0;

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
#pragma once

#include <VolcaniCore/Core/Defines.h>

#include "Framebuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "StorageBuffer.h"
#include "UniformBuffer.h"

namespace VolcanicEngine::Graphics {

enum DrawBufferIndex : u8 {
	E_Vertex, E_Index, E_Instance
};

struct DrawBufferSpec {
	u32 IndexCount = 0;
	bool DynamicIndices = true;
	u32 VertexCount = 0;
	bool DynamicVertices = true;
	u32 InstanceCount = 0;
	bool DynamicInstances = true;
	BufferLayout VertexLayout = { };
	BufferLayout InstanceLayout = { };
};

class DrawBuffer : public Derivable<DrawBuffer> {
public:
	const DrawBufferSpec Spec;

public:
	DrawBuffer(const DrawBufferSpec& spec)
		: Spec(spec) { };
	virtual ~DrawBuffer() = default;

	virtual void Add(DrawBufferIndex, const void* data, u32 count) = 0;
	virtual void Clear() = 0;
	virtual u32 GetIndexCount() const = 0;
	virtual u32 GetVertexCount() const = 0;
	virtual u32 GetInstanceCount() const = 0;
};

struct DrawPass {
	DrawBuffer* Buffer = nullptr;
	Ref<Shader> Pipeline = nullptr;
	Ref<Framebuffer> Output = nullptr;
};

struct TextureSlot {
	Ref<Texture> Sampler = nullptr;
	u32 Binding = 0;
};

struct AttachmentSlot {
	Ref<Attachment> Sampler = nullptr;
	u32 Binding = 0;
};

struct UniformSlot {
	Ref<UniformBuffer> Buffer = nullptr;
	std::string Name = "";
	u32 Binding = 0;
};

struct StorageSlot {
	Ref<StorageBuffer> Buffer = nullptr;
	std::string Name = "";
	u32 Binding = 0;
};

struct DrawUniforms {
	Map<std::string, i32> IntUniforms;
	Map<std::string, f32> FloatUniforms;

	Map<std::string, Vec2> Vec2Uniforms;
	Map<std::string, Vec3> Vec3Uniforms;
	Map<std::string, Vec4> Vec4Uniforms;

	Map<std::string, Mat2> Mat2Uniforms;
	Map<std::string, Mat3> Mat3Uniforms;
	Map<std::string, Mat4> Mat4Uniforms;

	List<UniformSlot> UniformBuffers;
	List<StorageSlot> StorageBuffers;
	Map<std::string, TextureSlot> TextureUniforms;
	Map<std::string, AttachmentSlot> AttachmentUniforms;

	operator bool () const {
		return UniformBuffers || StorageBuffers
		|| IntUniforms.size() || FloatUniforms.size() || TextureUniforms.size()
		|| Vec2Uniforms.size() || Vec3Uniforms.size() || Vec4Uniforms.size()
		|| Mat2Uniforms.size() || Mat3Uniforms.size() || Mat4Uniforms.size();
	}

	DrawUniforms& Set(const std::string& name, i32 data) {
		IntUniforms[name] = data;
		return *this;
	}
	DrawUniforms& Set(const std::string& name, f32 data) {
		FloatUniforms[name] = data;
		return *this;
	}
	DrawUniforms& Set(const std::string& name, const Vec2& data) {
		Vec2Uniforms[name] = data;
		return *this;
	}
	DrawUniforms& Set(const std::string& name, const Vec3& data) {
		Vec3Uniforms[name] = data;
		return *this;
	}
	DrawUniforms& Set(const std::string& name, const Vec4& data) {
		Vec4Uniforms[name] = data;
		return *this;
	}
	DrawUniforms& Set(const std::string& name, const Mat2& data) {
		Mat2Uniforms[name] = data;
		return *this;
	}
	DrawUniforms& Set(const std::string& name, const Mat3& data) {
		Mat3Uniforms[name] = data;
		return *this;
	}
	DrawUniforms& Set(const std::string& name, const Mat4& data) {
		Mat4Uniforms[name] = data;
		return *this;
	}
	DrawUniforms& Set(const std::string& name, const TextureSlot& data) {
		TextureUniforms[name] = data;
		return *this;
	}
	DrawUniforms& Set(const std::string& name, const AttachmentSlot& data) {
		AttachmentUniforms[name] = data;
		return *this;
	}
	DrawUniforms& Set(const UniformSlot& data) {
		UniformBuffers.Add(data);
		return *this;
	}
	DrawUniforms& Set(const StorageSlot& data) {
		StorageBuffers.Add(data);
		return *this;
	}
};

enum class DepthTestingMode { On, Off };
enum class BlendingMode { Off, Greatest, Additive };
enum class CullingMode { Off, Front, Back };

struct DrawCall;

// A draw command is a series of draw calls
// with a shared set of uniforms, usually utilized for
// a specific piece of geometry
struct DrawCommand {
	DrawPass* Pass = nullptr;

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

enum class DrawPrimitive { Point, Line, Triangle, Cubemap };
enum class DrawPartition { Single, Instanced, MultiDraw };

struct DrawCall {
	DrawCommand* Command = nullptr;

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

public:
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
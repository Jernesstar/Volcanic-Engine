#pragma once

#include <VolcaniCore/Core/Defines.h>

#include "Platform/Shader.h"
#include "Platform/Texture.h"
#include "Platform/Framebuffer.h"
#include "Platform/RendererAPI.h"

using namespace VolcaniCore;
using namespace VolcanicEngine::Graphics;

namespace VolcanicEngine::Graphics {

template<typename TOut>
using ValueCallback = std::function<TOut(void)>;

template<typename TOut>
using CallbackMap = Map<std::string, ValueCallback<TOut>>;

class Uniforms {
public:
	CallbackMap<int32_t> IntCallbacks;
	CallbackMap<float> FloatCallbacks;
	CallbackMap<TextureSlot> TextureCallbacks;
	CallbackMap<UniformSlot> BufferCallbacks;

	CallbackMap<glm::vec2> Vec2Callbacks;
	CallbackMap<glm::vec3> Vec3Callbacks;
	CallbackMap<glm::vec4> Vec4Callbacks;

	CallbackMap<glm::mat2> Mat2Callbacks;
	CallbackMap<glm::mat3> Mat3Callbacks;
	CallbackMap<glm::mat4> Mat4Callbacks;
	CallbackMap<AttachmentSlot> AttachmentSlotCallbacks;

	template<class TPredicate>
	Uniforms& Set(const std::string& uniformName, TPredicate&& callback) {
		using TUniform = std::decay_t<decltype(callback())>;
		GetCallbacks<TUniform>()[uniformName] = callback;

		return *this;
	}

	Uniforms& Clear() {
		IntCallbacks.clear();
		FloatCallbacks.clear();
		TextureCallbacks.clear();
		Vec2Callbacks.clear();
		Vec3Callbacks.clear();
		Vec4Callbacks.clear();
		Mat2Callbacks.clear();
		Mat3Callbacks.clear();
		Mat4Callbacks.clear();
		AttachmentSlotCallbacks.clear();
		BufferCallbacks.clear();

		return *this;
	}

	template<typename TUniform>
	CallbackMap<TUniform>& GetCallbacks();

	operator bool () const {
		return IntCallbacks.size() || FloatCallbacks.size()
		|| TextureCallbacks.size() || BufferCallbacks.size()
		|| Vec2Callbacks.size() || Vec3Callbacks.size() || Vec4Callbacks.size()
		|| Mat2Callbacks.size() || Mat3Callbacks.size() || Mat4Callbacks.size()
		|| AttachmentSlotCallbacks.size();
	}
};

class RenderPass {
public:
	static Ref<RenderPass> Create(
		const std::string& name, Ref<Shader> shader,
		Ref<Framebuffer> output = nullptr)
	{
		return CreateRef<RenderPass>(name, shader, output);
	}

public:
	const std::string Name;

public:
	RenderPass(const std::string& name,
				Ref<Shader> pipeline, Ref<Framebuffer> output)
		: Name(name)
	{
		m_Pass = DrawPass{ nullptr, pipeline, output };
	}

	~RenderPass() = default;

	void SetOutput(Ref<Framebuffer> output) { m_Pass.Output = output; }
	void SetData(DrawBuffer* buffer) { m_Pass.Buffer = buffer; }

	DrawPass* Get() { return &m_Pass; }
	Ref<Shader> GetPipeline() const { return m_Pass.Pipeline; }
	Ref<Framebuffer> GetOutput() const { return m_Pass.Output; }

	void SetUniforms(DrawCommand* command);
	Uniforms& GetUniforms() { return m_Uniforms; }

private:
	DrawPass m_Pass;
	Uniforms m_Uniforms;
};

}
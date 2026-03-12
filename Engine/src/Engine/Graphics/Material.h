#pragma once

#include "Platform/RendererAPI.h"

namespace VolcanicEngine::Graphics {

struct Material {
	Ref<Shader> ShaderRef;

	Map<Str, i32> IntUniforms;
	Map<Str, f32> FloatUniforms;

	Map<Str, Vec2> Vec2Uniforms;
	Map<Str, Vec3> Vec3Uniforms;
	Map<Str, Vec4> Vec4Uniforms;

	Map<Str, Mat2> Mat2Uniforms;
	Map<Str, Mat3> Mat3Uniforms;
	Map<Str, Mat4> Mat4Uniforms;

	Map<Str, u64> UniformBuffers;
	Map<Str, u64> StorageBuffers;
	Map<Str, u64> TextureUniforms;
	Map<Str, u64> AttachmentUniforms;
	Map<Str, u64> CubemapUniforms;
};

}
#pragma once

#include <variant>

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

#include "Platform/RendererAPI.h"
#include "Platform/Shader.h"
#include "Platform/Texture.h"

#include <Engine/Asset/AssetRegistry.h>

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

using MatPropValue = std::variant<i32, f32, Vec2, Vec3, Vec4, Mat4, Ref<Texture>>;

struct MatProp {
	ShaderPropType Type;
	MatPropValue Value;
};

struct Material {
	Asset ShaderAsset; // NullAsset = default render pipeline
	Map<Str, MatProp> Props; // name -> value

	void SetInt(const Str& name, i32 v) {
		Props[name] = { ShaderPropType::Int, v };
	}
	void SetFloat(const Str& name, f32 v) {
		Props[name] = { ShaderPropType::Float, v };
	}
	void SetVec2(const Str& name, const Vec2& v) {
		Props[name] = { ShaderPropType::Vec2, v };
	}
	void SetVec3(const Str& name, const Vec3& v) {
		Props[name] = { ShaderPropType::Vec3, v };
	}
	void SetVec4(const Str& name, const Vec4& v) {
		Props[name] = { ShaderPropType::Vec4, v };
	}
	void SetMat4(const Str& name, const Mat4& v) {
		Props[name] = { ShaderPropType::Mat4, v };
	}
	void SetTex(const Str& name, Ref<Texture> v) {
		Props[name] = { ShaderPropType::Texture, v };
	}
};

struct MaterialInstance {
	Asset ParentAsset;
	Map<Str, MatProp> Overrides;
};

class MaterialBinder {
public:
	static void Bind(DrawCommand* cmd, const Material& mat,
					 const MaterialInstance* inst = nullptr)
	{
		u32 texSlot = 0;
		auto resolve = [&](const Str& name) -> const MatPropValue& {
			if(inst) {
				auto it = inst->Overrides.find(name);
				if(it != inst->Overrides.end()) return it->second.Value;
			}
			return mat.Props.at(name).Value;
		};

		for(auto& [name, prop] : mat.Props) {
			std::visit([&](auto&& v) {
				using T = std::decay_t<decltype(v)>;
				if constexpr(std::is_same_v<T, Ref<Texture>>)
					cmd->Uniforms.Set(name, TextureSlot{ v, texSlot++ });
				else
					cmd->Uniforms.Set(name, v);
			}, resolve(name));
		}
	}
};

}
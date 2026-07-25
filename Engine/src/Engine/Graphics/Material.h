#pragma once

#include <variant>

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/Log.h>

#include "Platform/RendererAPI.h"
#include "Platform/Shader.h"
#include "Platform/Texture.h"

#include <Engine/Asset/AssetRegistry.h>

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

// The native GBuffer shader (registered as EditorAssetManager default asset 100),
// i.e. the default deferred render pipeline. A Material whose ShaderAsset is null
// (NullAsset) binds THIS shader. The model-material importer and MaterialBinder
// both resolve the null-shader sentinel to this ID — keep them pointing at this
// one constant so the contract can never drift.
static constexpr u64 k_DefaultGBufferShaderID = 100;

// First texture unit assigned when the reflected ShaderLayout does not declare an
// explicit binding for a sampler (legacy / no-layout path).
static constexpr u32 k_FirstTextureSlot = 0;

// Note: `Asset` holds an unresolved texture reference (id + type). Material
// deserialization stores texture props as Asset; the binder resolves them to a
// Ref<Texture> at bind time via the AssetManager.
using MatPropValue =
	std::variant<i32, f32, Vec2, Vec3, Vec4, Mat4, Ref<Texture>, Asset>;

struct MatProp {
	ShaderPropType Type;
	MatPropValue Value;
};

struct Material {
	// NullAsset (a default-constructed Asset) is the sentinel for "default render
	// pipeline": it resolves to the native GBuffer shader k_DefaultGBufferShaderID
	// at bind time. Any non-null Asset names an explicit shader.
	Asset ShaderAsset;
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
	// `layout` is the reflected ShaderLayout of the shader this material binds to
	// (Material::ShaderAsset, or the default GBuffer shader when null — see
	// k_DefaultGBufferShaderID). The binder cannot reach the AssetManager without
	// a circular include, so the CALLER resolves the layout (handling the
	// null-shader case) and passes it in. When `layout` is null the binder falls
	// back to unvalidated writes with running texture slots, so callers that have
	// no layout yet keep their old behaviour.
	static void Bind(DrawCommand* cmd, const Material& mat,
					 const MaterialInstance* inst = nullptr,
					 const ShaderLayout* layout = nullptr)
	{
		u32 texFallback = k_FirstTextureSlot;

		// Instance override wins, else base prop. Returns nullptr rather than
		// throwing (the old Props.at threw on override-only names).
		auto resolve = [&](const Str& name) -> const MatProp* {
			if(inst) {
				auto it = inst->Overrides.find(name);
				if(it != inst->Overrides.end()) return &it->second;
			}
			auto it = mat.Props.find(name);
			return it != mat.Props.end() ? &it->second : nullptr;
		};

		// Reflected declaration for a prop name (uniform or sampler), or nullptr.
		auto findDecl = [&](const Str& name) -> const ShaderPropDeclaration* {
			if(!layout) return nullptr;
			for(auto& u : layout->Uniforms) if(u.Name == name) return &u;
			for(auto& s : layout->Samplers) if(s.Name == name) return &s;
			return nullptr;
		};

		// Sampler slot from the reflected binding (so base + override texture of
		// the same name land on the SAME unit), else a running fallback counter.
		auto samplerSlot = [&](const Str& name) -> u32 {
			if(layout)
				for(auto& s : layout->Samplers)
					if(s.Name == name) return s.Binding;
			return texFallback++;
		};

		auto bindOne = [&](const Str& name) {
			const MatProp* prop = resolve(name);
			if(!prop) return;

			// Bind-time reflection validation (only when a layout is available):
			// names the shader does not declare are skipped with a warning; type
			// mismatches warn. This replaces the old blind write of every prop.
			if(layout) {
				const ShaderPropDeclaration* decl = findDecl(name);
				if(!decl) {
					Log::Warning("Material bind: prop '{}' (shader {}) is not "
						"declared by the bound shader layout; skipping.",
						name, (u64)mat.ShaderAsset.ID);
					return;
				}
				// Texture props reflect as Texture while a sampler decl carries
				// ShaderPropType::Texture too, so this compares cleanly.
				if(decl->Type != prop->Type) {
					Log::Warning("Material bind: prop '{}' type {} != shader-"
						"declared type {} (shader {}); skipping.",
						name, (u32)prop->Type, (u32)decl->Type,
						(u64)mat.ShaderAsset.ID);
					return;
				}
			}

			std::visit([&](auto&& v) {
				using T = std::decay_t<decltype(v)>;
				if constexpr(std::is_same_v<T, Ref<Texture>>)
					cmd->Uniforms.Set(name, TextureSlot{ v, samplerSlot(name) });
				else if constexpr(std::is_same_v<T, Asset>)
					// Unresolved texture reference. The default deferred G-Buffer
					// path resolves these explicitly (see DefaultRenderPipeline);
					// the generic binder cannot reach the AssetManager without a
					// circular include, so it is skipped here.
					(void)v;
				else
					cmd->Uniforms.Set(name, v);
			}, prop->Value);
		};

		// Iterate the UNION of base props and instance overrides so override-only
		// names bind too. Base names that are overridden bind once (resolve picks
		// the override); override-only names are appended.
		for(auto& [name, prop] : mat.Props)
			bindOne(name);
		if(inst)
			for(auto& [name, prop] : inst->Overrides)
				if(!mat.Props.count(name))
					bindOne(name);
	}
};

}
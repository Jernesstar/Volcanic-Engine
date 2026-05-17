#pragma once

#include "AssetRegistry.h"

#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Utils/BytesReader.h>

#include <Engine/Graphics/Geometry.h>
#include <Engine/Graphics/Material.h>
#include <Engine/Graphics/Model.h>
#include <Engine/Graphics/Platform/Shader.h>
#include <Engine/Graphics/Platform/Texture.h>
#include <Engine/Graphics/Platform/Cubemap.h>
#include <Engine/Graphics/Platform/RendererAPI.h>
#include <Engine/Audio/Sound.h>
#include <Engine/Script/ScriptModule.h>
#include <Engine/Script/ScriptEngine.h>

using namespace VolcaniCore;
using namespace VolcanicEngine::Audio;
using namespace VolcanicEngine::Graphics;
using namespace VolcanicEngine::Script;

namespace VolcanicEngine {

class ByteCodeReader : public asIBinaryStream {
public:
	ByteCodeReader(BytesReader* reader) : m_Reader(reader) { }
	~ByteCodeReader() = default;

	int Read(void* data, u32 size) override {
		m_Reader->ReadData(data, (u64)size);
		return 0;
	}
	int Write(const void* data, u32 size) override { return 0; }

private:
	BytesReader* m_Reader = nullptr;
};

template<typename T>
static Ref<T> LoadFromBytes(Bytes&& bytes);

// ── Deserialization helpers ──────────────────────────────────────────────────

static MatPropValue ReadMatPropValue(BytesReader& reader, ShaderPropType type) {
	switch(type) {
	case ShaderPropType::Int:   { i32 v; reader.Read(v); return v; }
	case ShaderPropType::Float: { f32 v; reader.Read(v); return v; }
	case ShaderPropType::Vec2:  { Vec2 v; reader.ReadData(&v.x, sizeof(Vec2)); return v; }
	case ShaderPropType::Vec3:  { Vec3 v; reader.ReadData(&v.x, sizeof(Vec3)); return v; }
	case ShaderPropType::Vec4:  { Vec4 v; reader.ReadData(&v.x, sizeof(Vec4)); return v; }
	case ShaderPropType::Mat4:  { Mat4 v; reader.ReadData(glm::value_ptr(v), sizeof(Mat4)); return v; }
	case ShaderPropType::Texture: {
		u64 id; u8 t;
		reader.Read(id);
		reader.Read(t);
		return Asset{ id, (AssetType)t };
	}
	default: return i32(0);
	}
}

static void ReadModelNode(BytesReader& reader, ModelNode& node) {
	reader.Read(node.Name);
	reader.ReadData(&node.LocalTransform, sizeof(Vec3) * 3);

	u64 geoId; u8 geoType;
	reader.Read(geoId);
	reader.Read(geoType);
	node.GeometryAsset = { geoId, (AssetType)geoType };

	u32 matCount;
	reader.Read(matCount);
	node.MaterialAssets.Allocate(matCount);
	for(u32 i = 0; i < matCount; i++) {
		u64 matId; u8 matType;
		reader.Read(matId);
		reader.Read(matType);
		node.MaterialAssets.Add({ matId, (AssetType)matType });
	}

	u32 childCount;
	reader.Read(childCount);
	node.Children.Allocate(childCount);
	for(u32 i = 0; i < childCount; i++) {
		ModelNode& child = node.Children.Emplace();
		ReadModelNode(reader, child);
	}
}

// ── LoadFromBytes specializations ───────────────────────────────────────────

template<>
inline Ref<Geometry> LoadFromBytes<Geometry>(Bytes&& bytes) {
	BytesReader reader(std::move(bytes));
	auto geo = CreateRef<Geometry>(GeometryType::Model);

	u64 count;
	reader.Read(count);
	geo->Surfaces.Allocate(count);
	for(u64 i = 0; i < count; i++) {
		Buffer<Vertex> verts;
		Buffer<u32> indices;
		u32 slot;
		reader.Read(verts);
		reader.Read(indices);
		reader.Read(slot);
		geo->Surfaces.Emplace(std::move(verts), std::move(indices), slot);
	}
	return geo;
}

template<>
inline Ref<Texture> LoadFromBytes<Texture>(Bytes&& bytes) {
	BytesReader reader(std::move(bytes));
	u32 width, height, bpp;
	reader.Read(width);
	reader.Read(height);
	reader.Read(bpp);

	Buffer<u8> data;
	reader.Read(data);

	auto tex = RendererAPI::Get()->CreateTexture({ width, height });
	tex->SetData(data);
	return tex;
}

template<>
inline Ref<Cubemap> LoadFromBytes<Cubemap>(Bytes&& bytes) {
	// TODO: cubemap face loading
	return nullptr;
}

// ShaderLayout is serialized as: [uniform count] [decl*] [sampler count] [decl*]
// Each decl: [name] [type: u8] [binding: u32] [set: u32]
template<>
inline Ref<Shader> LoadFromBytes<Shader>(Bytes&& bytes) {
	BytesReader reader(std::move(bytes));

	u64 fileCount;
	reader.Read(fileCount);
	List<Graphics::ShaderFile> files;
	files.Allocate(fileCount);
	for(u32 i = 0; i < fileCount; i++) {
		u32 type;
		reader.Read(type);
		Buffer<u32> data;
		reader.Read(data);
		files.AddMove({ (ShaderFileType)type, std::move(data) });
	}

	ShaderLayout layout;

	u32 uniformCount;
	reader.Read(uniformCount);
	layout.Uniforms.Allocate(uniformCount);
	for(u32 i = 0; i < uniformCount; i++) {
		ShaderPropDeclaration decl;
		u8 type;
		reader.Read(decl.Name);
		reader.Read(type); decl.Type = (ShaderPropType)type;
		reader.Read(decl.Binding);
		reader.Read(decl.Set);
		layout.Uniforms.Add(decl);
	}

	u32 samplerCount;
	reader.Read(samplerCount);
	layout.Samplers.Allocate(samplerCount);
	for(u32 i = 0; i < samplerCount; i++) {
		ShaderPropDeclaration decl;
		u8 type;
		reader.Read(decl.Name);
		reader.Read(type); decl.Type = (ShaderPropType)type;
		reader.Read(decl.Binding);
		reader.Read(decl.Set);
		layout.Samplers.Add(decl);
	}

	auto shader = RendererAPI::Get()->CreateShader({});
	shader->SetShaderData(std::move(files), layout);
	return shader;
}

template<>
inline Ref<Sound> LoadFromBytes<Sound>(Bytes&& bytes) {
	BytesReader reader(std::move(bytes));
	Buffer<f32> data;
	reader.Read(data);

	auto sound = CreateRef<Sound>();
	sound->GetInternal().loadRawWave(data.Get(), data.GetCount(), 44100.0f, 1, true, false);
	return sound;
}

template<>
inline Ref<ScriptModule> LoadFromBytes<ScriptModule>(Bytes&& bytes) {
	BytesReader reader(std::move(bytes));
	Str name;
	reader.Read(name);

	auto* mod = ScriptEngine::Get()->GetModule(name.c_str(), asGM_ALWAYS_CREATE);
	ByteCodeReader stream(&reader);
	mod->LoadByteCode(&stream);
	return CreateRef<ScriptModule>(mod);
}

// Material format: [shader id: u64] [shader type: u8]
//                  [prop count: u32] ([name] [type: u8] [value])*
// Texture value:   [asset id: u64] [asset type: u8]
template<>
inline Ref<Material> LoadFromBytes<Material>(Bytes&& bytes) {
	BytesReader reader(std::move(bytes));
	auto mat = CreateRef<Material>();

	u64 shaderId; u8 shaderType;
	reader.Read(shaderId);
	reader.Read(shaderType);
	mat->ShaderAsset = { shaderId, (AssetType)shaderType };

	u32 propCount;
	reader.Read(propCount);
	for(u32 i = 0; i < propCount; i++) {
		Str name;
		u8 type;
		reader.Read(name);
		reader.Read(type);
		auto propType = (ShaderPropType)type;
		mat->Props[name] = { propType, ReadMatPropValue(reader, propType) };
	}
	return mat;
}

// Model format: recursive ModelNode tree
// Node: [name] [mat4 transform] [geo id: u64] [geo type: u8]
//       [mat count: u32] ([mat id: u64] [mat type: u8])*
//       [child count: u32] [child nodes]*
template<>
inline Ref<ModelAsset> LoadFromBytes<ModelAsset>(Bytes&& bytes) {
	BytesReader reader(std::move(bytes));
	auto model = CreateRef<ModelAsset>();
	ReadModelNode(reader, model->Root);
	return model;
}

// ── AssetManager base ────────────────────────────────────────────────────────

class AssetManager : public Derivable<AssetManager> {
public:
	static AssetManager* Get() { return s_Instance; }

public:
	AssetManager() { s_Instance = this; }

	virtual void Load(Asset asset) {
		if(!asset || IsLoaded(asset))
			return;

		m_LoadedAssets[asset.ID] = true;
		Bytes bytes = m_AssetRegistry->GetData(asset);

		if(asset.Type == AssetType::Geometry)
			m_GeometryAssets[asset.ID] = LoadFromBytes<Geometry>(std::move(bytes));
		else if(asset.Type == AssetType::Material)
			m_MaterialAssets[asset.ID] = LoadFromBytes<Material>(std::move(bytes));
		else if(asset.Type == AssetType::Model)
			m_ModelAssets[asset.ID] = LoadFromBytes<ModelAsset>(std::move(bytes));
		else if(asset.Type == AssetType::Texture)
			m_TextureAssets[asset.ID] = LoadFromBytes<Texture>(std::move(bytes));
		else if(asset.Type == AssetType::Cubemap)
			m_CubemapAssets[asset.ID] = LoadFromBytes<Cubemap>(std::move(bytes));
		else if(asset.Type == AssetType::Shader)
			m_ShaderAssets[asset.ID] = LoadFromBytes<Shader>(std::move(bytes));
		else if(asset.Type == AssetType::Audio)
			m_AudioAssets[asset.ID] = LoadFromBytes<Sound>(std::move(bytes));
		else if(asset.Type == AssetType::Script)
			m_ScriptAssets[asset.ID] = LoadFromBytes<ScriptModule>(std::move(bytes));
	}

	virtual void Unload(Asset asset) {
		if(!asset || !IsLoaded(asset))
			return;

		m_LoadedAssets.erase(asset.ID);

		if(asset.Type == AssetType::Geometry)        m_GeometryAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Texture)    m_TextureAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Cubemap)    m_CubemapAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Shader)     m_ShaderAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Audio)      m_AudioAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Script)     m_ScriptAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Material)   m_MaterialAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Model)      m_ModelAssets.erase(asset.ID);
	}

	template<typename T>
	Ref<T> Get(Asset asset) {
		if(!asset) return nullptr;
		Load(asset);

		if(asset.Type == AssetType::Geometry)
			return std::reinterpret_pointer_cast<T>(m_GeometryAssets[asset.ID]);
		else if(asset.Type == AssetType::Texture)
			return std::reinterpret_pointer_cast<T>(m_TextureAssets[asset.ID]);
		else if(asset.Type == AssetType::Cubemap)
			return std::reinterpret_pointer_cast<T>(m_CubemapAssets[asset.ID]);
		else if(asset.Type == AssetType::Shader)
			return std::reinterpret_pointer_cast<T>(m_ShaderAssets[asset.ID]);
		else if(asset.Type == AssetType::Audio)
			return std::reinterpret_pointer_cast<T>(m_AudioAssets[asset.ID]);
		else if(asset.Type == AssetType::Script)
			return std::reinterpret_pointer_cast<T>(m_ScriptAssets[asset.ID]);
		else if(asset.Type == AssetType::Material)
			return std::reinterpret_pointer_cast<T>(m_MaterialAssets[asset.ID]);
		else if(asset.Type == AssetType::Model)
			return std::reinterpret_pointer_cast<T>(m_ModelAssets[asset.ID]);
		return nullptr;
	}

	template<typename T>
	Ref<T> Get(const Str& name) {
		return Get<T>(m_AssetRegistry->FindAsset(name));
	}

	bool IsLoaded(Asset asset) const {
		return m_LoadedAssets.count(asset.ID);
	}

	bool IsNativeAsset(Asset asset) const {
		return (u64)asset.ID <= 200;
	}

	void ClearCache() {
		m_LoadedAssets.clear();
		m_GeometryAssets.clear();
		m_TextureAssets.clear();
		m_CubemapAssets.clear();
		m_ShaderAssets.clear();
		m_MaterialAssets.clear();
		m_ModelAssets.clear();
		m_AudioAssets.clear();
		m_ScriptAssets.clear();
	}

	Ref<AssetRegistry> GetRegistry() const { return m_AssetRegistry; }

protected:
	Ref<AssetRegistry> m_AssetRegistry;
	Map<UUID, bool>    m_LoadedAssets;

	Map<UUID, Ref<Geometry>>     m_GeometryAssets;
	Map<UUID, Ref<Texture>>      m_TextureAssets;
	Map<UUID, Ref<Cubemap>>      m_CubemapAssets;
	Map<UUID, Ref<Shader>>       m_ShaderAssets;
	Map<UUID, Ref<Material>>     m_MaterialAssets;
	Map<UUID, Ref<ModelAsset>>   m_ModelAssets;
	Map<UUID, Ref<Sound>>        m_AudioAssets;
	Map<UUID, Ref<ScriptModule>> m_ScriptAssets;

private:
	inline static AssetManager* s_Instance;
};

}
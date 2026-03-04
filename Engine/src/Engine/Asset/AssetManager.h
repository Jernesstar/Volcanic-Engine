#pragma once

#include "AssetRegistry.h"

#include <VolcaniCore/Core/Template.h>

#include <Engine/Graphics/Material.h>
#include <Engine/Audio/Sound.h>
#include <Engine/Script/ScriptModule.h>

#include <VolcaniCore/Utils/BinaryReader.h>

using namespace VolcaniCore;
using namespace VolcanicEngine::Audio;
using namespace VolcanicEngine::Graphics;
using namespace VolcanicEngine::Script;

namespace VolcanicEngine {

template<typename T>
static Ref<T> LoadFromBytes(Bytes&& bytes);

template<>
inline Ref<Mesh> LoadFromBytes<Mesh>(Bytes&& bytes) {
	return nullptr;
}
template<>
inline Ref<Texture> LoadFromBytes<Texture>(Bytes&& bytes) {
	return nullptr;
}
template<>
inline Ref<Cubemap> LoadFromBytes<Cubemap>(Bytes&& bytes) {
	return nullptr;
}
template<>
inline Ref<Shader> LoadFromBytes<Shader>(Bytes&& bytes) {
	return nullptr;
}
template<>
inline Ref<Sound> LoadFromBytes<Sound>(Bytes&& bytes) {
	return nullptr;
}
template<>
inline Ref<ScriptModule> LoadFromBytes<ScriptModule>(Bytes&& bytes) {
	return nullptr;
}

template<>
inline Ref<Material> LoadFromBytes<Material>(Bytes&& bytes) {
	return nullptr;
}

class AssetManager : public Derivable<AssetManager> {
public:
	static AssetManager* Get() { return s_Instance; }

public:
	AssetManager() { s_Instance = this; }

	void Load(Asset asset) {
		if(IsLoaded(asset))
			return;

		m_LoadedAssets[asset.ID] = true;

		Bytes bytes = m_AssetRegistry->GetData(asset);

		if(asset.Type == AssetType::Mesh)
			m_MeshAssets[asset.ID] = LoadFromBytes<Mesh>(std::move(bytes));
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
		else if(asset.Type == AssetType::Material)
			m_MaterialAssets[asset.ID] = LoadFromBytes<Material>(std::move(bytes));
	}

	void Unload(Asset asset) {
		if(!IsLoaded(asset))
			return;

		m_LoadedAssets.erase(asset.ID);

		if(asset.Type == AssetType::Mesh)
			m_MeshAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Texture)
			m_TextureAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Cubemap)
			m_CubemapAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Shader)
			m_ShaderAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Audio)
			m_AudioAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Script)
			m_ScriptAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Material)
			m_MaterialAssets.erase(asset.ID);
	}

	template<typename T>
	Ref<T> Get(Asset asset) {
		Load(asset);

		if(asset.Type == AssetType::Mesh)
			return std::reinterpret_pointer_cast<T>(m_MeshAssets[asset.ID]);
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

		return nullptr;
	}

	template<typename T>
	Ref<T> Get(const std::string& name) {
		Asset asset = m_AssetRegistry->FindAsset(name);
		return Get<T>(asset);
	}

	bool IsLoaded(Asset asset) const {
		return m_LoadedAssets.find(asset.ID) != m_LoadedAssets.end();
	}

	void ClearCache() {
		m_LoadedAssets.clear();

		m_MeshAssets.clear();
		m_TextureAssets.clear();
		m_CubemapAssets.clear();
		m_ShaderAssets.clear();
		m_MaterialAssets.clear();
		m_AudioAssets.clear();
		m_ScriptAssets.clear();
	}

	Ref<AssetRegistry> GetRegistry() const { return m_AssetRegistry; }

protected:
	Ref<AssetRegistry> m_AssetRegistry;
	Map<UUID, bool> m_LoadedAssets;

	Map<UUID, Ref<Mesh>> m_MeshAssets;
	Map<UUID, Ref<Texture>> m_TextureAssets;
	Map<UUID, Ref<Cubemap>> m_CubemapAssets;
	Map<UUID, Ref<Shader>> m_ShaderAssets;
	Map<UUID, Ref<Material>> m_MaterialAssets;
	Map<UUID, Ref<Sound>> m_AudioAssets;
	Map<UUID, Ref<ScriptModule>> m_ScriptAssets;

private:
	inline static AssetManager* s_Instance;
};

}
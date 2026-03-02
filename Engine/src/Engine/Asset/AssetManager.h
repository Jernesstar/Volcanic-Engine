#pragma once

#include "AssetRegistry.h"

#include <VolcaniCore/Core/Template.h>

#include <Engine/Graphics/Platform/RendererAPI.h>
#include <Engine/Audio/Sound.h>
#include <Engine/Script/ScriptModule.h>

using namespace VolcaniCore;
using namespace VolcanicEngine::Audio;
using namespace VolcanicEngine::Graphics;
using namespace VolcanicEngine::Script;

namespace VolcanicEngine {

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

		if(asset.Type == AssetType::Mesh) {
			// Ref<Mesh> asset = CreateRef<Mesh>(bytes);
			// m_MeshAssets[asset.ID] = asset;
		}
		else if(asset.Type == AssetType::Texture) {
			// Ref<Texture> asset = CreateRef<Texture>(bytes);
			// m_TextureAssets[asset.ID] = asset;
		}
		else if(asset.Type == AssetType::Cubemap) {
			// Ref<Cubemap> asset = CreateRef<Cubemap>(bytes);
			// m_CubemapAssets[asset.ID] = asset;
		}
		else if(asset.Type == AssetType::Shader) {
			// Ref<Shader> asset = CreateRef<Shader>(bytes);
			// m_ShaderAssets[asset.ID] = asset;
		}
		else if(asset.Type == AssetType::Audio) {
			// Ref<Audio> asset = CreateRef<Audio>(bytes);
			// m_AudioAssets[asset.ID] = asset;
		}
		else if(asset.Type == AssetType::Script) {
			// Ref<Script> asset = CreateRef<Script>(bytes);
			// m_ScriptAssets[asset.ID] = asset;
		}
		else if(asset.Type == AssetType::Material) {
			// Ref<Material> asset = CreateRef<Material>(bytes);
			// m_MaterialAssets[asset.ID] = asset;
		}
	}

	void Unload(Asset asset) {
		if(!IsLoaded(asset))
			return;

		m_LoadedAssets.erase(asset.ID);

		if(asset.Type == AssetType::Mesh)
			m_MeshAssets.erase(asset.ID);
		else if(asset.Type == AssetType::Texture)
			m_TextureAssets.erase(asset.ID);
		// else if(asset.Type == AssetType::Cubemap)
		// 	m_CubemapAssets.erase(asset.ID);
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
		// else if(asset.Type == AssetType::Cubemap)
		// 	return std::reinterpret_pointer_cast<T>(m_CubemapAssets[asset.ID]);
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
		// m_CubemapAssets.clear();
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
	// Map<UUID, Ref<Cubemap>> m_CubemapAssets;
	Map<UUID, Ref<Shader>> m_ShaderAssets;
	Map<UUID, Ref<DrawUniforms>> m_MaterialAssets;
	Map<UUID, Ref<Sound>> m_AudioAssets;
	Map<UUID, Ref<ScriptModule>> m_ScriptAssets;

private:
	inline static AssetManager* s_Instance;
};


}
#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/UUID.h>
#include <VolcaniCore/Core/List.h>

#include <Engine/Graphics/Mesh.h>
#include <Engine/Graphics/Platform/Shader.h>
#include <Engine/Graphics/Platform/Texture.h>
#include <Engine/Graphics/Platform/Cubemap.h>
#include <Engine/Graphics/Platform/RendererAPI.h>
#include <Engine/Audio/Sound.h>
#include <Engine/Script/ScriptModule.h>
#include <Engine/Script/ScriptClass.h>

#include "Database.h"

using namespace VolcaniCore;

namespace VolcanicEngine {

enum class AssetType : u8 {
	None,
	Mesh,
	Texture,
	Cubemap,
	Shader,
	Font,
	Audio,
	Script,
	Material,
	Custom
};

struct Asset {
	UUID ID = 0;
	AssetType Type = AssetType::None;
	bool Primary = true;
	operator uint64_t() const { return ID; }
};

}

namespace std {

template<>
struct hash<VolcanicEngine::Asset> {
	std::size_t operator()(const VolcanicEngine::Asset& asset) const {
		return (uint64_t)asset;
	}
};

}

namespace VolcanicEngine {

template<typename T>
struct AssetRef {
	T* Data;
	bool Loaded;
};

class AssetRegistry {
public:
	AssetRegistry();
	~AssetRegistry();

	void Add(Asset asset);
	void Remove(Asset asset);
	void Set(Asset asset, Bytes data);
	Bytes Get(Asset asset);

	bool IsValid(Asset asset) const;
	bool IsLoaded(Asset asset) const;
	
	bool HasRefs(Asset asset) const;
	void AddRef(Asset base, Asset ref);
	const VolcaniCore::List<Asset>& GetRefs(Asset asset) const;

	void NameAsset(Asset asset, const std::string& name);
	void RemoveName(Asset asset);

	void For(const Func<void, Asset>& cb);
	void Clear();

	std::string GetAssetName(Asset asset) const;
	Asset FindAsset(const std::string& lookup) const;

private:
	Registry* m_Registry;
	Database* m_AssetMetadata; // ID, Type, Ref, Name
	Database* m_AssetData; // ID, Data
};

}

#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/UUID.h>
#include <VolcaniCore/Core/List.h>

#include <Engine/Graphics/Geometry.h>
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
	Geometry,
	Texture,
	Cubemap,
	Shader,
	Material,
	Font,
	Audio,
	Script,
	Model,      // NEW: import hierarchy descriptor (not GPU data)
	Custom
};

struct Asset {
	UUID ID = 0;
	AssetType Type = AssetType::None;
	bool Primary = true;

	operator u64() const { return ID; }
	operator bool() const { return ID != 0 && Type != AssetType::None; }
};

}

namespace std {

template<>
struct hash<VolcanicEngine::Asset> {
	std::size_t operator()(const VolcanicEngine::Asset& asset) const {
		return (u64)asset;
	}
};

}

namespace VolcanicEngine {

class AssetRegistry {
public:
	AssetRegistry();
	~AssetRegistry();

	void Add(Asset asset);
	void Remove(Asset asset);
	void SetData(Asset asset, Bytes&& data);
	Bytes GetData(Asset asset);

	bool HasRefs(Asset asset) const;
	void AddRef(Asset base, Asset ref);
	VolcaniCore::List<Asset> GetRefs(Asset asset) const;

	void NameAsset(Asset asset, const std::string& name);
	void RemoveName(Asset asset);
	std::string GetAssetName(Asset asset) const;
	Asset FindAsset(const std::string& lookup) const;

	void For(const Func<void, Asset>& cb);
	void Clear();

private:
	// Registry* m_Registry;
	// Database* m_AssetMetadata;	// ID: Asset{ ID, Type, Primary }
	// Database* m_AssetRefs;		// ID: Refs
	// Database* m_AssetNames;		// ID: Name
	// Database* m_AssetNamesReverse; // Reverse name lookup
	Map<UUID, Asset> m_Registry;
	Map<UUID, List<Asset>> m_Refs;
	Map<std::string, Asset> m_Names;
};

}

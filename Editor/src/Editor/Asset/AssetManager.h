#pragma once

#include <Engine/Asset/AssetManager.h>

using namespace VolcaniCore;
using namespace VolcanicEngine;

namespace VolcanicEditor {

extern std::string AssetTypeToString(AssetType type);
extern AssetType AssetTypeFromString(const std::string& str);

class EditorAssetManager : public AssetManager {
public:
	EditorAssetManager();
	~EditorAssetManager();

	void Build(Asset asset);

	u32 AddReloadCallback(const Func<void, Asset, bool>& callback);
	void RemoveReloadCallback(u32 id);

	Asset Add(AssetType type, UUID id = 0, bool primary = true,
		const std::string& path = "");
	void Remove(Asset asset);

	std::string GetPath(VolcaniCore::UUID id) const;
	VolcaniCore::UUID GetFromPath(const std::string& path) const;

	void Clear();
	void LoadRegistry(const std::string& path);
	void Save();
	void RuntimeSave(const std::string& exportPath);

public:
	std::string m_Path;
	Map<UUID, std::string> m_Paths;
};

}
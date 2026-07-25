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

	void LoadRegistry(const std::string& projectRoot);
	void Export(const std::string& exportPath);
	void Clear();
	void Save();

	void Build(Asset asset);

	u32 AddReloadCallback(const Func<void, Asset, bool>& callback);
	void RemoveReloadCallback(u32 id);

	Asset Add(AssetType type, UUID id = 0, bool primary = true,
		const std::string& path = "");
	void Remove(Asset asset);

	std::string GetPath(VolcaniCore::UUID id) const;
	VolcaniCore::UUID GetFromPath(const std::string& path) const;

private:
	// Scans a Shader folder, groups stage files by double-stem into one primary
	// Shader asset per group (deterministic UUID from the group name), adds the
	// stage files as non-primary refs, and Builds each group.
	void RegisterShaderGroups(const std::filesystem::path& shaderFolder);

	// Registry name lookup that returns a null Asset instead of throwing when the
	// name is unknown (AssetRegistry::FindAsset uses map::at). Used by the .mat
	// builder to resolve `Shader: <name>` and `Ref: <name>` texture references.
	Asset FindAssetByName(const std::string& name) const;

public:
	std::string m_Path;
	Map<UUID, std::string> m_Paths;
};

}
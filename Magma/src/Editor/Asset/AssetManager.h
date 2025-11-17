#include "Asset.h"

namespace Magma {

class AssetRegistry : public Derivable<AssetRegistry> {
public:
	virtual void Load(const std::string& path) = 0;
	virtual Ref<Asset> GetAsset(AssetID id) = 0;
	virtual void LoadAsset(AssetID id) = 0;
	virtual void UnloadAsset(AssetID id) = 0;

protected:
	Map<AssetID, bool> s_AssetRegistry;
	Map<AssetID, Ref<Asset>> s_AssetCache;
};

class EditorAssetRegistry : public AssetRegistry {
public:
	EditorAssetRegistry() = default;
	~EditorAssetRegistry() = default;

	void Load(const std::string& path) override;
	Ref<Asset> GetAsset(AssetID id) override;
	void LoadAsset(AssetID id) override;
	void UnloadAsset(AssetID id) override;

private:
	Map<AssetID, std::string> m_AssetPaths;
};

class RuntimeAssetRegistry : public AssetRegistry {
public:
	RuntimeAssetRegistry() = default;
	~RuntimeAssetRegistry() = default;

	void Load(const std::string& path) override;
	Ref<Asset> GetAsset(AssetID id) override;
	void LoadAsset(AssetID id) override;
	void UnloadAsset(AssetID id) override;

private:
	Map<AssetID, u32> m_AssetOffsets;
};

class AssetManager {
public:
	static Ref<Asset> GetAsset(AssetID id);
	static void LoadAsset(AssetID id);
	static void UnloadAsset(AssetID id);

private:
	Ref<AssetRegistry> m_AssetRegistry;
};

}
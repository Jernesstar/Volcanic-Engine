#pragma once

#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

// #include <Lava/Script/ScriptModule.h>


using namespace VolcaniCore;

namespace Magma {

enum AssetType {
	Unknown,
	Image,
	GIF,
	Video,
	Mesh,
	Shader,
	Font,
	Audio,
	AudioStream,
	Script,
	Custom
};

typedef u32 AssetID;

class Asset : public Derivable<Asset> {
public:
	const AssetType Type;
	AssetID ID;

public:
	Asset(AssetType type)
		: Type(type) { }
	~Asset() = default;
};

class ImageAsset : public Asset {
public:
	u32 Width, Height;
	Buffer<u8> Data;

public:
	ImageAsset()
		: Asset(AssetType::Image) { }
};

class GIFAsset : public Asset {
public:
	u32 Width, Height;
	List<Buffer<u8>> Data;

public:
	GIFAsset()
		: Asset(AssetType::GIF) { }
};

class AudioAsset : public Asset {
public:
	Buffer<f32> Data;
	u32 Channels, SampleRate;

public:
	AudioAsset()
		: Asset(AssetType::Audio) { }
};

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

	AssetID GetAssetID(const std::string& path);

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
	static void Init();
	static void Close();
	static Ref<AssetRegistry> GetRegistry();

	static Ref<Asset> GetAsset(AssetID id);
	static void LoadAsset(AssetID id);
	static void UnloadAsset(AssetID id);

private:
	inline static Ref<AssetRegistry> s_AssetRegistry;
};

class AssetImporter {
public:
	static Ref<ImageAsset> LoadImage(const std::string& path, bool flip = false);
};

}
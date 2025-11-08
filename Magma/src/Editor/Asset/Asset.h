#pragma once

#include <soloud.h>
#include <soloud_wav.h>

#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

// #include <Lava/Script/ScriptModule.h>

using namespace VolcaniCore;

namespace Magma {

enum AssetType {
	IMAGE,
	GIF,
	VIDEO,
	MESH,
	SHADER,
	SOUND,
	CUSTOM
};

class Asset : public Derivable<Asset> {
public:
	const AssetType Type;

public:
	Asset(AssetType type)
		: Type(type) { }
	~Asset() = default;
};

class Image : public Asset {
public:
	uint32_t Width, Height;
	Buffer<uint8_t> Data;

public:
	Image()
		: Asset(AssetType::IMAGE) { }
};

class GIF : public Asset {
public:
	List<Image> Frames;

public:
	GIF()
		: Asset(AssetType::GIF) { }
};

class Sound : public Asset {
public:
	Sound()
		: Asset(AssetType::SOUND) { }

	void Play(float volume = -1.0f);
	SoLoud::Wav& GetInternal() { return m_Sound; }

private:
	SoLoud::Wav m_Sound;
};

class AssetManager {
public:
	static Ref<Image> LoadImage(const std::string& path, bool flip = false);

public:
	inline static Map<std::string, Ref<Asset>> s_Assets;
};

}
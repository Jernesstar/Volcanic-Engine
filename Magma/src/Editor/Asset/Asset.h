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
	Unknown,
	Image,
	GIF,
	Video,
	Mesh,
	Shader,
	Sound,
	Script,
	Custom
};

typedef u32 AssetID;

class Asset : public Derivable<Asset> {
public:
	const AssetType Type;

public:
	Asset(AssetType type)
		: Type(type) { }
	~Asset() = default;
};

class ImageAsset : public Asset {
public:
	uint32_t Width, Height;
	Buffer<uint8_t> Data;

public:
	ImageAsset()
		: Asset(AssetType::Image) { }
};

class GIFAsset : public Asset {
public:
	uint32_t Width, Height;
	List<Buffer<uint8_t>> Data;

public:
	GIFAsset()
		: Asset(AssetType::GIF) { }
};

class SoundAsset : public Asset {
public:
	SoundAsset()
		: Asset(AssetType::Sound) { }

	void Play(float volume = -1.0f);
	SoLoud::Wav& GetInternal() { return m_Sound; }

private:
	SoLoud::Wav m_Sound;
};

}
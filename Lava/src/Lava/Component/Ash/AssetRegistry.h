#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/UUID.h>

using namespace VolcaniCore;

namespace Lava {

enum class AssetType : uint8_t {
	None,
	Mesh, // .glTF, .obj, .fbx
	Texture, // .png, .jpg, .bmp
	Cubemap, // Texture folder, Texture
	Shader, // .glsl, .hlsl
	Font, // .ttf
	Audio, // .wav, .ogg, .mp3
	Script, // .as
	Custom // Script defined asset
};

struct Asset {
	UUID ID = 0;
	// AssetType Type = AssetType::None;
};

}


namespace std {

template<>
struct hash<Ash::Asset> {
	std::size_t operator()(const Ash::Asset& asset) const {
		return (uint64_t)asset.ID;
	}
};

}

namespace Ash {

class AssetRegistry {
public:


private:
	OMap<Asset, bool> m_AssetRegistry;
};

}
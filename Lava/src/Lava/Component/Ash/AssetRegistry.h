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
struct hash<Lava::Asset> {
	std::size_t operator()(const Lava::Asset& asset) const {
		return (uint64_t)asset.ID;
	}
};

}

namespace Lava {

class AssetRegistry {
public:


private:
	
};

}
#include "Asset.h"

#include <stb_image/stb_image.h>

namespace Magma {

void EditorAssetRegistry::Load(const std::string& path) {

}

Ref<Asset> EditorAssetRegistry::GetAsset(AssetID id) {
	
}

void EditorAssetRegistry::LoadAsset(AssetID id) {
	
}

void EditorAssetRegistry::UnloadAsset(AssetID id) {
	
}

AssetID EditorAssetRegistry::GetAssetID(const std::string& path) {
	return 0;
}

void RuntimeAssetRegistry::Load(const std::string& path) {
	
}

Ref<Asset> RuntimeAssetRegistry::GetAsset(AssetID id) {
	
}

void RuntimeAssetRegistry::LoadAsset(AssetID id) {
	
}

void RuntimeAssetRegistry::UnloadAsset(AssetID id) {
	
}


void AssetManager::Init() {

}

void AssetManager::Close() {

}

Ref<AssetRegistry> AssetManager::GetRegistry() {
	return s_AssetRegistry;
}

// class AudioEngine {
// public:
// 	static void Init() {
// 		s_Engine = new SoLoud::Soloud;
// 		s_Engine->init();
// 	}

// 	static void Shutdown() {
// 		s_Engine->deinit();
// 		delete s_Engine;
// 	}

// 	static SoLoud::Soloud* Get() {
// 		return s_Engine;
// 	}

// private:
// 	inline static SoLoud::Soloud* s_Engine;
// };

Ref<ImageAsset> AssetImporter::LoadImage(const std::string& path, bool flip) {
	stbi_set_flip_vertically_on_load((int)flip);
	Ref<ImageAsset> image = CreateRef<ImageAsset>();
	int width, height, bpp;
	uint8_t* pixels = stbi_load(path.c_str(), &width, &height, &bpp, 4);
	if(!pixels) {
		VOLCANICORE_LOG_WARNING("Could not load image '%s'", path.c_str());
		return { };
	}

	image->Width = (uint32_t)width;
	image->Height = (uint32_t)height;
	image->Data = Buffer(pixels, image->Width * image->Height * bpp);
	return image;
}

// Buffer<float> AssetImporter::GetData(const std::string& path) {
// 	SoLoud::Wav sound;
// 	VOLCANICORE_ASSERT(sound.load(path.c_str()) == 0);
// 	Buffer<float> data(sound.mSampleCount);
// 	data.Set(sound.mData, sound.mSampleCount);
// 	return data;
// }

// Ref<Sound> AssetImporter::GetAudio(const std::string& path) {
// 	auto output = CreateRef<Sound>();
// 	VOLCANICORE_ASSERT(output->GetInternal().load(path.c_str()) == 0);
// 	return output;
// }

}

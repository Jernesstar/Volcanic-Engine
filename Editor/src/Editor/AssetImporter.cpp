#include "AssetImporter.h"

#include <glad/glad.h>

#include <stb_image/stb_image.h>

#include <soloud.h>
#include <soloud_wav.h>

#include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Core/Log.h>
#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/FileUtils.h>

#include "ScriptManager.h"

using namespace VolcaniCore;
using namespace Magma::Script;

namespace fs = std::filesystem;

namespace Magma {

ImageData AssetImporter::GetImageData(const std::string& path, bool flip) {
	stbi_set_flip_vertically_on_load((int)flip);
	ImageData image;
	int width, height, bpp;
	uint8_t* pixels = stbi_load(path.c_str(), &width, &height, &bpp, 4);
	if(!pixels) {
		VOLCANICORE_LOG_WARNING("Could not load image '%s'", path.c_str());
		return { };
	}

	image.Width = (uint32_t)width;
	image.Height = (uint32_t)height;
	image.Data = Buffer(pixels, image.Width * image.Height * bpp);
	return image;
}

Ref<Texture> AssetImporter::GetTexture(const std::string& path, bool flip) {
	return CreateRef<Texture>(GetImageData(path, flip));
}

Buffer<float> AssetImporter::GetAudioData(const std::string& path) {
	SoLoud::Wav sound;
	VOLCANICORE_ASSERT(sound.load(path.c_str()) == 0);
	Buffer<float> data(sound.mSampleCount);
	data.Set(sound.mData, sound.mSampleCount);
	return data;
}

Ref<Sound> AssetImporter::GetAudio(const std::string& path) {
	auto output = CreateRef<Sound>();
	VOLCANICORE_ASSERT(output->GetInternal().load(path.c_str()) == 0);
	return output;
}

asIScriptModule* AssetImporter::GetScriptData(const std::string& path,
	bool* error, std::string name)
{
	return ScriptManager::LoadScript(path, false, error, name);
}

Ref<ScriptModule> AssetImporter::GetScript(const std::string& path) {
	bool error;
	asIScriptModule* handle = ScriptManager::LoadScript(path, true, &error);
	if(error)
		return nullptr;

	return CreateRef<ScriptModule>(handle);
}

}
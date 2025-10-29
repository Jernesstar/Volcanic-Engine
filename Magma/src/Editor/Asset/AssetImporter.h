#pragma once

#include <glad/glad.h>

#include <soloud.h>
#include <soloud_wav.h>

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Buffer.h>

#include <Magma/Script/ScriptModule.h>

using namespace VolcaniCore;

namespace Magma {

class AudioEngine {
public:
	static void Init() {
		s_Engine = new SoLoud::Soloud;
		s_Engine->init();
	}

	static void Shutdown() {
		s_Engine->deinit();
		delete s_Engine;
	}

	static SoLoud::Soloud* Get() {
		return s_Engine;
	}

private:
	static SoLoud::Soloud* s_Engine;
};

class Sound {
public:
	Sound() = default;
	~Sound() = default;

	void Play(float volume = -1.0f) {
		AudioEngine::Get()->play(m_Sound, volume);
	}

	SoLoud::Wav& GetInternal() { return m_Sound; }

private:
	SoLoud::Wav m_Sound;
};

struct ImageData {
	uint32_t Width, Height;
	Buffer<uint8_t> Data;
};

struct Texture {
	uint32_t ID;

	Texture(ImageData pixels) {
		auto format = GL_RGBA8;
		auto filter = GL_NEAREST;

		glCreateTextures(GL_TEXTURE_2D, 1, &ID);
		glTextureStorage2D(ID, 1, format, pixels.Width, pixels.Height);
		glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, filter);
		glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, filter);
		glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureSubImage2D(ID, 0, 0, 0, pixels.Width, pixels.Height, GL_RGBA,
							GL_UNSIGNED_BYTE, pixels.Data.Get());
	}
};

class AssetImporter {
public:
	static ImageData GetImageData(const std::string& path,
		bool flip = true);
	static Ref<Texture> GetTexture(const std::string& path,
		bool flip = true);

	static VolcaniCore::Buffer<float> GetAudioData(const std::string& path);
	static Ref<Sound> GetAudio(const std::string& path);

	static asIScriptModule* GetScriptData(const std::string& path,
		bool* error = nullptr, std::string name = "");
	static Ref<Script::ScriptModule> GetScript(const std::string& path);
};

}
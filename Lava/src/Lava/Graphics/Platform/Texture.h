#pragma once

#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Buffer.h>

using namespace VolcaniCore;

namespace Lava::Graphics {

enum class TextureType { RGBA, Depth, Stencil };
enum class TextureFormat { Normal, Float, Depth };
enum class TextureSampling { Nearest, Linear };

struct ImageData {
	u32 Width, Height, Channels, BPP;
	Buffer<u8> Data;
};

struct TextureSpec {
	u32 Width = 0;
	u32 Height = 0;

	TextureType Type = TextureType::RGBA;
	TextureFormat Format = TextureFormat::Normal;
	TextureSampling Sampling = TextureSampling::Nearest;
};

class Texture : public Derivable<Texture> {
public:
	const TextureSpec Spec;

public:
	Texture(const TextureSpec& spec)
		: Spec(spec) { }

	virtual void SetData(const void* data) = 0;

	template<typename T>
	void SetData(const Buffer<T>& buffer) {
		SetData(buffer.Get());
	}
};

}
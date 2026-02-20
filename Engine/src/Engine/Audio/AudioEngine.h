#pragma once

#include <soloud.h>

namespace VolcanicEngine::Audio {

class AudioEngine {
public:
	static void Init();
	static void Shutdown();
	static SoLoud::Soloud* Get();
};

}
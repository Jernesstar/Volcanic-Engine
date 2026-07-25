#include "AudioEngine.h"

namespace VolcanicEngine::Audio {

static SoLoud::Soloud* s_Engine;

void AudioEngine::Init() {
	s_Engine = new SoLoud::Soloud;
	// Explicit 2048-sample buffer (~46ms @ 44.1kHz): backend-default buffers
	// underrun (audible stutter) when the render thread saturates the CPU.
	s_Engine->init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::AUTO,
		SoLoud::Soloud::AUTO, 2048);
}

void AudioEngine::Shutdown() {
	s_Engine->deinit();
	delete s_Engine;
}

SoLoud::Soloud* AudioEngine::Get() {
	return s_Engine;
}

}
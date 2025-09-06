#include "Lava.h"

#include <Magma/Audio/AudioEngine.h>
#include <Magma/Script/ScriptEngine.h>

#include "Component/Ash/Ash.h"
#include "Component/Igneous/Igneous.h"
#include "Component/Silica/Silica.h"
#include "Component/Cinder/Cinder.h"
#include "Component/Pyro/Pyro.h"

#include "Physics/Physics.h"

#include "ScriptGlue.h"

using namespace Magma::Audio;
using namespace Magma::Script;

using namespace Lava::Physics;

namespace Lava {

void InitComponents() {
	Physics::Init();
	AudioEngine::Init();
	ScriptEngine::Init();

	ScriptGlue::RegisterInterface();

	Ash::Init();
	Ash::RegisterInterface();

	Igneous::Init();
	Igneous::RegisterInterface();

	Silica::Init();
	Silica::RegisterInterface();

	Cinder::Init();
	Cinder::RegisterInterface();

	Pyro::Init();
	Pyro::RegisterInterface();
}

void CloseComponents() {
	Ash::Close();
	Igneous::Close();
	Silica::Close();
	Cinder::Close();
	Pyro::Close();

	ScriptEngine::Shutdown();
	AudioEngine::Shutdown();
	// Physics::Close();
}

void BeginFrame() {
	Ash::BeginFrame();
	Igneous::BeginFrame();
	Silica::BeginFrame();
	Cinder::BeginFrame();
	Pyro::BeginFrame();
}

void Update(TimeStep ts) {
	Ash::Update(ts);
	Igneous::Update(ts);
	Silica::Update(ts);
	Cinder::Update(ts);
	Pyro::Update(ts);
}

void EndFrame() {
	Ash::EndFrame();
	Igneous::EndFrame();
	Silica::EndFrame();
	Cinder::EndFrame();
	Pyro::EndFrame();
}

}


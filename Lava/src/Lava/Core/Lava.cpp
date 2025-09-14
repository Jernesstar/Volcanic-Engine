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

static List<Component*> s_Components;

void InitComponents() {
	Physics::Init();
	AudioEngine::Init();
	ScriptEngine::Init();

	ScriptGlue::RegisterInterface();

	auto ash = new Ash();
	ash->Init();
	s_Components.Add(ash);

	auto igneous = new Igneous();
	igneous->Init();
	s_Components.Add(igneous);

	auto silica = new Silica();
	silica->Init();
	s_Components.Add(silica);
}

void CloseComponents() {
	for(auto component : s_Components) {
		component->Shutdown();
		delete component;
	}
	s_Components.Clear();

	ScriptEngine::Shutdown();
	AudioEngine::Shutdown();
	// Physics::Close();
}

void BeginFrame() {
	for(auto component : s_Components)
		component->BeginFrame();
}

void Update(TimeStep ts) {
	for(auto component : s_Components)
		component->OnUpdate(ts);
}

void EndFrame() {
	for(auto component : s_Components)
		component->EndFrame();
}

}


#include "Lava.h"

#include <Magma/Script/ScriptEngine.h>

#include "Component.h"
#include "ScriptGlue.h"

using namespace Magma::Script;

namespace Lava {

static List<Component*> s_Components;

void InitComponents() {
	ScriptEngine::Init();
	ScriptGlue::RegisterInterface();

	// auto ash = new Ash();
	// ash->Init();
	// s_Components.Add(ash);
}

void CloseComponents() {
	for(auto component : s_Components) {
		component->Shutdown();
		delete component;
	}
	s_Components.Clear();

	ScriptEngine::Shutdown();
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


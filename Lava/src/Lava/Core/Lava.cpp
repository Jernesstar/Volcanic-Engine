#include "Lava.h"

#include <Magma/Core/DLL.h>
#include <Magma/Script/ScriptEngine.h>

#include "Component.h"
#include "ScriptGlue.h"

using namespace Magma::Script;

namespace Lava {

static List<Ref<Magma::DLL>> s_DLLs;
static List<Component*> s_Components;

void InitComponents() {
	ScriptEngine::Init();
	ScriptGlue::RegisterInterface();
}

void LoadComponent(const std::string& path) {
	auto dll = CreateRef<Magma::DLL>(path);
	Component* component = dll->GetFunction<Component*>("CreateComponent")();
	s_DLLs.Add(dll);
	s_Components.Add(component);
}

void CloseComponents() {
	for(Component* component : s_Components) {
		component->Shutdown();
		delete component;
	}
	s_Components.Clear();

	ScriptEngine::Shutdown();
}

void BeginFrame() {
	for(Component* component : s_Components)
		component->BeginFrame();
}

void Update(TimeStep ts) {
	for(Component* component : s_Components)
		component->OnUpdate(ts);
}

void EndFrame() {
	for(Component* component : s_Components)
		component->EndFrame();
}

}


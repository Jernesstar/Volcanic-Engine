#include "Lava.h"

#include <Magma/Core/DLL.h>
#include <Magma/Script/ScriptEngine.h>

#include "Component.h"
#include "ScriptGlue.h"

using namespace Magma::Script;

namespace Lava {

struct ComponentTuple {
	Ref<Magma::DLL> DLL;
	Component* Comp;
};

static List<ComponentTuple> s_Components;

void InitComponents() {
	ScriptEngine::Init();
	ScriptGlue::RegisterInterface();
}

void LoadComponent(const std::string& path) {
	auto [found, i] =
		s_Components.Find([&](const auto& t) { return t.DLL->Path == path; });
	if(found) {
		auto tuple = s_Components.Pop(i);
		tuple.Comp->Shutdown();
		delete tuple.Comp;
		tuple.DLL.reset();
	}

	auto dll = CreateRef<Magma::DLL>(path);
	Component* component = dll->GetFunction<Component*>("CreateComponent")();
	component->Init();
	s_Components.Emplace(dll, component);
}

void CloseComponents() {
	for(auto& tuple : s_Components) {
		tuple.Comp->Shutdown();
		delete tuple.Comp;
		tuple.DLL.reset();
	}

	s_Components.Clear();

	ScriptEngine::Shutdown();
}

void BeginFrame() {
	for(auto& tuple : s_Components)
		tuple.Comp->BeginFrame();
}

void Update(TimeStep ts) {
	for(auto& tuple : s_Components)
		tuple.Comp->OnUpdate(ts);
}

void EndFrame() {
	for(auto& tuple : s_Components)
		tuple.Comp->EndFrame();
}

}


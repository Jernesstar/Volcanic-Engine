#include "Lava.h"

#include "Script/ScriptEngine.h"

#include "DLL.h"
#include "ScriptGlue.h"
#include "Component.h"

using namespace Lava::Script;

namespace Lava {

struct ComponentTuple {
	Ref<DLL> Lib;
	Component* Comp;
};

static List<ComponentTuple> s_Components;

void InitComponents() {
	ScriptEngine::Init();
	ScriptGlue::RegisterInterface();
}

void LoadComponent(const std::string& path) {
	auto [found, i] =
		s_Components.Find([&](const auto& t) { return t.Lib->Path == path; });
	if(found) {
		auto tuple = s_Components.Pop(i);
		tuple.Comp->Shutdown();
		delete tuple.Comp;
		tuple.Lib.reset();
	}

	auto dll = CreateRef<DLL>(path);
	Component* component = dll->GetFunction<Component*>("CreateComponent")();
	component->Init();
	s_Components.Emplace(dll, component);
}

void CloseComponents() {
	for(auto& tuple : s_Components) {
		tuple.Comp->Shutdown();
		delete tuple.Comp;
		tuple.Lib.reset();
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


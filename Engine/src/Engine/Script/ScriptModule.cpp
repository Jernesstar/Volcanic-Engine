#include "ScriptModule.h"
#include "ScriptEngine.h"

using namespace VolcaniCore;

namespace VolcanicEngine::Script {

ScriptModule::ScriptModule(asIScriptModule* mod)
	: m_Handle(mod)
{
	if(m_Handle)
		m_ModuleName = m_Handle->GetName();
	ReloadClasses();
}

ScriptModule::~ScriptModule() {
	if(!m_Handle)
		return;

	asIScriptEngine* engine = ScriptEngine::Get();
	if(!engine) {
		m_Handle = nullptr;
		return;
	}

	asIScriptModule* current =
		engine->GetModule(m_ModuleName.c_str(), asGM_ONLY_IF_EXISTS);
	if(current == m_Handle)
		current->Discard();
	m_Handle = nullptr;
}

void ScriptModule::ReloadClasses() {
	if(!m_Handle)
		return;

	for(uint32_t i = 0; i < m_Handle->GetObjectTypeCount(); i++) {
		asITypeInfo* type = m_Handle->GetObjectTypeByIndex(i);
		std::string name = type->GetName();
		m_Classes[name] = CreateRef<ScriptClass>(name, type);
	}
}

Ref<ScriptClass> ScriptModule::GetClass(const std::string& name) const {
	if(!m_Classes.count(name)) {
		Log::Warning("Could not find class '{0}'", name);
		return nullptr;
	}
	return m_Classes.at(name);
}

}
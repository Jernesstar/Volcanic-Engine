#include "ScriptModule.h"

using namespace VolcaniCore;

namespace VolcanicEngine::Script {

ScriptModule::ScriptModule(asIScriptModule* mod)
	: m_Handle(mod)
{
	ReloadClasses();
}

ScriptModule::~ScriptModule() {
	if(m_Handle)
		m_Handle->Discard();
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
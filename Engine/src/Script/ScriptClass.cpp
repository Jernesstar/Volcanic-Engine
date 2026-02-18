#include "ScriptClass.h"

#include "ScriptModule.h"

using namespace VolcaniCore;

namespace VolcanicEngine::Script {

ScriptClass::ScriptClass(const std::string& name, asITypeInfo* type)
	: Name(name)
{
	VOLCANICORE_ASSERT(type);
	m_Type = type;
	m_Factory = m_Type->GetFactoryByIndex(0);
	// VOLCANICORE_ASSERT(m_Factory);

	for(uint32_t i = 0; i < m_Type->GetMethodCount(); i++) {
		auto method = m_Type->GetMethodByIndex(i);
		m_Functions[method->GetName()] = method;
	}

	for(uint32_t i = 0; i < m_Type->GetPropertyCount(); i++) {
		const char* name;
		m_Type->GetProperty(i, &name, nullptr);
		m_FieldMap[name] = i;
	}
}

bool ScriptClass::Implements(const std::string& interfaceName) {
	asITypeInfo* interface =
		ScriptEngine::Get()->GetTypeInfoByName(interfaceName.c_str());
	return m_Type->Implements(interface);
}

bool ScriptClass::DerivesFrom(const std::string& className) {
	asITypeInfo* type =
		ScriptEngine::Get()->GetTypeInfoByName(className.c_str());
	return m_Type->DerivesFrom(type);
}

void ScriptClass::SetInstanceMethod(const List<std::string>& args) {
	std::string method = Name + " @" + Name + "(";
	for(auto& arg : args)
		method += arg;
	method += ")";

	m_Factory = m_Type->GetFactoryByDecl(method.c_str());
}

asIScriptFunction* ScriptClass::GetFunction(const std::string& name) const {
	if(!m_Functions.count(name)) {
		Log::Warning(
			"ScriptClass::GetFunction: \
			Could not find function '{0}' in class : {1}", name, Name);
		return nullptr;
	}
	return m_Functions.at(name);
}

ScriptFunc ScriptClass::GetFunc() const {
	return ScriptFunc{ m_Factory, ScriptEngine::GetContext() };
}

}
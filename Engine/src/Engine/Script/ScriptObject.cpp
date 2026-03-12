#include "ScriptObject.h"

#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/Math.h>

#include "ScriptClass.h"
#include "ScriptModule.h"

using namespace VolcaniCore;

namespace VolcanicEngine::Script {

bool ScriptField::Is(ScriptQualifier q) {
	switch(q) {
		case ScriptQualifier::AppObject:
			return TypeID & asTYPEID_APPOBJECT;
		case ScriptQualifier::ScriptObject:
			return TypeID & asTYPEID_SCRIPTOBJECT;
	}

	return false;
}

ScriptObject::ScriptObject() {
	m_RefCount = 1;
}

ScriptObject::ScriptObject(asIScriptObject* obj, ScriptClass* cl,
						   bool initialized)
{
	m_Handle = obj;
	m_Class = cl;
	m_Initialized = initialized;
	AddRef();
}

ScriptObject::~ScriptObject() {
	Release();
}

uint32_t ScriptObject::AddRef() {
	m_Handle->AddRef();
	return ++m_RefCount;
}

uint32_t ScriptObject::Release() {
	if(m_Handle) {
		m_Handle->Release();
		m_RefCount--;
	}

	return m_RefCount;
}

void ScriptObject::DestroyAndRelease() {
	while(m_Handle && --m_RefCount)
		m_Handle->Release();
	m_Handle = nullptr;
}

ScriptField ScriptObject::GetProperty(const std::string& name) {
	if(!m_Class->m_FieldMap.count(name))
		return { };

	return GetProperty(m_Class->m_FieldMap.at(name));
}

ScriptField ScriptObject::GetProperty(uint32_t idx) {
	VOLCANICORE_ASSERT(idx < m_Handle->GetPropertyCount());

	auto address = m_Handle->GetAddressOfProperty(idx);
	std::string name = m_Handle->GetPropertyName(idx);
	auto id = m_Handle->GetPropertyTypeId(idx);
	auto type = ScriptEngine::Get()->GetTypeInfoById(id);

	return { address, name, id, type };
}

ScriptFunc ScriptObject::GetFunc(const std::string& name) const {
	auto* function = m_Class->GetFunction(name);
	auto* context = ScriptEngine::GetContext();
	VOLCANICORE_ASSERT(function);
	VOLCANICORE_ASSERT(context);
	return { function, context, m_Handle };
}

}
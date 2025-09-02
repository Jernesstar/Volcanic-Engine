#include "Tab.h"

#include <Magma/Script/ScriptEngine.h>
#include <Magma/Script/ScriptClass.h>

using namespace Magma::Script;

namespace Magma {

void Tab::RegisterInterface() {
	auto* engine = ScriptEngine::Get();

	engine->RegisterObjectType("Tab_t", 0, asOBJ_REF | asOBJ_NOCOUNT);

	engine->RegisterObjectMethod("Tab_t", "Panel_t@ GetPanel(const string &in)",
		asMETHOD(Tab, GetPanel), asCALL_THISCALL);
	engine->RegisterObjectProperty("Tab_t", "string Name", asOFFSET(Tab, Name));
}

Tab::Tab() {

}

Tab::~Tab() {
	m_IsDead->Release();
}

void Tab::Init(ScriptObject obj) {
	m_IsDead = obj.GetHandle()->GetWeakRefFlag();
	m_IsDead->AddRef();
	m_ScriptObj = obj;
	Type = m_ScriptObj.GetClass()->Name;
}

void Tab::OnLoad(const std::string& path) {
	m_ScriptObj.Call("OnLoad", path);
}

void Tab::OnSelect() {
	m_ScriptObj.Call("OnSelect");
}

void Tab::OnDeselect() {
	m_ScriptObj.Call("OnDeselect");
}

void Tab::OnClose() {
	m_ScriptObj.Call("OnClose");
}

void Tab::OnUpdate(TimeStep ts) {
	if(!m_IsDead->Get())
		m_ScriptObj.Call("OnUpdate", (float)ts);
}

void Tab::OnRender() {
	m_ScriptObj.Call("OnRender");
}

Panel* Tab::GetPanel(const std::string& name) {
	auto [found, i] =
		Panels.Find([&](auto& panel) { return panel.Name == name; });
	if(found)
		return Panels.At(i);

	VOLCANICORE_LOG_WARNING("Could not find panel '%s'", name.c_str());
	return nullptr;
}

}
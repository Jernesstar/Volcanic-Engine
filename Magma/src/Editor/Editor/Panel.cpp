#include "Panel.h"

#include <Magma/Script/ScriptEngine.h>

#include "Tab.h"

using namespace Magma::Script;

namespace Magma {

void Panel::RegisterInterface() {
	auto* engine = ScriptEngine::Get();

	engine->RegisterObjectType("Panel_t", 0, asOBJ_REF | asOBJ_NOCOUNT);

	engine->RegisterObjectMethod("Panel_t", "Panel_t@ GetPanel(const string &in)",
		asMETHOD(Panel, GetPanel), asCALL_THISCALL);
}

Panel::Panel(const std::string& name, ScriptObject obj)
	: Name(name), m_ScriptObj(obj)
{
	m_IsDead = obj.GetHandle()->GetWeakRefFlag();
	m_IsDead->AddRef();
	m_ScriptObj = obj;
}

Panel::~Panel() {
	m_IsDead->Release();
}

void Panel::OnOpen() {
	if(!m_IsDead->Get())
		m_ScriptObj.Call("OnOpen");
}

void Panel::OnClose() {
	if(!m_IsDead->Get())
		m_ScriptObj.Call("OnClose");
}

void Panel::OnUpdate(TimeStep ts) {
	if(!m_IsDead->Get())
		m_ScriptObj.Call("OnUpdate", (float)ts);
}

void Panel::OnRender() {
	if(!m_IsDead->Get())
		m_ScriptObj.Call("OnRender");
}

Panel* Panel::GetPanel(const std::string& name) {
	return m_Tab->GetPanel(name);
}

}
#include "Panel.h"

#include <Magma/Script/ScriptEngine.h>

#include "Editor.h"
#include "Tab.h"

using namespace Magma::Script;

namespace Magma::UI {

void Panel::RegisterInterface() {
	auto* engine = ScriptEngine::Get();

	engine->RegisterObjectType("Panel_t", 0, asOBJ_REF | asOBJ_NOCOUNT);

	engine->RegisterObjectMethod("Panel_t", "Panel_t@ GetPanel(const string &in)",
		asMETHOD(Panel, GetPanel), asCALL_THISCALL);
}

Panel::Panel(const std::string& tab, const std::string& name)
	: Name(name)
{
	VOLCANICORE_LOG_INFO("%s", name.c_str());
	m_ScriptObj = Editor::GetPanelClass(tab, name)->Instantiate();
	m_IsDead = m_ScriptObj.GetHandle()->GetWeakRefFlag();
	m_IsDead->AddRef();
}

Panel::~Panel() {
	m_ScriptObj.DestroyAndRelease();
	if(m_IsDead)
		m_IsDead->Release();
}

void Panel::OnOpen() {
	if(m_IsDead && !m_IsDead->Get())
		m_ScriptObj.Call("OnOpen");
}

void Panel::OnClose() {
	if(m_IsDead && !m_IsDead->Get())
		m_ScriptObj.Call("OnClose");
}

void Panel::OnUpdate(TimeStep ts) {
	if(m_IsDead && !m_IsDead->Get())
		m_ScriptObj.Call("OnUpdate", (float)ts);
}

void Panel::OnRender() {
	if(m_IsDead && !m_IsDead->Get())
		m_ScriptObj.Call("OnRender");
}

Panel* Panel::GetPanel(const std::string& name) {
	return m_Tab->GetPanel(name);
}

}
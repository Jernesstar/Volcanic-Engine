#include "Tab.h"

#include <VolcaniCore/Core/FileUtils.h>

#include <Lava/Script/ScriptEngine.h>
#include <Lava/Script/ScriptClass.h>

#include "Editor.h"
#include "Asset.h"
#include "ScriptManager.h"

namespace fs = std::filesystem;

using namespace Lava::Script;

namespace Magma::UI {

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
	m_ScriptObj.DestroyAndRelease();
	if(m_IsDead)
		m_IsDead->Release();
}

void Tab::Init(const std::string& type) {
	Type = type;
	m_ScriptObj = Editor::GetTabClass(type)->Instantiate();
	m_IsDead = m_ScriptObj.GetHandle()->GetWeakRefFlag();
	m_IsDead->AddRef();

	auto mod = Editor::GetModule(type);
	for(auto& [name, cls] : mod->GetClasses())
		if(cls->DerivesFrom("Panel"))
			Panels.Emplace(type, name);

	ScriptField field = m_ScriptObj.GetProperty("TabHandle");
	*field.As<Tab*>() = this;
	OnOpen();
}

void Tab::OnOpen() {
	if(m_IsDead && !m_IsDead->Get())
		m_ScriptObj.Call("OnOpen");
}

void Tab::OnClose() {
	if(m_IsDead && !m_IsDead->Get())
		m_ScriptObj.Call("OnClose");
}

void Tab::OnLoad(const std::string& path) {
	if(m_IsDead && !m_IsDead->Get())
		m_ScriptObj.Call("OnLoad", path);
}

void Tab::OnSelect() {
	if(m_IsDead && !m_IsDead->Get())
		m_ScriptObj.Call("OnSelect");
}

void Tab::OnDeselect() {
	if(m_IsDead && !m_IsDead->Get())
		m_ScriptObj.Call("OnDeselect");
}

void Tab::OnUpdate(TimeStep ts) {
	if(m_IsDead && !m_IsDead->Get())
		m_ScriptObj.Call("OnUpdate", (float)ts);

	for(auto& panel : Panels)
		panel.OnUpdate(ts);
}

void Tab::OnRender() {
	if(m_IsDead && !m_IsDead->Get())
		m_ScriptObj.Call("OnRender");

	for(auto& panel : Panels)
		panel.OnRender();
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
#include "Tab.h"

#include <VolcaniCore/Core/FileUtils.h>

#include <Magma/Script/ScriptEngine.h>
#include <Magma/Script/ScriptClass.h>

#include "Editor.h"
#include "AssetImporter.h"
#include "ScriptManager.h"

namespace fs = std::filesystem;

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

void Tab::Init(const std::string& type) {
	const auto& lavaFlow = Editor::GetLavaFlow();
	auto uiPath = fs::path(lavaFlow.Path) / lavaFlow.Name / "Object" / type / "UI";
	auto modulePath = (uiPath / type).string() + "Tab.as";
	auto moduleData =
		ScriptManager::LoadScript(modulePath, false, nullptr, "",
			{ "Magma/scripts", "Lava/scripts" });
	Ref<ScriptModule> mod = CreateRef<ScriptModule>(moduleData);
	Ref<ScriptClass> cls = mod->GetClass(type);
	m_ScriptObj = cls->Instantiate();

	auto field = m_ScriptObj.GetProperty("TabHandle");
	// *field.As<Tab>() = &newTab;

	for(auto path : FileUtils::GetFiles(uiPath.string(), { ".as" })) {
		auto name = fs::path(path).filename().string();
		Ref<ScriptModule> panelMod = AssetImporter::GetScript(path);
		Ref<ScriptClass> panelClass = panelMod->GetClass(type);
		ScriptObject panelScriptObj = panelClass->Instantiate();
		Panels.Emplace(name, panelScriptObj);
	}

	m_IsDead = m_ScriptObj.GetHandle()->GetWeakRefFlag();
	m_IsDead->AddRef();
	Type = type;
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
#include "Editor.h"

#include <fstream>
#include <iostream>
#include <regex>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Algo.h>
#include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Core/Log.h>

#include <VolcanicWindow/Application.h>
#include <VolcanicWindow/Events.h>
#include <VolcanicWindow/Input.h>

#include <Lava/Core/Lava.h>

#include "Integration/AI/AI.h"
#include "Integration/VersionControl/VersionControl.h"
#include "Integration/Lang/ScriptManager.h"

#include "Utils/YAMLSerializer.h"
#include "Widget/Widget.h"
#include "Graphics/Renderer.h"
#include "Graphics/Platform/RendererAPI.h"
#include "Networking/Networking.h"
#include "Asset/Asset.h"

#undef LoadImage
#undef LoadImageW
#undef LoadImageA

using namespace VolcaniCore;
using namespace VolcanicWindow;

// using namespace Magma::Asset;
using namespace Magma::UI;
using namespace Magma::Graphics;
using namespace Magma::Networking;

namespace fs = std::filesystem;

namespace Magma {

static void ProjectLoad(const std::string& path, Project& project);
static void ProjectSave(const Project& project);
static void ProjectSaveRuntime(const Project& project);

static Map<std::string, Ref<ScriptModule>> s_TabModules;

static VC::VCManager* s_VCManager = nullptr;
// static AI::AIManager* s_AIManager = nullptr;
// static Lang::LangManager* s_LangManager = nullptr;
// static Collab::CollabManager* s_CollabManager = nullptr;

void Editor::Open() {
	// Editor::RegisterInterface();

	Application::PushDir();
	auto logo =
		AssetImporter::LoadImage(
			"Magma/assets/images/VolcanicDisplay.png", false);

	// auto window = Application::As<WindowApplication>()->GetWindow();
	// window->SetIcon({ logo->Width, logo->Height, logo->Data.Copy() });

	AssetManager::Init();
	Renderer::Init();
	WidgetManager::Init();

	// WidgetManager::Load("Magma/assets/UI/component.asx");
	// WidgetManager::Load("Magma/assets/UI/lavaflow.asx");

	Application::PopDir();

	Events::RegisterListener<KeyPressedEvent>(
		[](const KeyPressedEvent& event)
		{
			if(event.Key == Key::R)
				WidgetManager::Reload();
		});

	AI::AIManager::Init();
}

void Editor::Close() {
	CloseProject();
	CloseLavaFlow();
	CloseComponent();

	WidgetManager::Close();
	Renderer::Close();
	AssetManager::Close();
}

Ref<ScriptModule> Editor::GetModule(const std::string& name) {
	if(!s_TabModules.count(name)) {
		VOLCANICORE_LOG_INFO("Could not find module '%s'", name.c_str());
		return nullptr;
	}
	return s_TabModules[name];
}

Ref<ScriptClass> Editor::GetTabClass(const std::string& name) {
	if(!s_TabModules.count(name)) {
		VOLCANICORE_LOG_INFO("Could not find tab class '%s'", name.c_str());
		return nullptr;
	}
	return s_TabModules[name]->GetClass(name + "Tab");
}

Ref<ScriptClass> Editor::GetPanelClass(const std::string& tab,
									   const std::string& name)
{
	if(!s_TabModules.count(tab)) {
		VOLCANICORE_LOG_INFO("Could not find panel class '%s'", name.c_str());
		return nullptr;
	}
	return s_TabModules[tab]->GetClass(name);
}

void Editor::Load(const CommandLineArgs& args) {
	if(args["--project"])
		OpenProject(args["--project"]);
	else if(args["--lavaflow"])
		OpenLavaFlow(args["--lavaflow"]);
	else if(args["--component"])
		OpenComponent(args["--component"]);
}

void Editor::Update(TimeStep ts) {
	if(Mode == EditorMode::Project)
		for(auto& tab : m_Tabs)
			tab.OnUpdate(ts);

	WidgetManager::Update(ts);
}

void Editor::Render() {
	Renderer::BeginFrame();

	if(Mode == EditorMode::None)
		RenderStartScreen();
	else if(Mode == EditorMode::Component)
		RenderComponentEditor();
	else if(Mode == EditorMode::Flow)
		RenderFlowEditor();
	else if(Mode == EditorMode::Project)
		RenderProjectEditor();

	WidgetManager::Render();

	Renderer::EndFrame();
}

void Editor::RenderStartScreen() {
	static uint32_t mode = 0; // 1 = Project, 2 = Flow, 3 = Component

	auto root = UI::WidgetManager::GetRoot();
	bool click = false;

	// if(root->Find("CloseButton")->State.Clicked)
	// 	Application::Close();
	// if(root->Find("MinimizeButton")->State.Clicked)
	// 	Application::Minimize();

	// if(root->Find("SignInButton")->State.Clicked)
	// 	NetworkingManager::GitHubOAuth();

	// if(root->Find("ProjectButton")->State.Clicked) {
	// 	click = true;
	// 	mode = 1;
	// }
	// else if(root->Find("LavaFlowButton")->State.Clicked) {
	// 	click = true;
	// 	mode = 2;
	// }
	// else if(root->Find("ComponentButton")->State.Clicked) {
	// 	click = true;
	// 	mode = 3;
	// }

	// if(click) {
	// 	Ref<UI::Widget> w;
	// 	w = root->Find("ProjectWindow");
	// 	w->Visible = mode == 1;
	// 	w = root->Find("LavaFlowWindow");
	// 	w->Visible = mode == 2;
	// 	w = root->Find("ComponentWindow");
	// 	w->Visible = mode == 3;
	// }
}

void Editor::RenderComponentEditor() {

}

void Editor::RenderFlowEditor() {

}

void Editor::RenderProjectEditor() {

}

static bool s_NewTab = false;

void Editor::NewTab() {

}

void Editor::OpenTab(const std::string& type) {
	UI::Tab& newTab = m_Tabs.Emplace();
	newTab.Init(type);
	SetTab(m_Tabs.Count());
}

void Editor::LoadTab(const std::string& type) {
	// m_ScriptObj.Call("LoadTab", type);
}

void Editor::ReopenTab() {
	if(m_ClosedTabs) {
		m_Tabs.AddMove(m_ClosedTabs.Pop());
		SetTab(m_Tabs.Count());
	}
}

void Editor::CloseTab(uint32_t idx) {
	if(idx == 0)
		return;

	m_ClosedTabs.AddMove(m_Tabs.Pop(idx - 1));
	if(idx == m_CurrentTab)
		SetTab(idx - 1);
}

void Editor::SetTab(uint32_t idx) {
	auto title = "Magma Editor: " + m_Project.Name;

	if(GetCurrentTab())
		GetCurrentTab()->OnDeselect();

	if(idx > 0) {
		m_CurrentTab = idx;
		UI::Tab* tab = GetCurrentTab();
		tab->OnSelect();

		title += " - " + tab->Name;
	}

	// Application::GetWindow()->SetTitle(title);
}

void Editor::NewProject() {

}

void Editor::OpenProject() {
	// s_OpenProjectDialog->Render();
}

void Editor::OpenProject(const std::string& file) {
	ProjectLoad(file, m_Project);
	auto [found, i] =
		m_Cache.PastProjects.Find(file);
	if(!found)
		m_Cache.PastProjects.Add(file);

	// m_LavaFlow.Path = m_Project.LavaFlow;
	// LoadLavaFlow(m_LavaFlow.Path);
	// OpenTab("Project");
	Mode = EditorMode::Project;
}

void Editor::CloseProject() {
	m_Tabs.Clear();
	m_ClosedTabs.Clear();
	m_CurrentTab = 0;

	if(m_Project.Path != "")
		ProjectSave(m_Project);
	m_Project = { };
}

void Editor::NewLavaFlow() {
	
}

void Editor::OpenLavaFlow() {
	
}

void Editor::OpenLavaFlow(const std::string& file) {
	CloseLavaFlow();

	YAML::Node fileNode;
	try {
		fileNode = YAML::LoadFile(file);
	} catch (YAML::ParserException e) {
		VOLCANICORE_LOG_INFO("File '%s' is not well formatted", file.c_str());
		return;
	} catch (YAML::BadFile e) {
		VOLCANICORE_LOG_INFO("File '%s' is bad", file.c_str());
		return;
	}

	auto lavaFlowNode = fileNode["LavaFlow"];
	VOLCANICORE_ASSERT(lavaFlowNode);

	auto path = fs::path(file).parent_path();
	m_LavaFlow.Path = path.string();
	m_LavaFlow.Name = path.filename().string();
	m_LavaFlow.Components = lavaFlowNode["Components"].as<List<std::string>>();
	m_LavaFlow.ObjectList = lavaFlowNode["Objects"].as<List<std::string>>();

	List<std::string> includes =
		m_LavaFlow.Components.Apply<std::string>(
			[](const std::string& object) -> std::string
			{
				return object + "/Script";
			});
	for(auto type : m_LavaFlow.ObjectList) {
		auto uiPath = fs::path(m_LavaFlow.Path) / "Editor" / "UI" / type;
		auto moduleData =
			ScriptManager::LoadScript(
				FileUtils::GetFiles(uiPath.string(), { ".as" }),
				false, nullptr, type, includes);

		s_TabModules[type] = CreateRef<ScriptModule>(moduleData);
	}

	Mode = EditorMode::Flow;
}

void Editor::CloseLavaFlow() {
	s_TabModules.clear();
}

void Editor::NewComponent() {

}

void Editor::OpenComponent() {

}

void Editor::OpenComponent(const std::string& file) {
	YAML::Node fileNode;
	try {
		fileNode = YAML::LoadFile(file);
	} catch (YAML::ParserException e) {
		VOLCANICORE_LOG_INFO("File '%s' is not well formatted", file.c_str());
		return;
	} catch (YAML::BadFile e) {
		VOLCANICORE_LOG_INFO("File '%s' is bad", file.c_str());
		return;
	}

	auto componentNode = fileNode["Component"];
	VOLCANICORE_ASSERT(componentNode);

	auto path = fs::path(file).parent_path();
	m_Component.Path = fs::canonical(path).string();
	m_Component.Name = path.filename().string();
	m_Component.BuildAuto = componentNode["Build"]["Auto"].as<bool>();
	m_Component.CoreDeps =
		componentNode["Dependencies"]["Core"].as<List<std::string>>();
	m_Component.EditorDeps =
		componentNode["Dependencies"]["Editor"].as<List<std::string>>();

	Mode = EditorMode::Component;
}

void Editor::CloseComponent() {
	Mode = EditorMode::None;
}

void ProjectLoad(const std::string& path, Project& project) {
	YAML::Node file;
	try {
		file = YAML::LoadFile(path);
	}
	catch(YAML::ParserException e) {
		VOLCANICORE_LOG_INFO("File '%s' is not well formatted", path.c_str());
		return;
	}
	catch(YAML::BadFile e) {
		VOLCANICORE_LOG_INFO("File '%s' is bad", path.c_str());
		return;
	}

	auto projNode = file["Project"];
	VOLCANICORE_ASSERT(projNode);

	project.Path = fs::canonical(path).parent_path().string();
	project.Name = projNode["Name"].as<std::string>();
	project.LavaFlow = projNode["LavaFlow"].as<std::string>();
}

void ProjectSave(const Project& project) {
	YAMLSerializer serializer;
	serializer.BeginMapping(); // File

	serializer.WriteKey("Project")
	.BeginMapping()
		.WriteKey("Name").Write(project.Name)
		.WriteKey("LavaFlow").Write(project.LavaFlow)
		;

	serializer
	.EndMapping(); // Project

	serializer.EndMapping(); // File

	auto exportPath = (fs::path(project.Path) / ".magma.proj").string();
	serializer.Finalize(exportPath);
}

void ProjectSaveRuntime(const Project& project) {
	// auto exportPath = (fs::path(project.ExportPath) / ".volc.proj").string();
	// BinaryWriter writer(exportPath);
	// writer.Write(project.Name);
	// writer.Write(project.App);
}

void Editor::RegisterInterface() {

}

}
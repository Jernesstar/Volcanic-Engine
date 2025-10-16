#include "Editor.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <fstream>
#include <iostream>
#include <regex>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Algo.h>
#include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Core/Log.h>
#include <VolcaniCore/Core/Input.h>

#include <Magma/Core/YAMLSerializer.h>

#include <Lava/Core/Lava.h>

#include "UI/Widget.h"
#include "AI/AI.h"
#include "VersionControl/VersionControl.h"

#include "AssetImporter.h"
#include "ScriptManager.h"
#include "Widget.h"

using namespace VolcaniCore;
using namespace Magma;

namespace fs = std::filesystem;

namespace Magma {

static void ProjectLoad(const std::string& path, Project& project);
static void ProjectSave(const Project& project);
static void ProjectSaveRuntime(const Project& project);

static Map<std::string, Ref<ScriptModule>> s_TabModules;

static Ref<UI::Root> s_TitleBar;
static Ref<UI::Root> s_StartScreen;
static Ref<UI::Root> s_ComponentEditorScreen;
static Ref<UI::Root> s_FlowEditorScreen;
static Ref<UI::Root> s_ProjectEditorScreen;

static Ref<Texture> s_Logo;

static UI::UIManager* s_UIManager = nullptr;
static VC::VCManager* s_VCManager = nullptr;

void Editor::Open() {
	// Editor::RegisterInterface();

	s_UIManager = new UI::UIManager();
	s_UIManager->Init();
	s_UIManager->Load("Editor/assets/start.json");

	s_VCManager = new VC::VCManager();
	s_VCManager->Init();

	Application::PushDir();
	s_Logo = AssetImporter::GetTexture("Editor/assets/images/VolcanicDisplay.png");
	Application::PopDir();
}

void Editor::Close() {
	CloseProject();
	CloseLavaFlow();

	s_VCManager->Shutdown();
	delete s_VCManager;
	s_UIManager->Shutdown();
	delete s_UIManager;
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
	s_UIManager->Update(ts);

	if(Mode == EditorMode::Project) {
		s_ProjectEditorScreen->Update(ts);
		for(auto& tab : m_Tabs)
			tab.OnUpdate(ts);
	}
}

void Editor::Render() {
	s_UIManager->BeginFrame();

	RenderTitleBar();

	if(Mode == EditorMode::None)
		RenderStartScreen();
	else if(Mode == EditorMode::Component)
		RenderComponentEditor();
	else if(Mode == EditorMode::Flow)
		RenderFlowEditor();
	else if(Mode == EditorMode::Project)
		RenderProjectEditor();

	s_UIManager->EndFrame();
}

void Editor::RenderTitleBar() {
	// s_TitleBar->Render();

	ImGuiWindowFlags titleBarFlags = ImGuiWindowFlags_NoTitleBar
								   | ImGuiWindowFlags_NoCollapse
								   | ImGuiWindowFlags_NoResize
								   | ImGuiWindowFlags_NoMove
								   | ImGuiWindowFlags_NoDocking
								   | ImGuiWindowFlags_NoScrollbar
								   | ImGuiWindowFlags_NoSavedSettings
								   | ImGuiWindowFlags_NoScrollWithMouse;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize({ viewport->Size.x, 60.0f });
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { 0.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.01f, 0.01f, 0.01f, 1.0f });

	ImGui::Begin("##TitleBar", nullptr, titleBarFlags);
	{
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor(1);
		ImGui::SetCursorPos({ 5.0f, 5.0f });
		ImGui::Image((ImTextureID)(intptr_t)s_Logo->ID, ImVec2(50.0f, 50.0f), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();

		ImGui::SetWindowFontScale(2.0f);
		if(Mode == EditorMode::Project)
			ImGui::Text("Project Editor");
		else if(Mode == EditorMode::Flow)
			ImGui::Text("Flow Editor");
		else if(Mode == EditorMode::Component)
			ImGui::Text("Component Editor");
		else
			ImGui::Text("Magma Editor v0.1.0");
		ImGui::SetWindowFontScale(1.0f);
		ImGui::SameLine();

		ImVec2 menuSize = { ImGui::GetContentRegionAvail().x - 100.0f, 25.0f };
		ImGui::BeginChild("##MenuBar", menuSize, 0,
			titleBarFlags | ImGuiWindowFlags_MenuBar);
		{
			if(ImGui::BeginMenuBar()) {
				if(ImGui::BeginMenu("Test")) {
					ImGui::MenuItem("Item 1");
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			if(Mode == EditorMode::Project)
				// m_ScriptObj.Call("OnRenderMenuBar");
			;
		}
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::SetCursorPosX(
			ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x - 45.0f);
		if(ImGui::Button("-"))
			Application::GetWindow()->Minimize();
		ImGui::SameLine();
		if(ImGui::Button("X"))
			Application::Close();

		// ImVec2 tabBarSize = { ImGui::GetContentRegionAvail().x - 100.0f, 0.0f };
		// ImGui::SetCursorPos({ 60.0f, 35.0f });
		// if(Mode == EditorMode::Project
		// && ImGui::BeginChild("##TabBar", tabBarSize))
		// {
		// 	auto tabBarFlags = ImGuiTabBarFlags_Reorderable
		// 					 | ImGuiTabBarFlags_NoTooltip;
		// 	if(ImGui::BeginTabBar("##Tabs", tabBarFlags)) {
		// 		auto plusFlags = ImGuiTabItemFlags_Trailing
		// 					   | ImGuiTabItemFlags_NoReorder;
		// 		if(ImGui::TabItemButton("+", plusFlags))
		// 			ImGui::OpenPopup("New Tab");
		// 		NewTab();

		// 		uint32_t tabToDelete = 0;
		// 		for(uint32_t i = 0; i < m_Tabs.Count(); i++) {
		// 			Tab& tab = m_Tabs[i];
		// 			TabState state =
		// 				UIRenderer::DrawTab(tab.Name, tab.Type != "Project");
		// 			if(state.Closed)
		// 				tabToDelete = i + 1;
		// 			else if(state.Clicked)
		// 				SetTab(i);
		// 		}

		// 		if(tabToDelete != 0)
		// 			CloseTab(tabToDelete);

		// 		ImGui::EndTabBar();
		// 	}

		// 	ImGui::EndChild();
		// }
	}
	ImGui::End();
}

void Editor::RenderStartScreen() {
	// static uint32_t mode = 0; // 1 = Project, 2 = Flow, 3 = Component

	// s_StartScreen->Render();

	// if(s_StartScreen->Find("ProjectButton")->State.Clicked) {
	// 	mode = 1;
	// 	auto w = s_StartScreen->Find("ProjectWindow");
	// 	w->Visible = true;
	// }
	// else if(s_StartScreen->Find("FlowButton")->State.Clicked)
	// 	mode = 2;
	// else if(s_StartScreen->Find("ComponentButton")->State.Clicked)
	// 	mode = 3;

	auto flags = ImGuiWindowFlags_NoDecoration
			   | ImGuiWindowFlags_NoMove
			   | ImGuiWindowFlags_NoDocking;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos({ viewport->Pos.x, viewport->Pos.y + 60.0f });
	ImGui::SetNextWindowSize({ viewport->Size.x, viewport->Size.y - 60.0f });
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

	ImGui::Begin("##StartScreen", nullptr, flags);
	{
		ImGui::PopStyleVar(3);

		ImVec2 size = { 400, ImGui::GetContentRegionAvail().y };
		auto childFlags = ImGuiChildFlags_Border;
		ImGui::BeginChild("Options", size, childFlags);
		{
			if(ImGui::Button("New Project")) {

			}
			if(ImGui::Button("Open Project")) {
				// FileDialog dialog;
				// dialog.Width = 500;
				// dialog.Height = 500;
				// dialog.Title = "Open Project";
				// dialog.StartPath = Application::GetHomeDir();
				// dialog.Extensions = { "proj.yml" };
				// dialog.OnSelect =
				// 	[&](std::string& path)
				// 	{
				// 		Application::GetWindow()->Maximize();
				// 		OpenProject(path);
				// 		Mode = EditorMode::Project;
				// 	};
				// dialog.Draw();
			}
			if(ImGui::Button("New LavaFlow")) {

			}
			if(ImGui::Button("Open LavaFlow")) {
			// 	FileDialog dialog;
			// 	dialog.Width = 500;
			// 	dialog.Height = 500;
			// 	dialog.Title = "Open LavaFlow";
			// 	dialog.StartPath = Application::GetHomeDir();
			// 	dialog.Extensions = { "flow.yml" };
			// 	dialog.OnSelect =
			// 		[&](std::string& path)
			// 		{
			// 			OpenLavaFlow(path);
			// 			Mode = EditorMode::Flow;
			// 		};
			// 	dialog.Draw();
			}
			if(ImGui::Button("New Component")) {
				
			}
			if(ImGui::Button("Open Component")) {
				// FileDialog dialog;
				// dialog.Width = 500;
				// dialog.Height = 500;
				// dialog.Title = "Open Component";
				// dialog.StartPath = Application::GetHomeDir();
				// dialog.Extensions = { "comp.yml" };
				// dialog.OnSelect =
				// 	[&](std::string& path)
				// 	{
				// 		OpenComponent(path);
				// 		Mode = EditorMode::Component;
				// 	};
				// dialog.Draw();
			}
			if(ImGui::Button("MagmAI")) {
				// m_Assistant.HandleRequest("What is the meaning of life?");
			}
			if(ImGui::Button("Repo")) {
				VC::Repo repo;
				auto path = Application::GetHomeDir() + "\\TestRepo";
				repo.Init(path);
			}
		}
		ImGui::EndChild();

		// s_WelcomeImage.Draw();
	}
	ImGui::End();
}

void Editor::RenderComponentEditor() {
	// s_ComponentEditorWindow->Render();

	auto flags = ImGuiWindowFlags_NoDecoration
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoDocking;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos({ viewport->Pos.x, viewport->Pos.y + 60.0f });
	ImGui::SetNextWindowSize({ viewport->Size.x, viewport->Size.y - 60.0f });
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

	ImGui::Begin("##ComponentEditor", nullptr, flags);
	{
		ImGui::PopStyleVar(3);

		ImGui::Text("Name: %s", m_Component.Name.c_str());
		ImGui::Text("Path: %s", m_Component.Path.c_str());

		if(ImGui::Button("Generate Premake")) {
			Application::PushDir();
			auto proj = FileUtils::ReadFile("Editor/assets/scripts/component.lua");
			auto core = FileUtils::ReadFile("Editor/assets/scripts/component-core.lua");
			auto editor = FileUtils::ReadFile("Editor/assets/scripts/component-editor.lua");
			// auto runtime = FileUtils::ReadFile("Editor/assets/scripts/component-runtime.lua");
			Application::PopDir();

			auto volcPath =
				fs::path(Application::GetLibraryDir()) / "VolcaniCore" / "src";
			auto magmaPath =
				fs::path(Application::GetLibraryDir()) / "Magma" / "src";
			Replace(proj, "${0}", m_Component.Name);
			Replace(proj, "${1}", fs::path(m_Component.Path).generic_string());
			Replace(proj, "${2}", volcPath.generic_string());
			Replace(proj, "${3}", magmaPath.generic_string());

			auto premakePath =
				fs::path(m_Component.Path) / "Build" / "premake5.lua";
			File premake(premakePath.string(), true);
			premake.Write(proj);

			for(auto dep : m_Component.CoreDeps)
				premake.Write(
					std::string("table.insert(CoreDeps, ") + "\"" + dep + "\")");
			for(auto dep : m_Component.EditorDeps)
				premake.Write(
					std::string("table.insert(EditorDeps, ") + "\"" + dep + "\")");

			for(auto dep : m_Component.CoreDeps) {
				premake.Write(
					std::string("VendorPaths[\"") + dep + "\"] = \"%{VendorPath}/" + dep + "\"");
			}
			for(auto dep : m_Component.EditorDeps) {
				premake.Write(
					std::string("VendorPaths[\"") + dep + "\"] = \"%{VendorPath}/" + dep + "\"");
			}

			// for(auto dep : m_Component.CoreDeps) {
			// 	premake.Write(
			// 		std::string("IncludePaths[\"") + dep + "\"] = \"%{VendorPath}/" + dep + "\"");
			// }
			// for(auto dep : m_Component.EditorDeps) {
			// 	premake.Write(
			// 		std::string("IncludePaths[\"") + dep + "\"] = \"%{VendorPath}/" + dep + "\"");
			// }

			for(auto dep : m_Component.CoreDeps)
				premake.Write(std::string("include \"Dependencies/") + dep + "\"");
			for(auto dep : m_Component.EditorDeps)
				premake.Write(std::string("include \"Dependencies/") + dep + "\"");

			Replace(core, "${0}", m_Component.Name);
			Replace(editor, "${0}", m_Component.Name);

			premake.Write(core);
			premake.Write(editor);
			VOLCANICORE_LOG_INFO("Completed: Generate Premake");
		}

		static std::string console;
		if(ImGui::Button("Run Premake")) {
			Application::PushDir();
			std::string command;
			command = ".vendor\\premake\\bin\\Windows\\premake5.exe gmake --file=\"";
			command += (fs::path(m_Component.Path) / "Build" / "premake5.lua").string();
			command += "\"";

			command += " 2>&1";
			FILE* pipe = _popen(command.c_str(), "r");

			if(!pipe) {
				VOLCANICORE_LOG_ERROR("Failed: Build for Windows");
				return;
			}
			else {
				char buffer[128];
				
				while(!feof(pipe)) {
					if(fgets(buffer, 128, pipe) != nullptr)
						console.append(buffer);
				}
			}

			_pclose(pipe);
			Application::PopDir();
			VOLCANICORE_LOG_INFO("Completed: Run Premake");
		}

		if(ImGui::Button("Build for Windows")) {
			std::string command;
			command = "cd /D \"";
			command += (fs::path(m_Component.Path) / "Build" / "Platform" / "gcc-Windows").make_preferred().string();
			command += "\" && ";
			command += "mingw32-make.exe -f Makefile";

			command += " 2>&1";
			FILE* pipe = _popen(command.c_str(), "r");

			if(!pipe) {
				VOLCANICORE_LOG_ERROR("Failed: Build for Windows");
				return;
			}
			else {
				char buffer[128];
				
				while(!feof(pipe)) {
					if(fgets(buffer, 128, pipe) != nullptr)
						console.append(buffer);
				}
			}

			std::cout << console;
			_pclose(pipe);
			VOLCANICORE_LOG_INFO("Completed: Build for Windows");
		}

		if(ImGui::Button("Load Component")) {
			auto path = fs::path(m_Component.Path) / "Build" / "Platform" / "gcc-Windows" / "lib" / m_Component.Name;
			Lava::LoadComponent(path.string() + "-Core.dll");
			VOLCANICORE_LOG_INFO("Completed: Load Component");
		}

		ImGui::SeparatorText("Dependencies");
		ImGui::Text("Core");
		ImGui::Indent();

		if(ImGui::Button("Add Dependency##0")) {

		}

		for(auto dep : m_Component.CoreDeps)
			ImGui::Text(dep.c_str());
		ImGui::Unindent();

		ImGui::Text("Editor");
		ImGui::Indent();

		if(ImGui::Button("Add Dependency##1")) {

		}

		for(auto dep : m_Component.EditorDeps)
			ImGui::Text(dep.c_str());
		ImGui::Unindent();

		ImGui::BeginChild("##Console", { 900, 400 }, ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
		ImGui::TextUnformatted(console.c_str());
		ImGui::EndChild();
	}
	ImGui::End();
}

void Editor::RenderFlowEditor() {
	// s_FlowEditorWindow->Render();

}

void Editor::RenderProjectEditor() {
	// s_AppEditorWindow->Render();

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

	Application::GetWindow()->SetTitle(title);
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
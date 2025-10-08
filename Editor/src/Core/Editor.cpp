#include <drogon/drogon.h>

#include "Editor.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <fstream>
#include <iostream>
#include <regex>

#include <miniz-cpp/zip_file.hpp>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Algo.h>
#include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Core/Log.h>
#include <VolcaniCore/Core/Input.h>

#include <Magma/Core/YAMLSerializer.h>

#include <Lava/Core/Lava.h>

#include "UI/Widget.h"
#include "AI/AI.h"

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

static Ref<UI::Window> s_TitleBar;
static Ref<UI::Window> s_StartWindow;
static Ref<UI::Window> s_ComponentEditorWindow;
static Ref<UI::Window> s_FlowEditorWindow;
static Ref<UI::Window> s_AppEditorWindow;

static Ref<Texture> s_Logo;

using namespace drogon;

static std::thread s_ServerThread;

void Editor::Open() {
	Editor::RegisterInterface();

	m_App = CreateRef<Lava::App>();

	Application::PushDir();
	s_Logo =
		AssetImporter::GetTexture("Editor/assets/images/VolcanicDisplay.png");

	// if(FileUtils::PathExists("Editor/.cache/data.yml")) {
	// 	YAML::Node file;
	// 	try {
	// 		file = YAML::LoadFile("Editor/.cache/data.yml");
	// 	}
	// 	catch(YAML::ParserException e) {
	// 		VOLCANICORE_LOG_INFO("Malformed cache file");
	// 		return;
	// 	}
	// 	catch(YAML::BadFile e) {
	// 		VOLCANICORE_LOG_INFO("Bad cache file");
	// 		return;
	// 	}

	// 	auto data = file["EditorData"];
	// 	m_Cache.PastProjects = data["PastProjects"].as<List<std::string>>();
	// 	m_Cache.PastLavaFlows = data["PastLavaFlows"].as<List<std::string>>();
	// }

	Application::PopDir();

	s_ServerThread = std::thread(
		[]()
		{
			app().registerHandler("/v1/rpc",
				[](const HttpRequestPtr& req, Func<void, const HttpResponsePtr&>&& callback)
				{
					Json::CharReaderBuilder builder;
					Json::CharReader* reader = builder.newCharReader();
					Json::Value request;
					reader->parse(req->bodyData(),
						req->bodyData() + req->bodyLength(), &request, nullptr);

					auto method = request["Method"].asString();
					auto data = request["Data"].asString();
					VOLCANICORE_LOG_INFO("Method: '%s', Data: '%s'", method.c_str(), data.c_str());

					Json::Value response;
					response["Status"] = "Ok";
					callback(HttpResponse::newHttpJsonResponse(response));
				}, { HttpMethod::Post });

			app().addListener("127.0.0.1", 8848)
				.run();
		});

	s_ServerThread.detach();
}

void Editor::Close() {
	CloseProject();
	CloseLavaFlow();
	m_App.reset();
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
	if(Mode == EditorMode::Component)
		s_ComponentEditorWindow->Update(ts);
	else if(Mode == EditorMode::Flow)
		s_FlowEditorWindow->Update(ts);
	else if(Mode == EditorMode::Project) {
		s_AppEditorWindow->Update(ts);
		for(auto& tab : m_Tabs)
			tab.OnUpdate(ts);
	}
}

void Editor::Render() {
	if(Mode == EditorMode::None)
		RenderStartScreen();
	else if(Mode == EditorMode::Component)
		RenderComponentEditor();
	else if(Mode == EditorMode::Flow)
		RenderFlowEditor();
	else if(Mode == EditorMode::Project)
		RenderProjectEditor();
}

void Editor::RenderTitleBar() {
	if(s_TitleBar) {
		s_TitleBar->Render();
		return;
	}

	s_TitleBar = CreateRef<UI::Window>("TitleBar");
	s_TitleBar->IsRoot = true;
	s_TitleBar->Color = Vec4(0.2f, 0.2f, 0.2f, 1.0f);
	auto image = CreateRef<UI::Image>("WelcomeImage");
	image->Content = s_Logo;
	image->x = 5;
	image->y = 5;
	image->Width = 50;
	image->Height = 50;
	s_TitleBar->Add(image);

	// ImGuiWindowFlags titleBarFlags = ImGuiWindowFlags_NoTitleBar
	// 							   | ImGuiWindowFlags_NoCollapse
	// 							   | ImGuiWindowFlags_NoResize
	// 							   | ImGuiWindowFlags_NoMove
	// 							   | ImGuiWindowFlags_NoDocking
	// 							   | ImGuiWindowFlags_NoScrollbar
	// 							   | ImGuiWindowFlags_NoSavedSettings
	// 							   | ImGuiWindowFlags_NoScrollWithMouse;

	// const ImGuiViewport* viewport = ImGui::GetMainViewport();
	// ImGui::SetNextWindowPos(viewport->Pos);
	// ImGui::SetNextWindowSize({ viewport->Size.x, 60.0f });
	// ImGui::SetNextWindowViewport(viewport->ID);
	// ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	// ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.01f, 0.01f, 0.01f, 1.0f });

	// ImGui::Begin("##TitleBar", nullptr, titleBarFlags);
	// {
	// 	ImGui::PopStyleVar(3);
	// 	ImGui::PopStyleColor(1);

	// 	ImGui::SetWindowFontScale(2.0f);
	// 	if(Mode == EditorMode::Project)
	// 		ImGui::Text("Project Editor");
	// 	else if(Mode == EditorMode::Flow)
	// 		ImGui::Text("Flow Editor");
	// 	else if(Mode == EditorMode::Component)
	// 		ImGui::Text("Component Editor");
	// 	else
	// 		ImGui::Text("Magma Editor v0.1");
	// 	ImGui::SetWindowFontScale(1.0f);
	// 	ImGui::SameLine();

	// 	ImVec2 menuSize = { ImGui::GetContentRegionAvail().x - 100.0f, 25.0f };
	// 	ImGui::BeginChild("##MenuBar", menuSize, 0,
	// 		titleBarFlags | ImGuiWindowFlags_MenuBar);
	// 	{
	// 		if(ImGui::BeginMenuBar()) {
	// 			if(ImGui::BeginMenu("Test")) {
	// 				ImGui::MenuItem("Item 1");
	// 				ImGui::EndMenu();
	// 			}
	// 			ImGui::EndMenuBar();
	// 		}

	// 		if(Mode == EditorMode::Project)
	// 			// m_ScriptObj.Call("OnRenderMenuBar");
	// 		;
	// 	}
	// 	ImGui::EndChild();

	// 	ImGui::SameLine();
	// 	ImGui::SetCursorPosX(
	// 		ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x - 45.0f);
	// 	if(ImGui::Button("-"))
	// 		Application::GetWindow()->Minimize();
	// 	ImGui::SameLine();
	// 	if(ImGui::Button("X"))
	// 		Application::Close();

	// 	ImVec2 tabBarSize = { ImGui::GetContentRegionAvail().x - 100.0f, 0.0f };
	// 	ImGui::SetCursorPos({ 60.0f, 35.0f });
	// 	if(Mode == EditorMode::Project
	// 	&& ImGui::BeginChild("##TabBar", tabBarSize))
	// 	{
	// 		auto tabBarFlags = ImGuiTabBarFlags_Reorderable
	// 						 | ImGuiTabBarFlags_NoTooltip;
	// 		if(ImGui::BeginTabBar("##Tabs", tabBarFlags)) {
	// 			auto plusFlags = ImGuiTabItemFlags_Trailing
	// 						   | ImGuiTabItemFlags_NoReorder;
	// 			if(ImGui::TabItemButton("+", plusFlags))
	// 				ImGui::OpenPopup("New Tab");
	// 			NewTab();

	// 			uint32_t tabToDelete = 0;
	// 			for(uint32_t i = 0; i < m_Tabs.Count(); i++) {
	// 				Tab& tab = m_Tabs[i];
	// 				TabState state =
	// 					UIRenderer::DrawTab(tab.Name, tab.Type != "Project");
	// 				if(state.Closed)
	// 					tabToDelete = i + 1;
	// 				else if(state.Clicked)
	// 					SetTab(i);
	// 			}

	// 			if(tabToDelete != 0)
	// 				CloseTab(tabToDelete);

	// 			ImGui::EndTabBar();
	// 		}

	// 		ImGui::EndChild();
	// 	}
	// }
	// ImGui::End();
}

void Editor::RenderStartScreen() {
	static uint32_t mode = 0; // 1 = Project, 2 = Flow, 3 = Component

	if(s_StartWindow) {
		s_StartWindow->Render();
		if(s_StartWindow->Find("ProjectButton")->State.Clicked) {
			mode = 1;
			auto w = s_StartWindow->Find("ProjectWindow");
			w->Visible = true;
		}
		else if(s_StartWindow->Find("FlowButton")->State.Clicked)
			mode = 2;
		else if(s_StartWindow->Find("ComponentButton")->State.Clicked)
			mode = 3;

		// ...

		return;
	}

	s_StartWindow = CreateRef<UI::Window>("Window");
	s_StartWindow->IsRoot = true;
	s_StartWindow->Color = Vec4(0.2f, 0.2f, 0.2f, 1.0f);

	auto projectButton = CreateRef<UI::Button>("ProjectButton");
	projectButton->Add(CreateRef<UI::Text>("Project"));
	projectButton->Width = 50.0f;
	projectButton->Height = 30.0f;
	// projectButton->AddLine();
	s_StartWindow->Add(projectButton);

	auto projectDialog = CreateRef<UI::FileDialog>("ProjectDialog");
	projectDialog->Width = 500;
	projectDialog->Height = 500;
	projectDialog->OpenDir = false;
	projectDialog->StartPath = Application::GetHomeDir();
	projectDialog->Extensions = { "proj.yml" };
	projectDialog->OnSelect =
		[](const fs::path& path) {
			// Application::GetWindow()->Maximize();
			// OpenProject(path);
			// Mode = EditorMode::Project;
		};
	s_StartWindow->Add(projectDialog);

	auto flowButton = CreateRef<UI::Button>("FlowButton");
	flowButton->Add(CreateRef<UI::Text>("Component"));
	flowButton->Width = 50.0f;
	flowButton->Height = 30.0f;
	// flowButton->AddLine();
	s_StartWindow->Add(flowButton);

	auto floatDialog = CreateRef<UI::FileDialog>("FlowDialog");
	floatDialog->Width = 500;
	floatDialog->Height = 500;
	floatDialog->OpenDir = false;
	floatDialog->StartPath = Application::GetHomeDir();
	floatDialog->Extensions = { "flow.yml" };
	floatDialog->OnSelect =
		[](const fs::path& path) {
			// Application::GetWindow()->Maximize();
			// OpenProject(path);
			// Mode = EditorMode::Project;
		};
	s_StartWindow->Add(floatDialog);

	auto componentButton = CreateRef<UI::Button>("ComponentButton");
	componentButton->Add(CreateRef<UI::Text>("Component"));
	componentButton->Width = 50.0f;
	componentButton->Height = 30.0f;
	// componentButton->AddLine();
	s_StartWindow->Add(componentButton);

	auto componentDialog = CreateRef<UI::FileDialog>("ComponentDialog");
	componentDialog->Width = 500;
	componentDialog->Height = 500;
	componentDialog->OpenDir = false;
	componentDialog->StartPath = Application::GetHomeDir();
	componentDialog->Extensions = { "comp.yml" };
	componentDialog->OnSelect =
		[](const fs::path& path) {
			// Application::GetWindow()->Maximize();
			// OpenProject(path);
			// Mode = EditorMode::Project;
		};
	s_StartWindow->Add(componentDialog);
}

void Editor::RenderComponentEditor() {
	if(s_ComponentEditorWindow) {
		s_ComponentEditorWindow->Render();

		// ...

		return;
	}

	s_ComponentEditorWindow = CreateRef<UI::Window>("ComponentEditorWindow");

	// auto flags = ImGuiWindowFlags_NoDecoration
	// 		| ImGuiWindowFlags_NoMove
	// 		| ImGuiWindowFlags_NoDocking;

	// const ImGuiViewport* viewport = ImGui::GetMainViewport();
	// ImGui::SetNextWindowPos({ viewport->Pos.x, viewport->Pos.y + 60.0f });
	// ImGui::SetNextWindowSize({ viewport->Size.x, viewport->Size.y - 60.0f });
	// ImGui::SetNextWindowViewport(viewport->ID);
	// ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	// ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	// ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

	// ImGui::Begin("##ComponentEditor", nullptr, flags);
	// {
	// 	ImGui::PopStyleVar(3);

	// 	ImGui::Text("Name: %s", m_Component.Name.c_str());
	// 	ImGui::Text("Path: %s", m_Component.Path.c_str());

	// 	if(ImGui::Button("Generate Premake")) {
	// 		Application::PushDir();
	// 		auto proj = FileUtils::ReadFile("Editor/assets/scripts/component.lua");
	// 		auto core = FileUtils::ReadFile("Editor/assets/scripts/component-core.lua");
	// 		auto editor = FileUtils::ReadFile("Editor/assets/scripts/component-editor.lua");
	// 		// auto runtime = FileUtils::ReadFile("Editor/assets/scripts/component-runtime.lua");
	// 		Application::PopDir();

	// 		auto volcPath =
	// 			fs::path(Application::GetLibraryDir()) / "VolcaniCore" / "src";
	// 		auto magmaPath =
	// 			fs::path(Application::GetLibraryDir()) / "Magma" / "src";
	// 		Replace(proj, "${0}", m_Component.Name);
	// 		Replace(proj, "${1}", fs::path(m_Component.Path).generic_string());
	// 		Replace(proj, "${2}", volcPath.generic_string());
	// 		Replace(proj, "${3}", magmaPath.generic_string());

	// 		auto premakePath =
	// 			fs::path(m_Component.Path) / "Build" / "premake5.lua";
	// 		File premake(premakePath.string(), true);
	// 		premake.Write(proj);

	// 		for(auto dep : m_Component.CoreDeps)
	// 			premake.Write(
	// 				std::string("table.insert(CoreDeps, ") + "\"" + dep + "\")");
	// 		for(auto dep : m_Component.EditorDeps)
	// 			premake.Write(
	// 				std::string("table.insert(EditorDeps, ") + "\"" + dep + "\")");

	// 		for(auto dep : m_Component.CoreDeps) {
	// 			premake.Write(
	// 				std::string("VendorPaths[\"") + dep + "\"] = \"%{VendorPath}/" + dep + "\"");
	// 		}
	// 		for(auto dep : m_Component.EditorDeps) {
	// 			premake.Write(
	// 				std::string("VendorPaths[\"") + dep + "\"] = \"%{VendorPath}/" + dep + "\"");
	// 		}

	// 		// for(auto dep : m_Component.CoreDeps) {
	// 		// 	premake.Write(
	// 		// 		std::string("IncludePaths[\"") + dep + "\"] = \"%{VendorPath}/" + dep + "\"");
	// 		// }
	// 		// for(auto dep : m_Component.EditorDeps) {
	// 		// 	premake.Write(
	// 		// 		std::string("IncludePaths[\"") + dep + "\"] = \"%{VendorPath}/" + dep + "\"");
	// 		// }

	// 		for(auto dep : m_Component.CoreDeps)
	// 			premake.Write(std::string("include \"Dependencies/") + dep + "\"");
	// 		for(auto dep : m_Component.EditorDeps)
	// 			premake.Write(std::string("include \"Dependencies/") + dep + "\"");

	// 		Replace(core, "${0}", m_Component.Name);
	// 		Replace(editor, "${0}", m_Component.Name);

	// 		premake.Write(core);
	// 		premake.Write(editor);
	// 		VOLCANICORE_LOG_INFO("Completed: Generate Premake");
	// 	}

	// 	static std::string console;
	// 	if(ImGui::Button("Run Premake")) {
	// 		Application::PushDir();
	// 		std::string command;
	// 		command = ".vendor\\premake\\bin\\Windows\\premake5.exe gmake --file=\"";
	// 		command += (fs::path(m_Component.Path) / "Build" / "premake5.lua").string();
	// 		command += "\"";

	// 		command += " 2>&1";
	// 		FILE* pipe = _popen(command.c_str(), "r");

	// 		if(!pipe) {
	// 			VOLCANICORE_LOG_ERROR("Failed: Build for Windows");
	// 			return;
	// 		}
	// 		else {
	// 			char buffer[128];
				
	// 			while(!feof(pipe)) {
	// 				if(fgets(buffer, 128, pipe) != nullptr)
	// 					console.append(buffer);
	// 			}
	// 		}

	// 		_pclose(pipe);
	// 		Application::PopDir();
	// 		VOLCANICORE_LOG_INFO("Completed: Run Premake");
	// 	}

	// 	if(ImGui::Button("Build for Windows")) {
	// 		std::string command;
	// 		command = "cd /D \"";
	// 		command += (fs::path(m_Component.Path) / "Build" / "Platform" / "gcc-Windows").make_preferred().string();
	// 		command += "\" && ";
	// 		command += "mingw32-make.exe -f Makefile";

	// 		command += " 2>&1";
	// 		FILE* pipe = _popen(command.c_str(), "r");

	// 		if(!pipe) {
	// 			VOLCANICORE_LOG_ERROR("Failed: Build for Windows");
	// 			return;
	// 		}
	// 		else {
	// 			char buffer[128];
				
	// 			while(!feof(pipe)) {
	// 				if(fgets(buffer, 128, pipe) != nullptr)
	// 					console.append(buffer);
	// 			}
	// 		}

	// 		std::cout << console;
	// 		_pclose(pipe);
	// 		VOLCANICORE_LOG_INFO("Completed: Build for Windows");
	// 	}

	// 	if(ImGui::Button("Load Component")) {
	// 		auto path = fs::path(m_Component.Path) / "Build" / "Platform" / "gcc-Windows" / "lib" / m_Component.Name;
	// 		Lava::LoadComponent(path.string() + "-Core.dll");
	// 		VOLCANICORE_LOG_INFO("Completed: Load Component");
	// 	}

	// 	ImGui::SeparatorText("Dependencies");
	// 	ImGui::Text("Core");
	// 	ImGui::Indent();

	// 	if(ImGui::Button("Add Dependency##0")) {

	// 	}

	// 	for(auto dep : m_Component.CoreDeps)
	// 		ImGui::Text(dep.c_str());
	// 	ImGui::Unindent();

	// 	ImGui::Text("Editor");
	// 	ImGui::Indent();

	// 	if(ImGui::Button("Add Dependency##1")) {

	// 	}

	// 	for(auto dep : m_Component.EditorDeps)
	// 		ImGui::Text(dep.c_str());
	// 	ImGui::Unindent();

	// 	ImGui::BeginChild("##Console", { 900, 400 }, ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
	// 	ImGui::TextUnformatted(console.c_str());
	// 	ImGui::EndChild();
	// }
	// ImGui::End();
}

void Editor::RenderFlowEditor() {
	if(s_FlowEditorWindow) {
		s_FlowEditorWindow->Render();

		// ...

		return;
	}

	s_FlowEditorWindow = CreateRef<UI::Window>("FlowEditorWindow");

}

void Editor::RenderProjectEditor() {
	if(s_AppEditorWindow) {
		s_AppEditorWindow->Render();

		// ...

		return;
	}

	s_AppEditorWindow = CreateRef<UI::Window>("FlowEditorWindow");

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
	UI::Panel::RegisterInterface();
	UI::Tab::RegisterInterface();
	UI::WidgetManager::RegisterInterface();
}

}
#include "Editor.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <fstream>
#include <iostream>
#include <regex>

#include <asio.hpp>
#include <asio/ssl.hpp>

#include <miniz-cpp/zip_file.hpp>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Core/Log.h>
#include <VolcaniCore/Core/Input.h>

#include <Magma/Graphics/RendererAPI.h>
#include <Magma/Core/YAMLSerializer.h>

#include "UI/UI.h"

#include "AssetImporter.h"
#include "ScriptManager.h"

using asio::ip::tcp;

using namespace VolcaniCore;
using namespace Magma;
using namespace Magma::UI;

namespace fs = std::filesystem;

namespace Magma {

static void ProjectLoad(const std::string& path, Project& project);
static void ProjectSave(const Project& project);
static void ProjectSaveRuntime(const Project& project);

static UI::Image s_WelcomeImage;
static Map<std::string, Ref<ScriptModule>> s_TabModules;

static std::string HTTPGet(asio::io_context& io, asio::ssl::context& sslContext,
						 std::string host, std::string path, int depth = 5) {
	if (depth == 0) throw std::runtime_error("Too many redirects");

	asio::ssl::stream<tcp::socket> socket(io, sslContext);

	// Resolve + connect
	tcp::resolver resolver(io);
	auto endpoints = resolver.resolve(host, "443");
	asio::connect(socket.lowest_layer(), endpoints);

	// TLS handshake
	socket.handshake(asio::ssl::stream_base::client);

	// Send HTTP request
	std::string req =
		"GET " + path + " HTTP/1.1\r\n"
		"Host: " + host + "\r\n"
		"User-Agent: MagmaEditor/1.0\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n\r\n";
	asio::write(socket, asio::buffer(req));

	// Read full response
	std::string response;
	char buf[8192];
	while(true) {
		asio::error_code ec;
		std::size_t n = socket.read_some(asio::buffer(buf), ec);
		if(n > 0)
			response.append(buf, buf + n);
		if(ec == asio::error::eof)
			break;
		if(ec)
			throw asio::system_error(ec);
	}

	auto statusEnd = response.find("\r\n");
	if(statusEnd == std::string::npos)
		throw std::runtime_error("Bad HTTP response");
	std::string line = response.substr(0, statusEnd);
	int statusCode = 0;
	sscanf(line.c_str(), "HTTP/1.%*d %d", &statusCode);

	// Headers end
	auto headerEnd = response.find("\r\n\r\n");
	if(headerEnd == std::string::npos)
		throw std::runtime_error("Bad HTTP response 2");
	std::string headers = response.substr(0, headerEnd);
	std::string body = response.substr(headerEnd + 4);

	// Handle redirect
	if(statusCode == 301 || statusCode == 302) {
		std::smatch m;
		std::regex re("Location: (.+)\r");
		if (std::regex_search(headers, m, re)) {
			std::string location = m[1];
			std::cout << "Redirect -> " << location << "\n";

			// Parse new URL
			if(location.rfind("https://", 0) == 0)
				location = location.substr(8);
			auto slash = location.find('/');
			std::string newHost = location.substr(0, slash);
			std::string newPath = location.substr(slash);

			return HTTPGet(io, sslContext, newHost, newPath, depth - 1);
		}
		throw std::runtime_error("Redirect without Location header");
	}

	// Success
	if(statusCode == 200)
		return body;

	throw std::runtime_error("HTTP error " + std::to_string(statusCode));
}

static void DownloadLavaFlow(const Cache::Flow& flow) {
	try {
		asio::io_context io;
		asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12_client);

		std::string body =
			HTTPGet(io, ssl_ctx, "github.com",
					flow.URL + "/archive/refs/heads/main.zip");

		miniz_cpp::zip_file zip(std::vector<uint8_t>(body.begin(), body.end()));

		std::string cachePath = "Editor/.cache/.lavaflow";
		for(auto& name : zip.namelist()) {
			fs::path outPath =
				cachePath / fs::path(name).lexically_relative(flow.Name + "-main");
			outPath = outPath.generic_string();
			std::cout << outPath << "\n";

			// Directory entry
			if (name.back() == '/') {
				fs::create_directories(outPath);
				continue;
			}

			fs::create_directories(fs::path(outPath).parent_path());
			std::ofstream out(outPath, std::ios::binary);
			std::string data = zip.read(name);
			out.write(data.data(), data.size());
			out.close();
		}
	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << "\n";
	}
}

void Editor::Open() {
	Editor::RegisterInterface();

	m_App = CreateRef<Lava::App>();

	Application::PushDir();
	s_WelcomeImage.Content =
		AssetImporter::GetTexture("Editor/assets/images/VolcanicDisplay.png");

	s_WelcomeImage.UsePosition = false;
	s_WelcomeImage.Width = 590;
	s_WelcomeImage.Height = 590;

	if(FileUtils::PathExists("Editor/.cache/data.yml")) {
		YAML::Node file;
		try {
			file = YAML::LoadFile("Editor/.cache/data.yml");
		}
		catch(YAML::ParserException e) {
			VOLCANICORE_LOG_INFO("Malformed cache file");
			return;
		}
		catch(YAML::BadFile e) {
			VOLCANICORE_LOG_INFO("Bad cache file");
			return;
		}

		auto data = file["EditorData"];
		m_Cache.PastProjects = data["PastProjects"].as<List<std::string>>();
		for(auto node : data["PastLavaFlows"])
			m_Cache.PastLavaFlows.Emplace(
				node["Flow"]["Name"].as<std::string>(),
				node["Flow"]["Path"].as<std::string>(),
				node["Flow"]["URL"].as<std::string>(),
				node["Flow"]["Local"].as<bool>()
			);
	}

	Application::PopDir();
}

void Editor::Close() {
	CloseProject();
	m_App.reset();
}

Ref<ScriptModule> Editor::GetModule(const std::string& name) {
	return s_TabModules[name];
}

Ref<ScriptClass> Editor::GetTabClass(const std::string& name) {
	return s_TabModules[name]->GetClass(name);
}

Ref<ScriptClass> Editor::GetPanelClass(const std::string& tab,
									   const std::string& name)
{
	return s_TabModules[tab]->GetClass(name);
}

void Editor::Load(const CommandLineArgs& args) {
	// if(args["--project"]) {
	// 	NewProject(args["--project"]);

	// 	if(args["--export"]) {
	// 		auto path = (args["--export"].Args[0]);
	// 		ExportProject(path);
	// 		Application::Close();
	// 		return;
	// 	}

	// 	NewTab(CreateRef<ProjectTab>());
	// }
	// for(auto& path : args["--scene"])
	// 	NewTab(CreateRef<SceneTab>(path));
	// for(auto& path : args["--ui"])
	// 	NewTab(CreateRef<UITab>(path));
}

void Editor::Update(TimeStep ts) {
	for(auto tab : m_Tabs)
		tab.OnUpdate(ts);
}

void Editor::Render() {
	bool dockspaceOpen = true;
	ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGuiWindowFlags windowFlags = m_Tabs ? ImGuiWindowFlags_MenuBar : 0;
	windowFlags |= ImGuiWindowFlags_NoDocking
				 | ImGuiWindowFlags_NoMove
				 | ImGuiWindowFlags_NoDecoration
				 | ImGuiWindowFlags_NoBackground
				 | ImGuiWindowFlags_NoInputs
				 | ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpaceWindow", &dockspaceOpen, windowFlags);
	{
		ImGui::PopStyleVar(3);

		// Menu bar here

		auto tabBarFlags = ImGuiTabBarFlags_Reorderable
						 | ImGuiTabBarFlags_TabListPopupButton;
		if(m_Tabs && ImGui::BeginTabBar("Tabs", tabBarFlags)) {
			auto plusFlags = ImGuiTabItemFlags_Trailing
						   | ImGuiTabItemFlags_NoReorder;
			// if(ImGui::TabItemButton("+", plusFlags))
			// 	menu.tab.newTab = true;

			uint32_t tabToDelete = 0;
			for(uint32_t i = 0; i < m_Tabs.Count(); i++) {
				Tab& tab = m_Tabs[i];
				TabState state =
					UIRenderer::DrawTab(tab.Name, tab.Type == "Project");
				if(state.Closed)
					tabToDelete = i + 1;
				else if(state.Clicked)
					SetTab(i);
			}

			if(tabToDelete != 0)
				CloseTab(tabToDelete);

			ImGui::EndTabBar();

			// GetProjectTab()->RenderButtons();
		}

		if(!m_Tabs)
			RenderSplashScreen();
		else {
			ImGuiID dockspaceID = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

			if(GetCurrentTab()->Type != "None")
				GetCurrentTab()->OnRender();
			else
				RenderEmptyTab(m_Tabs[m_CurrentTab]);
		}
	}
	ImGui::End();

	// if(menu.project.newProject)
	// 	NewProject();
	// if(menu.project.openProject)
	// 	OpenProject();
	// if(menu.project.runProject)
	// 	RunProject();
	// if(menu.project.exportProject)
	// 	ExportProject(m_Project.ExportPath);

	// if(menu.tab.newTab)
	// 	NewTab();
	// if(menu.tab.newScene)
	// 	NewTab(CreateRef<SceneTab>());
	// if(menu.tab.newUI)
	// 	NewTab(CreateRef<UITab>());

	// if(menu.tab.openTab)
	// 	LoadTab();
	// if(menu.tab.openScene)
	// 	OpenTab(TabType::Scene);
	// if(menu.tab.openUI)
	// 	OpenTab(TabType::UI);

	// if(menu.tab.reopenTab)
	// 	ReopenTab();
	// if(menu.tab.closeTab)
	// 	CloseTab(m_CurrentTab);
}

void Editor::RenderEmptyTab(Tab& tab) {
	ImGui::Begin("##Empty");
	{
		for(auto object : m_LavaFlow.ObjectList) {
			if(ImGui::Button(("New " + object).c_str()))
				tab.Init(object);
			if(ImGui::Button(("Open " + object).c_str())) {
				tab.Init(object);
				tab.OnLoad();
			}
		}
	}
	ImGui::End();
}

void Editor::RenderSplashScreen() {
	auto flags = ImGuiWindowFlags_NoDecoration
			   | ImGuiWindowFlags_NoMove
			   | ImGuiWindowFlags_NoDocking;
	ImGui::Begin("##Welcome", nullptr, flags);
	{
		ImGui::Text("Magma Editor");

		ImVec2 size = { 400, ImGui::GetContentRegionAvail().y };
		auto childFlags = ImGuiChildFlags_Border;
		ImGui::BeginChild("Options", size, childFlags);
		{
			if(ImGui::Button("Open Project")) {
				FileDialog dialog;
				dialog.Width = 500;
				dialog.Height = 500;
				dialog.Title = "Open Project";
				dialog.StartPath = Application::GetHomeDir();
				dialog.Extensions = { "magma.proj" };
				dialog.OnSelect =
					[&](std::string& path)
					{
						NewProject(path);
						OpenTab("Project");
					};
				dialog.Draw();
			}
			if(ImGui::Button("New Project")) {

			}
			if(ImGui::Button("Open LavaFlow")) {

			}
			if(ImGui::Button("New LavaFlow")) {

			}

			UIRenderer::DrawFileDialog("Open Project");
			UIRenderer::DrawFileDialog("New Project");
			UIRenderer::DrawFileDialog("Open LavaFlow");
			UIRenderer::DrawFileDialog("New LavaFlow");

			ImGui::SeparatorText("Previous projects");
			for(auto proj : m_Cache.PastProjects)
				if(ImGui::Selectable(proj.c_str()))
					NewProject(proj);

			ImGui::SeparatorText("Previous LavaFlows");
			ImVec2 size = { 590, 40 };

			for(auto flow : m_Cache.PastLavaFlows) {
				auto buttonFlags = ImGuiButtonFlags_EnableNav
								 | ImGuiButtonFlags_MouseButtonLeft;

				ImGui::SetNextItemAllowOverlap();
				ImVec2 min = ImGui::GetCursorScreenPos();
				ImGui::InvisibleButton(flow.Path.c_str(), size, buttonFlags);
				ImVec2 max = ImGui::GetCursorScreenPos();
				bool clicked = ImGui::IsItemActivated();

				ImGui::SetCursorScreenPos({ min.x + 10, min.y });
				ImGui::SetNextItemAllowOverlap();
				ImGui::Text(flow.Name.c_str());
				ImGui::SetNextItemAllowOverlap();
				ImGui::Text(flow.Path.c_str());
				ImGui::SetCursorScreenPos(max);
				ImGui::Dummy({ 0, 0 });

				ImVec2 p0 = min;
				ImVec2 p1 = ImVec2(max.x + size.x, max.y);

				ImDrawList* drawList = ImGui::GetWindowDrawList();
				drawList->AddRect(p0, p1, ImColor(255, 255, 255, 255), 0, 0, 1.0f);

				if(clicked)
					LoadLavaFlow(flow.Path);
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();

		s_WelcomeImage.Draw();
	}
	ImGui::End();
}

void Editor::SetTab(uint32_t idx) {
	auto title = "Magma Editor: " + m_Project.Name;

	if(GetCurrentTab())
		GetCurrentTab()->OnDeselect();

	if(idx) {
		m_CurrentTab = idx;
		Tab* tab = GetCurrentTab();
		tab->OnSelect();
		title += " - " + tab->Name;
	}

	Application::GetWindow()->SetTitle(title);
}

void Editor::NewTab(Tab tab) {
	// menu.tab.newScene = false;
	// menu.tab.newUI = false;

	s_Instance->m_Tabs.Add(tab);
	s_Instance->SetTab(s_Instance->m_Tabs.Count());
}

void Editor::NewTab() {
	// menu.tab.newTab = false;

	Tab& newTab = m_Tabs.Emplace();
	newTab.Name = "New Tab";
	NewTab(newTab);
}

void Editor::OpenTab(const std::string& type) {
	Tab& newTab = m_Tabs.Emplace();
	newTab.Init(type);
	SetTab(m_Tabs.Count());
}

void Editor::LoadTab(const std::string& type) {
	// m_ScriptObj.Call("LoadTab", type);
}

void Editor::ReopenTab() {
	// menu.tab.reopenTab = false;

	if(m_ClosedTabs)
		NewTab(m_ClosedTabs.Pop());
}

void Editor::CloseTab(uint32_t idx) {
	// menu.tab.closeTab = false;
	if(idx == 0)
		return;

	m_ClosedTabs.Add(m_Tabs.Pop(idx - 1));
	if(idx == m_CurrentTab)
		SetTab(idx - 1);
}

void Editor::NewProject() {
}

void Editor::NewProject(const std::string& volcPath) {
	ProjectLoad(volcPath, m_Project);

	std::string pathName = m_Project.LavaFlow;
	if(pathName.substr(0, 2) == "./")
		m_LavaFlow.Path =
			(fs::path(m_Project.Path) / ".lavaflow" / pathName).string();
	else
		m_LavaFlow.Path = (fs::path("Editor") / ".cache" / ".lavaflow" / pathName).string();

	m_LavaFlow.Path = fs::canonical(m_LavaFlow.Path).string();

	LoadLavaFlow(m_LavaFlow.Path);

	// m_AssetManager.Load(m_Project.Path);
	auto rootPath = fs::path(volcPath).parent_path();

	m_App->AppLoad =
		[this, rootPath](Ref<ScriptModule>& script)
		{
			auto scriptPath = rootPath / "Project" / "App.as";
			script = AssetImporter::GetScript(scriptPath.string());
		};

	m_App->Running = false;
	// m_ScriptObj.Call("NewProject");
}

void Editor::OpenProject() {
	UI::FileDialog dialog;
	dialog.Title = "Open Project";
	dialog.StartPath = ".";
	dialog.Extensions = { "magma.proj" };
	dialog.OnSelect =
		[&](std::string& path)
		{
			CloseProject();
			NewProject(path);
			OpenTab("Project");
		};
	dialog.Draw();

	// menu.project.openProject = false;
}

void Editor::RunProject() {
	// menu.project.runProject = false;

// 	std::string command;
// #ifdef VOLCANICENGINE_WINDOWS
// 	command = "Runtime --project ";
// 	command += m_Project.ExportPath + "\\.volc.proj";
// #elif VOLCANICENGINE_LINUX
// 	command = "Runtime --project ";
// 	command += m_Project.ExportPath + "/.volc.proj";
// #endif
// 	system(command.c_str());
}

void Editor::CloseProject() {
	m_Tabs.Clear();
	m_ClosedTabs.Clear();
	m_CurrentTab = 0;

	// m_AssetManager.Save();
	// m_AssetManager.Clear();

	if(m_Project.Path != "")
		ProjectSave(m_Project);
	m_Project = { };
}

void Editor::ExportProject() {
	// IGFD::FileDialogConfig config;
	// config.path = ".";
	// auto instance = ImGuiFileDialog::Instance();
	// instance->OpenDialog("ChooseDir", "Choose Directory", nullptr, config);

	// std::string exportPath = "";
	// if(instance->Display("ChooseDir")) {
	// 	if(instance->IsOk())
	// 		exportPath = instance->GetCurrentPath();

	// 	instance->Close();
	// }
	// menu.project.exportProjectTo = false;

	// if(exportPath == "")
	// 	return;

	// ExportProject(exportPath);
}

void Editor::ExportProject(const std::string& exportPath) {
	// menu.project.exportProject = false;

	if(fs::is_regular_file(exportPath)) {
		VOLCANICORE_LOG_INFO("'%s' is not a valid directory path");
		return;
	}

}


void Editor::LoadLavaFlow(const std::string& pathName) {
	// Load LavaFlow data from file
	auto flowFile = (fs::path(pathName) / ".flow.yml").string();
	YAML::Node file;
	try {
		file = YAML::LoadFile(flowFile);
	} catch (YAML::ParserException e) {
		VOLCANICORE_LOG_INFO("File '%s' is not well formatted", flowFile.c_str());
		return;
	} catch (YAML::BadFile e) {
		VOLCANICORE_LOG_INFO("File '%s' is bad", flowFile.c_str());
		return;
	}

	auto lavaFlowNode = file["LavaFlow"];
	VOLCANICORE_ASSERT(lavaFlowNode);

	m_LavaFlow.Name = lavaFlowNode["Name"].as<std::string>();
	m_LavaFlow.ObjectList = lavaFlowNode["Objects"].as<List<std::string>>();

	List<std::string> includes =
	{
		"Editor/scripts",
		"Editor/scripts/Magma",
		"Editor/scripts/Magma/Editor",
		"Editor/scripts/Magma/UI",
		"Editor/scripts/Magma/Object",
		"Lava/scripts",
		"Lava/scripts/Lava",
		"Lava/scripts/Lava/Physics",
		"Lava/scripts/Lava/ECS",
		"Lava/scripts/Lava/Component",
		"Lava/scripts/Lava/Component/Ash",
		"Lava/scripts/Lava/Component/Igneous",
		"Lava/scripts/Lava/Component/Silica",
		"Lava/scripts/Lava/Component/Cinder",
		"Lava/scripts/Lava/Component/Pyro",
	};

	for(auto type : m_LavaFlow.ObjectList) {
		auto uiPath = fs::path(m_LavaFlow.Path) / "Editor" / "UI" / type;
		auto moduleData =
			ScriptManager::LoadScript(
				FileUtils::GetFiles(uiPath.string(), { ".as" }),
				false, nullptr, type, includes);

		s_TabModules[type] = CreateRef<ScriptModule>(moduleData);
	}
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
		.WriteKey("Name").Write(project.Name)
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
	Panel::RegisterInterface();
	Tab::RegisterInterface();
	// Widget::RegisterInterface();
}

}
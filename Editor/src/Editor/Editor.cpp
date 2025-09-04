#include "Editor.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include <fstream>
#include <iostream>

#include <asio.hpp>
#include <asio/ssl.hpp>

#include <miniz-cpp/zip_file.hpp>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Core/Log.h>
#include <VolcaniCore/Core/Input.h>

#include <Magma/Graphics/RendererAPI.h>
#include <Magma/Core/YAMLSerializer.h>

#include "UI/UIRenderer.h"

#include "AssetImporter.h"
#include "ScriptManager.h"

using asio::ip::tcp;

using namespace VolcaniCore;
using namespace Magma;
using namespace Magma::UI;

namespace fs = std::filesystem;

namespace Magma {
struct {
	struct {
		bool newProject    = false;
		bool openProject   = false;
		bool runProject    = false;
		bool exportProject = false;
		bool newLavaFlow   = false;
		bool openLavaFlow  = false;
	} project;

	struct {
		bool newTab      = false;
		bool openTab     = false;
		bool reopenTab   = false;
		bool closeTab    = false;
	} tab;
} static menu;

static void ProjectLoad(const std::string& path, Project& project);
static void ProjectSave(const Project& project);
static void ProjectSaveRuntime(const Project& project);


std::string download_url(asio::io_context& io, asio::ssl::context& ssl_ctx,
						 std::string host, std::string path, int depth = 5) {
	if (depth == 0) throw std::runtime_error("Too many redirects");

	asio::ssl::stream<tcp::socket> socket(io, ssl_ctx);

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
	for (;;) {
		asio::error_code ec;
		std::size_t n = socket.read_some(asio::buffer(buf), ec);
		if (n > 0) response.append(buf, buf + n);
		if (ec == asio::error::eof) break;
		if (ec) throw asio::system_error(ec);
	}

	// Parse status line
	auto status_line_end = response.find("\r\n");
	if (status_line_end == std::string::npos) throw std::runtime_error("Bad HTTP response");
	std::string status_line = response.substr(0, status_line_end);
	int status_code = 0;
	sscanf(status_line.c_str(), "HTTP/1.%*d %d", &status_code);

	// Headers end
	auto header_end = response.find("\r\n\r\n");
	if (header_end == std::string::npos) throw std::runtime_error("Bad HTTP response 2");
	std::string headers = response.substr(0, header_end);
	std::string body = response.substr(header_end + 4);

	// Handle redirect
	if (status_code == 301 || status_code == 302) {
		std::smatch m;
		std::regex re("Location: (.+)\r");
		if (std::regex_search(headers, m, re)) {
			std::string location = m[1];
			std::cout << "Redirect -> " << location << "\n";

			// Parse new URL
			if (location.rfind("https://", 0) == 0) location = location.substr(8);
			auto slash = location.find('/');
			std::string new_host = location.substr(0, slash);
			std::string new_path = location.substr(slash);

			return download_url(io, ssl_ctx, new_host, new_path, depth - 1);
		}
		throw std::runtime_error("Redirect without Location header");
	}

	// Success
	if (status_code == 200) {
		return body;
	}

	throw std::runtime_error("HTTP error " + std::to_string(status_code));
}

static UI::Image s_WelcomeImage;

void Editor::Open() {
	Editor::RegisterInterface();
	Panel::RegisterInterface();
	Tab::RegisterInterface();

	m_App = CreateRef<Lava::App>();

	Application::PushDir();
	s_WelcomeImage.Content =
		AssetImporter::GetTexture("Editor/assets/images/VolcanicDisplay.png");

	s_WelcomeImage.UsePosition = false;
	s_WelcomeImage.Width = 700;
	s_WelcomeImage.Height = 700;

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
		m_Cache.PreviousProjects = data["LastProjects"].as<List<std::string>>();
		m_Cache.PreviousLavaFlows = data["LastLavaFlows"].as<List<std::string>>();
	}

	for(auto flow : m_Cache.PreviousLavaFlows) {
		if(FileUtils::PathExists("Editor/.cache/.lavaflow/" + flow))
			continue;

		try {
			asio::io_context io;
			asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12_client);

			// Start with GitHub repo zip
			std::string body = download_url(io, ssl_ctx,
				"github.com",
				"/LavaFlow-Technologies/" + flow + "/archive/refs/heads/main.zip");

			// Open output file
			std::ofstream out("Editor/.cache/.repo/" + flow + "", std::ios::binary);
			out.write(body.data(), body.size());
			out.close();

			miniz_cpp::zip_file zip(zip_path);

			for (auto& name : zip.namelist()) {
				std::string out_path = out_dir + "/" + name;

				if (name.back() == '/') {
					// Directory entry
					fs::create_directories(out_path);
					continue;
				}

				fs::create_directories(fs::path(out_path).parent_path());

				auto data = zip.read(name); // returns std::string with file contents

				std::ofstream out(out_path, std::ios::binary);
				out.write(data.data(), data.size());
				out.close();

				std::cout << "Extracted: " << out_path << "\n";
			}
		}
		catch (std::exception &e) {
			std::cerr << "Error: " << e.what() << "\n";
		}
	}

	Application::PopDir();
}

void Editor::Close() {
	CloseProject();
	m_App.reset();
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
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;

	windowFlags |= ImGuiWindowFlags_NoDocking
				 | ImGuiWindowFlags_NoCollapse
				 | ImGuiWindowFlags_NoNavFocus
				 | ImGuiWindowFlags_NoTitleBar
				 | ImGuiWindowFlags_NoResize
				 | ImGuiWindowFlags_NoMove
				 | ImGuiWindowFlags_NoBackground
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

		if(m_Tabs && ImGui::BeginMainMenuBar()) {
			if(ImGui::BeginMenu("Project")) {
				if(ImGui::MenuItem("New", "Ctrl+N"))
					menu.project.newProject = true;
				if(ImGui::MenuItem("Open", "Ctrl+P"))
					menu.project.openProject = true;
				// if(ImGui::MenuItem("Run", "Ctrl+R"))
				// 	menu.project.runProject = true;

				ImGui::Separator();
				if(ImGui::MenuItem("Export"))
					menu.project.exportProject = true;

				ImGui::EndMenu();
			}

			// if(GetProjectTab() && ImGui::BeginMenu("Tab")) {
			// 	if(ImGui::MenuItem("New", "Ctrl+T")
			// 	|| Input::KeysPressed(Key::Ctrl, Key::T))
			// 		menu.tab.newTab = true;
			// 	if(ImGui::MenuItem("New Scene Tab")
			// 	|| Input::KeysPressed(Key::Ctrl, Key::T))
			// 		menu.tab.newScene = true;
			// 	if(ImGui::MenuItem("New UI Tab")
			// 	|| Input::KeysPressed(Key::Ctrl, Key::T))
			// 		menu.tab.newUI = true;
			// 	ImGui::Separator();

			// 	if(ImGui::MenuItem("Open", "Ctrl+O")
			// 	|| Input::KeyPressed(Key::O))
			// 		menu.tab.openTab = true;
			// 	if(ImGui::MenuItem("Reopen", "Ctrl+Shift+T")
			// 	|| Input::KeysPressed(Key::Ctrl, Key::Shift, Key::T))
			// 		menu.tab.reopenTab = true;
			// 	if(ImGui::MenuItem("Close", "Ctrl+W")
			// 	|| Input::KeysPressed(Key::Ctrl, Key::W))
			// 		menu.tab.closeTab = true;

			// 	ImGui::EndMenu();
			// }

			ImGui::EndMainMenuBar();
		}

		auto tabBarFlags = ImGuiTabBarFlags_Reorderable
						 | ImGuiTabBarFlags_TabListPopupButton;
		if(m_Tabs && ImGui::BeginTabBar("Tabs", tabBarFlags)) {
			auto plusFlags = ImGuiTabItemFlags_Trailing
						   | ImGuiTabItemFlags_NoReorder;
			if(ImGui::TabItemButton("+", plusFlags))
				menu.tab.newTab = true;

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

		ImGuiID dockspaceID = ImGui::GetID("DockSpace");
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

		if(!m_Tabs)
			RenderWelcomeScreen();
		else {
			// if(m_CurrentTab != 0)
			// 	GetProjectTab()->RenderEssentialPanels();

			if(GetCurrentTab()->Type != "None")
				GetCurrentTab()->OnRender();
			else
				RenderEmptyTab(m_Tabs[m_CurrentTab]);
		}
	}
	ImGui::End();

	if(menu.project.newProject)
		NewProject();
	if(menu.project.openProject)
		OpenProject();
	if(menu.project.runProject)
		RunProject();
	// if(menu.project.exportProject)
	// 	ExportProject(m_Project.ExportPath);

	if(menu.tab.newTab)
		NewTab();
	// if(menu.tab.newScene)
	// 	NewTab(CreateRef<SceneTab>());
	// if(menu.tab.newUI)
	// 	NewTab(CreateRef<UITab>());

	if(menu.tab.openTab)
		LoadTab();
	// if(menu.tab.openScene)
	// 	OpenTab(TabType::Scene);
	// if(menu.tab.openUI)
	// 	OpenTab(TabType::UI);

	if(menu.tab.reopenTab)
		ReopenTab();
	if(menu.tab.closeTab)
		CloseTab(m_CurrentTab);
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

void Editor::RenderWelcomeScreen() {
	auto flags = ImGuiWindowFlags_NoDecoration
			   | ImGuiWindowFlags_NoMove
			   | ImGuiWindowFlags_NoDocking;
	ImGui::Begin("##Welcome", nullptr, flags);
	{
		ImVec2 size = { 300, ImGui::GetContentRegionAvail().y };
		auto childFlags = ImGuiChildFlags_Border;
		ImGui::BeginChild("Options", size, childFlags);
		{
			if(ImGui::Button("Open Project"))
				menu.project.openProject = true;
			if(ImGui::Button("New Project"))
				menu.project.newProject = true;
			if(ImGui::Button("Open LavaFlow"))
				menu.project.openLavaFlow = true;
			if(ImGui::Button("New LavaFlow"))
				menu.project.newLavaFlow = true;

			ImGui::SeparatorText("Previous projects");
			for(auto prev : m_Cache.PreviousProjects)
				if(ImGui::Selectable(prev.c_str()))
					NewProject(prev);

			ImGui::SeparatorText("Previous LavaFlows");
			for(auto prev : m_Cache.PreviousProjects)
				if(ImGui::Selectable(prev.c_str()))
					NewProject(prev);

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
	menu.tab.newTab = false;

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
	std::string exts;
	// if(type == "None")
	// 	exts = ".magma.scene, .magma.ui.json";
	// else if(type == TabType::Scene)
	// 	exts = ".magma.scene";
	// else if(type == TabType::UI)
	// 	exts = ".magma.ui.json";

	IGFD::FileDialogConfig config;
	config.path = m_Project.Path;
	auto instance = ImGuiFileDialog::Instance();
	instance->OpenDialog("ChooseFile", "Choose File", exts.c_str(), config);

	if(instance->Display("ChooseFile")) {
		if(instance->IsOk()) {
			std::string path = instance->GetFilePathName();

		}

		instance->Close();
		menu.tab.openTab = false;
	}
}

void Editor::ReopenTab() {
	menu.tab.reopenTab = false;

	if(m_ClosedTabs)
		NewTab(m_ClosedTabs.Pop());
}

void Editor::CloseTab(uint32_t idx) {
	menu.tab.closeTab = false;
	if(idx == 0)
		return;

	m_ClosedTabs.Add(m_Tabs.Pop(idx - 1));
	if(idx == m_CurrentTab)
		SetTab(idx - 1);
}

void Editor::NewProject() {
	menu.project.newProject = false;

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
}

void Editor::OpenProject() {
	IGFD::FileDialogConfig config;
	config.path = ".";
	auto instance = ImGuiFileDialog::Instance();
	instance->OpenDialog("ChooseFile", "Choose File", ".magma.proj", config);

	if(instance->Display("ChooseFile", 32, { 500.0f, 600.0f })) {
		if(instance->IsOk()) {
			std::string path = instance->GetCurrentPath();
			std::string name = instance->GetFilePathName();
			auto fullPath = fs::path(path) / name;
			CloseProject();
			NewProject(fullPath.string());
			OpenTab("Project");
		}

		instance->Close();
		menu.project.openProject = false;
	}
}

void Editor::RunProject() {
	menu.project.runProject = false;

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

// void Editor::ExportProject() {
// 	IGFD::FileDialogConfig config;
// 	config.path = ".";
// 	auto instance = ImGuiFileDialog::Instance();
// 	instance->OpenDialog("ChooseDir", "Choose Directory", nullptr, config);

// 	std::string exportPath = "";
// 	if(instance->Display("ChooseDir")) {
// 		if(instance->IsOk())
// 			exportPath = instance->GetCurrentPath();

// 		instance->Close();
// 		menu.project.exportProjectTo = false;
// 	}

// 	if(exportPath == "")
// 		return;

// 	ExportProject(exportPath);
// }

void Editor::ExportProject(const std::string& exportPath) {
	menu.project.exportProject = false;

	if(fs::is_regular_file(exportPath)) {
		VOLCANICORE_LOG_INFO("'%s' is not a valid directory path");
		return;
	}

	// fs::create_directories(exportPath);
	// fs::create_directories(fs::path(exportPath) / "Class");
	// fs::create_directories(fs::path(exportPath) / "Scene");
	// fs::create_directories(fs::path(exportPath) / "UI");

	// m_Project.ExportPath = exportPath;
	// ProjectSaveRuntime(m_Project);

	// for(auto& screen : m_Project.Screens) {
	// 	auto* mod = ScriptManager::LoadScript(
	// 		(fs::path(m_Project.Path) / "Project" / "Screen" / screen.Name
	// 		).string() + ".as", false);
	// 	BinaryWriter writer(
	// 		(fs::path(exportPath) / "Class" / screen.Name).string() + ".class");
	// 	ScriptManager::SaveScript(mod, writer);
	// }

	// fs::path sceneFolder = fs::path(m_Project.Path) / "Visual" / "Scene";
	// for(auto path : FileUtils::GetFiles(sceneFolder.string())) {
	// 	Scene scene;
	// 	SceneLoader::EditorLoad(scene, path);
	// 	SceneLoader::RuntimeSave(scene, m_Project.Path, exportPath);
	// }
	// fs::path uiFolder = fs::path(m_Project.Path) / "Visual" / "UI";
	// for(auto path : FileUtils::GetFiles(uiFolder.string())) {
	// 	if(fs::path(path).filename().string() == "theme.magma.ui.json")
	// 		continue;

	// 	UIPage uiPage;
	// 	UILoader::EditorLoad(uiPage, path, UITab::GetTheme());
	// 	UILoader::RuntimeSave(uiPage, m_Project.Path, exportPath);
	// }

	// auto* mod = ScriptManager::LoadScript(
	// 	(fs::path(m_Project.Path) / "Project" / "App" / m_Project.App
	// 	).string() + ".as", false);
	// BinaryWriter writer((fs::path(exportPath) / ".volc.class").string());
	// ScriptManager::SaveScript(mod, writer);

	// m_AssetManager.RuntimeSave(exportPath);

	// auto runtimeEnv = getenv("VOLC_RUNTIME");
	// VOLCANICORE_ASSERT(runtimeEnv);
	// std::string runtimePath = runtimeEnv;
	// auto target = (fs::path(exportPath) / m_Project.App).string() + ".exe";

	// if(FileUtils::PathExists(target))
	// 	fs::remove(target);

	// fs::copy_file(runtimePath, target, fs::copy_options::overwrite_existing);
	// VOLCANICORE_LOG_INFO("Exported project successfully");
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

	// ScriptManager::LoadScript(pathName, true, nullptr, "");
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

}

}
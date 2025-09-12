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
#include "Widget.h"

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

static void DownloadLavaFlow(const std::string& url) {
	try {
		asio::io_context io;
		asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12_client);
		auto flowName = url.substr(url.rfind('/') + 1);

		std::string body =
			HTTPGet(io, ssl_ctx, "github.com",
					url + "/archive/refs/heads/main.zip");

		miniz_cpp::zip_file zip(std::vector<uint8_t>(body.begin(), body.end()));

		std::string cachePath = "Editor/.cache/.lavaflow";
		for(auto& name : zip.namelist()) {
			fs::path outPath =
				cachePath / fs::path(name).lexically_relative(flowName + "-main");
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
		m_Cache.PastLavaFlows = data["PastLavaFlows"].as<List<std::string>>();
	}

	Application::PopDir();
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
		NewProject(args["--project"]);
	if(args["--lavaflow"])
		NewProject(args["--lavaflow"]);
}

void Editor::Update(TimeStep ts) {
	// for(auto& tab : m_Tabs)
	// 	tab.OnUpdate(ts);
}

void Editor::Render() {
	if(!m_Tabs) {
		RenderSplashScreen();
		return;
	}

	WidgetRenderer::BeginFrame();

	RenderTitleBar();

	bool dockspaceOpen = true;
	ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGuiWindowFlags windowFlags;
	windowFlags |= ImGuiWindowFlags_NoDocking
				 | ImGuiWindowFlags_NoMove
				 | ImGuiWindowFlags_NoDecoration
				 | ImGuiWindowFlags_NoBackground
				 | ImGuiWindowFlags_NoInputs
				 | ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos({ viewport->Pos.x, viewport->Pos.y + 25.0f });
	ImGui::SetNextWindowSize({ viewport->Size.x, viewport->Size.y - 25.0f });
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpaceWindow", &dockspaceOpen, windowFlags);
	{
		ImGui::PopStyleVar(3);
		ImGuiID dockspaceID = ImGui::GetID("DockSpace");
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
		GetCurrentTab()->OnRender();
	}
	ImGui::End();

	WidgetRenderer::EndFrame();
}

void Editor::RenderSplashScreen() {
	auto flags = ImGuiWindowFlags_NoDecoration
			   | ImGuiWindowFlags_NoMove
			   | ImGuiWindowFlags_NoDocking;
	ImGui::Begin("##Welcome", nullptr, flags);
	{
		ImVec2 curPos = ImGui::GetCursorPos();
		ImVec2 buttonPos =
			{ curPos.x + ImGui::GetContentRegionAvail().x - 40, curPos.y };
		ImGui::SetCursorPos(buttonPos);
		if(ImGui::Button("-"))
			Application::GetWindow()->Minimize();
		ImGui::SameLine();
		if(ImGui::Button("X"))
			Application::Close();
		ImGui::SetCursorPos(curPos);
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

			for(auto path : m_Cache.PastLavaFlows) {
				// auto buttonFlags = ImGuiButtonFlags_EnableNav
				// 				 | ImGuiButtonFlags_MouseButtonLeft;

				// ImGui::SetNextItemAllowOverlap();
				// ImVec2 min = ImGui::GetCursorScreenPos();
				// ImGui::InvisibleButton(flow.Path.c_str(), size, buttonFlags);
				// ImVec2 max = ImGui::GetCursorScreenPos();
				// bool clicked = ImGui::IsItemActivated();

				// ImGui::SetCursorScreenPos({ min.x + 10, min.y });
				// ImGui::SetNextItemAllowOverlap();
				// ImGui::Text(flow.Name.c_str());
				// ImGui::SetNextItemAllowOverlap();
				// ImGui::Text(flow.Path.c_str());
				// ImGui::SetCursorScreenPos(max);
				// ImGui::Dummy({ 0, 0 });

				// ImVec2 p0 = min;
				// ImVec2 p1 = ImVec2(max.x + size.x, max.y);

				// ImDrawList* drawList = ImGui::GetWindowDrawList();
				// drawList->AddRect(p0, p1, ImColor(255, 255, 255, 255), 0, 0, 1.0f);

				// if(clicked)
				// 	LoadLavaFlow(flow.Path);
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();

		s_WelcomeImage.Draw();
	}
	ImGui::End();
}

static bool s_NewTab = false;

void Editor::RenderTitleBar() {
	ImGuiWindowFlags titleBarFlags = ImGuiWindowFlags_NoTitleBar
								   | ImGuiWindowFlags_NoCollapse
								   | ImGuiWindowFlags_NoResize
								   | ImGuiWindowFlags_NoMove
								   | ImGuiWindowFlags_NoDocking
								   | ImGuiWindowFlags_NoScrollbar
								   | ImGuiWindowFlags_NoSavedSettings
								   | ImGuiWindowFlags_NoScrollWithMouse;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(ImVec2{ viewport->WorkSize.x, 60.0f });
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2{ 0.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.01f, 0.01f, 0.01f, 1.0f });

	ImGui::Begin("Window-Frame-Titlebar", nullptr, titleBarFlags);
	{
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor(1);
		auto image = s_WelcomeImage;
		image.x = 5;
		image.y = 5;
		image.Width = 50;
		image.Height = 50;
		image.UsePosition = true;
		image.Draw();
		ImGui::SameLine();

		ImGui::SetWindowFontScale(2.0f);
		ImGui::Text("Magma Editor");
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
			// m_ScriptObj.Call("OnRenderMenuBar");
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

		ImVec2 tabBarSize = { ImGui::GetContentRegionAvail().x - 100.0f, 0.0f };
		ImGui::SetCursorPos(ImVec2(60.0f, 35.0f));
		ImGui::BeginChild("##TabBar", tabBarSize);
		{
			auto tabBarFlags = ImGuiTabBarFlags_Reorderable
							 | ImGuiTabBarFlags_NoTooltip;
			if(ImGui::BeginTabBar("##Tabs", tabBarFlags)) {
				auto plusFlags = ImGuiTabItemFlags_Trailing
							   | ImGuiTabItemFlags_NoReorder;
				if(ImGui::TabItemButton("+", plusFlags))
					ImGui::OpenPopup("New Tab");
				NewTab();

				uint32_t tabToDelete = 0;
				for(uint32_t i = 0; i < m_Tabs.Count(); i++) {
					Tab& tab = m_Tabs[i];
					TabState state =
						UIRenderer::DrawTab(tab.Name, tab.Type != "Project");
					if(state.Closed)
						tabToDelete = i + 1;
					else if(state.Clicked)
						SetTab(i);
				}

				if(tabToDelete != 0)
					CloseTab(tabToDelete);

				ImGui::EndTabBar();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void Editor::NewTab() {
	if(ImGui::BeginPopup("New Tab")) {
		for(auto object : m_LavaFlow.ObjectList) {
			if(object == "Project")
				continue;

			ImGui::SeparatorText(object.c_str());

			if(ImGui::Button(("New " + object).c_str())) {
				OpenTab(object);
				ImGui::CloseCurrentPopup();
			}
			if(ImGui::Button(("Open " + object).c_str())) {
				OpenTab(object);
				// tab.OnLoad();
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
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
		Tab* tab = GetCurrentTab();
		tab->OnSelect();

		title += " - " + tab->Name;
	}

	Application::GetWindow()->SetTitle(title);
}

void Editor::NewProject() {

}

void Editor::NewProject(const std::string& volcPath) {
	ProjectLoad(volcPath, m_Project);
	auto [found, i] =
		m_Cache.PastProjects.Find([&](auto& path) { return path == volcPath; });
	if(!found)
		m_Cache.PastProjects.Add(volcPath);

	Application::GetWindow()->Maximize();
	m_LavaFlow.Path = m_Project.LavaFlow;
	LoadLavaFlow(m_LavaFlow.Path);
	OpenTab("Project");
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

	UIRenderer::DrawFileDialog("Open Project");
}

void Editor::RunProject() {

}

void Editor::CloseProject() {
	m_Tabs.Clear();
	m_ClosedTabs.Clear();
	m_CurrentTab = 0;

	if(m_Project.Path != "")
		ProjectSave(m_Project);
	m_Project = { };
}

void Editor::ExportProject() {

}

void Editor::ExportProject(const std::string& exportPath) {
	if(fs::is_regular_file(exportPath)) {
		VOLCANICORE_LOG_INFO("'%s' is not a valid directory path");
		return;
	}

}

void Editor::NewLavaFlow(const std::string& path) {
	
}

void Editor::LoadLavaFlow(const std::string& path) {
	CloseLavaFlow();

	// Load LavaFlow data from file
	auto flowFile = (fs::path(path) / ".flow.yml").string();
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

	m_LavaFlow.Path = path;
	m_LavaFlow.Name = path.substr(path.rfind("/"));
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

void Editor::CloseLavaFlow() {
	s_TabModules.clear();
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
	Panel::RegisterInterface();
	Tab::RegisterInterface();
	WidgetRenderer::RegisterInterface();
}

}
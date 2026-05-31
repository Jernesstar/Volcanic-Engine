#include "Editor.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>
#include <iterator>

#define RAPIDJSON_ASSERT(x) ((void)0)
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>

#define IM_ASSERT(x) ((void)0)
#define assert(x) ((void)0)
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <ImGuizmo/ImGuizmo.h>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Window/Events.h>

#include <Engine/App/App.h>
#include <Engine/Graphics/Renderer.h>
#include <Engine/Graphics/Renderer2D.h>
#include <Engine/Script/ScriptGlue.h>
#include <Engine/Scene/Scene.h>

#include "./SceneRenderer.h"
#include "../Asset/AssetManager.h"
#include "../Asset/AssetImporter.h"
#include "../UI/Core/UIRenderer.h"
#include "../UI/Editor/Panels.h"

#include "SceneLoader.h"
#include "ScriptManager.h"

using namespace VolcaniCore;
using namespace VolcanicEngine;
using namespace VolcanicEngine::Graphics;
using namespace VolcanicEngine::Script;

namespace VolcanicEditor {

static Project s_Project;
static Ref<App> s_App;
static Ref<Project> s_CurrentProject;
static Ref<Scene> s_CurrentScene;
static ECS::Entity s_Selected;

static Ref<EditorAssetManager> s_AssetManager;

static EditorMode s_EditorMode = EditorMode::Edit;

static Ref<std::thread> s_AppThread;
static std::mutex s_Mutex;
static std::condition_variable s_Condition;
static EditorMode s_State = EditorMode::Edit;
static bool s_Updated = false;
static bool s_Debugging = false;

static SceneHierarchyPanel  s_Hierarchy;
static SceneVisualizerPanel s_Visualizer;
static ComponentEditorPanel s_ComponentEditor;
static ContentBrowserPanel  s_ContentBrowser;
static AssetEditorPanel     s_AssetEditor;
static ConsolePanel         s_Console;

static bool s_DockspaceBuilt = false;

static TimeStep s_TimeStep;
struct {
	struct {
		bool newProject    = false;
		bool openProject   = false;
		bool runProject    = false;
		bool exportProject = false;
		bool exportProjectTo = false;

		bool addScreen     = false;
	} project;

	struct {
		bool newTab      = false;
		bool newScene    = false;
		bool newUI       = false;
		bool openTab     = false;
		bool openScene   = false;
		bool openUI      = false;
		bool reopenTab   = false;
		bool closeTab    = false;
	} tab;
} static menu;

// Layout ID constants (cached after first build)
static ImGuiID s_DockMain   = 0;
static ImGuiID s_DockLeft   = 0;
static ImGuiID s_DockCenter = 0;
static ImGuiID s_DockRight  = 0;
static ImGuiID s_DockBottom = 0;

// ── Helpers ───────────────────────────────────────────────────────────────────

ECS::Entity Editor::GetSelected() {
	return s_Selected;
}
void Editor::SetSelected(ECS::Entity e) {
	s_Selected = e;
}
void Editor::ClearSelected() {
	s_Selected = ECS::Entity{ };
}

EditorMode Editor::GetMode() {
	return s_EditorMode;
}

static void BuildDockLayout(ImGuiID dockspaceID) {
	ImGui::DockBuilderRemoveNode(dockspaceID);
	ImGui::DockBuilderAddNode(dockspaceID,
		ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetMainViewport()->Size);

	s_DockMain = dockspaceID;

	// Split off left (hierarchy) and bottom (console + content + asset)
	ImGui::DockBuilderSplitNode(s_DockMain, ImGuiDir_Left,
		0.20f, &s_DockLeft, &s_DockMain);
	ImGui::DockBuilderSplitNode(s_DockMain, ImGuiDir_Down,
		0.28f, &s_DockBottom, &s_DockCenter);

	// Split off right (component editor) from center
	ImGui::DockBuilderSplitNode(s_DockCenter, ImGuiDir_Right,
		0.22f, &s_DockRight, &s_DockCenter);

	// Assign panels
	ImGui::DockBuilderDockWindow("Scene Hierarchy",  s_DockLeft);
	ImGui::DockBuilderDockWindow("Scene Visualizer", s_DockCenter);
	ImGui::DockBuilderDockWindow("Component Editor", s_DockRight);

	// Bottom row — all three share the same node (tabbed)
	ImGui::DockBuilderDockWindow("Content Browser", s_DockBottom);
	ImGui::DockBuilderDockWindow("Asset Editor", s_DockBottom);
	ImGui::DockBuilderDockWindow("Console", s_DockBottom);

	ImGui::DockBuilderFinish(dockspaceID);
}

static void DrawMainMenuBar() {
	if(!ImGui::BeginMainMenuBar())
		return;

	if(ImGui::BeginMenu("Project")) {
		if(ImGui::MenuItem("New",  "Ctrl+N")) { }
		if(ImGui::MenuItem("Open", "Ctrl+P")) { }
		if(ImGui::MenuItem("Save", "Ctrl+S")) { }
		ImGui::Separator();
		if(ImGui::MenuItem("Run", "Ctrl+R"))
			Editor::OnPlay();
		ImGui::Separator();
		if(ImGui::MenuItem("Export")) { }
		if(ImGui::MenuItem("Export To")) { }
		ImGui::EndMenu();
	}

	if(ImGui::BeginMenu("Scene")) {
		if(ImGui::MenuItem("New Scene")) { }
		if(ImGui::MenuItem("Open Scene")) { }
		if(ImGui::MenuItem("Save Scene")) { }
		ImGui::EndMenu();
	}

	ImGui::Separator();
	if(s_EditorMode == EditorMode::Edit) {
		if(ImGui::MenuItem("Play"))
			Editor::OnPlay();
	}
	if(s_EditorMode == EditorMode::Play) {
		if(ImGui::MenuItem("Pause"))
			Editor::OnPause();
	}
	else if(s_EditorMode == EditorMode::Pause) {
		if(ImGui::MenuItem("Resume"))
			Editor::OnResume();
	}
	if(s_EditorMode != EditorMode::Edit) {
		if(ImGui::MenuItem("Stop"))
			Editor::OnStop();
	}


	ImGui::EndMainMenuBar();
}

void Editor::Init(const CommandLineArgs& args) {
	Log::Init();
	Renderer::Init();
	UIRenderer::Init();

	ScriptEngine::Init();
	ScriptGlue::RegisterInterface();

	s_AssetManager = CreateRef<EditorAssetManager>();

	s_App = CreateRef<App>();
	s_App->AppLoad =
		[](Ref<ScriptModule>& script)
		{
			script = AssetImporter::GetScript("App/Game.as");
		};
	s_App->SceneLoad =
		[](Scene& scene)
		{
			Str scenePath = "App/Scene/" + scene.Name + ".scene";
			SceneLoader::EditorLoad(scene, scenePath);
		};
	s_App->Log =
		[](const Str& str)
		{
		};
	s_App->Running = false;

	if(args["--open_project"]) {
		Str path = args["--open_project"];
		Editor::OpenProject(path);
	}
	if(args["--new_project"]) {
		Str path = args["--new_project"];
		Editor::NewProject(path);
	}
	if(args["--open_scene"]) {
		Str name = args["--open_scene"];
		Editor::OpenScene(name);
	}
}

void Editor::Close() {
	OnStop();

	s_CurrentScene.reset();
	s_App.reset();
	s_AssetManager.reset();

	ScriptEngine::Shutdown();
	UIRenderer::Close();
	Renderer::Close();
}

void Editor::Update(TimeStep ts) {
	UIRenderer::BeginFrame();
	Renderer::BeginFrame();
	// ImGuizmo::BeginFrame();

	if(s_EditorMode == EditorMode::Play) {
		if(s_Debugging) {
			std::lock_guard<std::mutex> lock(s_Mutex);
			s_TimeStep = ts;
			s_State = s_EditorMode;
			s_Updated = true;
			s_Condition.notify_one();
		}
		else
			s_App->OnUpdate(ts);

		return;
	}

	if(!s_CurrentScene)
		return;

	s_Hierarchy.Update(ts);
	s_Visualizer.Update(ts);
	s_ComponentEditor.Update(ts);
	s_ContentBrowser.Update(ts);
	s_AssetEditor.Update(ts);
	s_Console.Update(ts);
}

void Editor::Render() {
	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(vp->Pos);
	ImGui::SetNextWindowSize(vp->Size);
	ImGui::SetNextWindowViewport(vp->ID);

	ImGuiWindowFlags hostFlags =
		ImGuiWindowFlags_NoDocking
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoNavFocus
		| ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::Begin("##DockHost", nullptr, hostFlags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspaceID = ImGui::GetID("MainDockSpace");
	ImGui::DockSpace(dockspaceID,
		{ 0.0f, 0.0f }, ImGuiDockNodeFlags_PassthruCentralNode);

	if(!s_DockspaceBuilt) {
		BuildDockLayout(dockspaceID);
		s_DockspaceBuilt = true;
	}

	ImGui::End();

	DrawMainMenuBar();

	s_Hierarchy.Draw();
	s_Visualizer.Draw();
	s_ComponentEditor.Draw();
	s_ContentBrowser.Draw();
	s_AssetEditor.Draw();
	s_Console.Draw();

	Renderer::EndFrame();
	UIRenderer::EndFrame();
}

Project& Editor::GetProject() {
	return s_Project;
}

void Editor::OpenProject(const Str& path) {
	Application::PushDir(path);
	s_AssetManager->LoadRegistry();
}

void Editor::NewProject(const Str& path) {

}

void Editor::SaveProject() {

}

void Editor::NewScene(const Str& path) {

}

void Editor::OpenScene(const Str& name) {
	s_CurrentScene = CreateRef<Scene>(name);
	SceneLoader::EditorLoad(*s_CurrentScene, "App/Scene/" + name + ".scene");
	s_Hierarchy.SetContext(s_CurrentScene.get());
	s_Visualizer.SetContext(s_CurrentScene.get());
}

void Editor::SaveScene(const Str& name) {

}

void Editor::SaveScene() {
	SaveScene(s_CurrentScene->Name);
}

void Editor::OnPlay(bool debug) {
	if(s_EditorMode != EditorMode::Edit)
		return;

	Log::Info("OnPlay");
	s_EditorMode = EditorMode::Play;
	SaveScene();

	s_Debugging = debug;
	if(!s_Debugging) {
		App::Get()->Running = true;
		App::Get()->OnLoad();
		App::Get()->LoadScene(s_CurrentScene.get());
		return;
	}

	ScriptManager::StartDebug();

	s_Updated = false;
	s_AppThread =
		CreateRef<std::thread>(
		[]()
		{
			App::Get()->Running = true;
			App::Get()->OnLoad();
			App::Get()->LoadScene(s_CurrentScene.get());

			while(true) {
				std::unique_lock<std::mutex> lock(s_Mutex);
				s_Condition.wait(lock, []() { return s_Updated; });
				s_Updated = false;

				if(s_State == EditorMode::Play)
					App::Get()->OnUpdate(s_TimeStep);
				else if(s_State == EditorMode::Pause)
					continue;
				else if(s_State == EditorMode::Edit)
					break;
			}

			App::Get()->OnClose();
			App::Get()->Running = false;

			asThreadCleanup();
		});

	s_AppThread->detach();
}

void Editor::OnPause() {
	Log::Info("OnPause");
	s_EditorMode = EditorMode::Pause;
	if(!s_Debugging)
		App::Get()->Running = false;
	else {
		std::lock_guard<std::mutex> lock(s_Mutex);
		s_State = s_EditorMode;
		s_Updated = true;
		s_Condition.notify_one();
	}
}

void Editor::OnResume() {
	Log::Info("OnResume");
	s_EditorMode = EditorMode::Play;
	if(!s_Debugging)
		App::Get()->Running = true;
	else {
		std::lock_guard<std::mutex> lock(s_Mutex);
		s_State = s_EditorMode;
		s_Updated = true;
		s_Condition.notify_one();
	}
}

void Editor::OnStop() {
	if(s_EditorMode == EditorMode::Edit)
		return;

	Log::Info("OnStop");
	s_EditorMode = EditorMode::Edit;
	if(s_Debugging) {
		{
			std::lock_guard<std::mutex> lock(s_Mutex);
			s_State = s_EditorMode;
			s_Updated = true;
			s_Condition.notify_one();
		}

		s_AppThread.reset();
		ScriptManager::EndDebug();
		s_Debugging = false;
	}
	else {
		App::Get()->OnClose();
		App::Get()->Running = false;
	}
}

}
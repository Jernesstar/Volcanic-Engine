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
#include <Engine/Scene/SceneRenderer.h>

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

static Ref<EditorAssetManager> s_AssetManager;
static Ref<EditorSceneRenderer> s_EditorSceneRenderer;
static Ref<RenderPass> s_OutputPass;

enum class TabMode { None, World3D, World2D, Canvas };
enum class EditorMode { Edit, Preview, Play, Pause };

static TabMode s_TabMode = TabMode::None;
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
	ImGui::DockBuilderDockWindow("Content Browser",  s_DockBottom);
	ImGui::DockBuilderDockWindow("Asset Editor",     s_DockBottom);
	ImGui::DockBuilderDockWindow("Console",          s_DockBottom);

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
		if(ImGui::MenuItem("Run", "Ctrl+R")) Editor::OnPlay();
		ImGui::Separator();
		if(ImGui::MenuItem("Export"))   { }
		if(ImGui::MenuItem("Export To")){ }
		ImGui::EndMenu();
	}

	if(ImGui::BeginMenu("Scene")) {
		if(ImGui::MenuItem("New Scene")) { }
		if(ImGui::MenuItem("Open Scene")) { }
		if(ImGui::MenuItem("Save Scene")) { }
		ImGui::EndMenu();
	}

	// Play / pause / stop toolbar in the menu bar
	ImGui::Separator();
	if(s_EditorMode == EditorMode::Edit || s_EditorMode == EditorMode::Pause) {
		if(ImGui::MenuItem("▶ Play"))  Editor::OnPlay();
	}
	if(s_EditorMode == EditorMode::Play) {
		if(ImGui::MenuItem("⏸ Pause")) Editor::OnPause();
	}
	if(s_EditorMode != EditorMode::Edit) {
		if(ImGui::MenuItem("⏹ Stop"))  Editor::OnStop();
	}

	ImGui::EndMainMenuBar();
}

void Editor::Init(const CommandLineArgs& args) {
	Log::Init();

	Renderer::Init();

	UIRenderer::Init();

	// float fontSize = 15.0f;
	// ImGuiIO& io = ImGui::GetIO();
	// io.Fonts->AddFontFromFileTTF(
	// 	"Editor/assets/fonts/JetBrainsMono-Bold.ttf", fontSize);
	// io.FontDefault =
	// 	io.Fonts->AddFontFromFileTTF(
	// 		"Editor/assets/fonts/JetBrainsMono-Regular.ttf", fontSize);
	// io.IniFilename = nullptr;

	ScriptEngine::Init();
	ScriptGlue::RegisterInterface();

	s_AssetManager = CreateRef<EditorAssetManager>();
	s_EditorSceneRenderer = CreateRef<EditorSceneRenderer>();

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

	auto window = Application::GetWindow();
	auto output =
		RendererAPI::Get()->CreateFramebuffer({
			.Attachments = {
				{ AttachmentTarget::Color, 1920, 1080 }
			},
			.EnableRead = true
		});

	auto libPath = Application::GetLibraryDir();
	s_OutputPass =
		RenderPass::Create("Output",
			AssetImporter::GetShader({
				libPath + "/Editor/assets/Shaders/Framebuffer.glsl.vert",
				libPath + "/Editor/assets/Shaders/Framebuffer.glsl.frag"
			}), output);
	s_OutputPass->SetData(Renderer2D::GetScreenBuffer());

	s_App->ChangeScreen = false;
	s_App->RenderScene = false;
	s_App->Running = false;
	s_App->SetOutputPass(s_OutputPass);

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

	UIRenderer::Close();
	Renderer::Close();
	ScriptEngine::Shutdown();
}

void Editor::Update(TimeStep ts) {
	Renderer::BeginFrame();
	UIRenderer::BeginFrame();
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

	s_CurrentScene->OnUpdate(ts);
	if(s_EditorMode == EditorMode::Edit)
		s_EditorSceneRenderer->Update(ts);
	else if(s_EditorMode == EditorMode::Preview)
		s_App->GetSceneRenderer()->Update(ts);

	s_Hierarchy.Update(ts);
	s_Visualizer.Update(ts);
	s_ComponentEditor.Update(ts);
	s_ContentBrowser.Update(ts);
	s_AssetEditor.Update(ts);
	s_Console.Update(ts);
}

void Editor::Render() {
	// ── Full-screen dockspace window ──────────────────────────────────────
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

	// ── Menu bar (drawn on top of dockspace) ──────────────────────────────
	DrawMainMenuBar();

	// ── Panels ────────────────────────────────────────────────────────────
	s_Hierarchy.Draw();
	s_Visualizer.Draw();
	s_ComponentEditor.Draw();
	s_ContentBrowser.Draw();
	s_AssetEditor.Draw();
	s_Console.Draw();

	UIRenderer::EndFrame();
	Renderer::EndFrame();
}

Project& Editor::GetProject() {
	return s_Project;
}

void Editor::OpenProject(const Str& path) {
	Application::PushDir(path);
	s_AssetManager->LoadRegistry();
	// if(!s_App->GetSceneRenderer())
	// 	s_App->CreateSceneRenderer();
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
	s_TabMode = TabMode::World3D;
}

void Editor::SaveScene(const Str& name) {

}

void Editor::OnPlay(bool debug) {
	if(s_EditorMode == EditorMode::Edit) {
		Str screen = s_CurrentScene->Screen;
		s_EditorMode = EditorMode::Play;
		// SaveScene();

		App::Get()->PrepareScreen();

		s_Debugging = debug;
		if(s_Debugging) {
			ScriptManager::StartDebug();

			s_Updated = false;
			s_AppThread = CreateRef<std::thread>(
				[screen]()
				{
					App::Get()->Running = true;
					App::Get()->OnLoad();
					App::Get()->LoadScene(s_CurrentScene.get());
					App::Get()->ScreenSet(screen);

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
		else {
			App::Get()->Running = true;
			App::Get()->OnLoad();
			App::Get()->LoadScene(s_CurrentScene.get());
			App::Get()->ScreenSet(screen);
		}
	}
}

void Editor::OnPause() {
	s_EditorMode = EditorMode::Pause;
	if(s_Debugging) {
		std::lock_guard<std::mutex> lock(s_Mutex);
		s_State = s_EditorMode;
		s_Updated = true;
		s_Condition.notify_one();
	}
	else
		App::Get()->Running = false;
}

void Editor::OnResume() {
	s_EditorMode = EditorMode::Play;
	if(s_Debugging) {
		std::lock_guard<std::mutex> lock(s_Mutex);
		s_State = s_EditorMode;
		s_Updated = true;
		s_Condition.notify_one();
	}
	else
		App::Get()->Running = true;
}

void Editor::OnStop() {
	if(s_EditorMode == EditorMode::Edit)
		return;

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

	// Ref<Tab> current = Editor::GetCurrentTab();
	// if(current->Type == TabMode::Scene) {
	// 	auto tab = current->As<SceneTab>();
	// 	tab->Reset();
	// }
}

}
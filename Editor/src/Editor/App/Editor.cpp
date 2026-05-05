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

void Editor::Init(const CommandLineArgs& args) {
	Log::Init();

	Renderer::Init();

	UIRenderer::Init();

	float fontSize = 15.0f;
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF(
		"Editor/assets/fonts/JetBrainsMono-Bold.ttf", fontSize);
	io.FontDefault =
		io.Fonts->AddFontFromFileTTF(
			"Editor/assets/fonts/JetBrainsMono-Regular.ttf", fontSize);
	io.IniFilename = nullptr;

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
		// OnPlay();
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

	Renderer::BeginFrame();

	if(s_EditorMode == EditorMode::Edit) {
		s_EditorSceneRenderer->Update(ts);
		s_CurrentScene->OnUpdate(ts);
	}
	else if(s_EditorMode == EditorMode::Preview) {
		s_App->GetSceneRenderer()->Update(ts);
		s_CurrentScene->OnUpdate(ts);
	}
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

		ImGui::BeginMainMenuBar();
		{
			if(ImGui::BeginMenu("Project")) {
				if(ImGui::MenuItem("New", "Ctrl+N"))
					menu.project.newProject = true;
				if(ImGui::MenuItem("Open", "Ctrl+P"))
					menu.project.openProject = true;
				if(ImGui::MenuItem("Run", "Ctrl+R"))
					menu.project.runProject = true;

				ImGui::Separator();
				if(ImGui::MenuItem("Export"))
					menu.project.exportProject = true;
				if(ImGui::MenuItem("Export To"))
					menu.project.exportProjectTo = true;

				ImGui::Separator();
				if(ImGui::MenuItem("Add Screen"))
					menu.project.addScreen = true;

				ImGui::EndMenu();
			}
		}
		ImGui::EndMainMenuBar();

		ImGuiID dockspaceID = ImGui::GetID("DockSpace");
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
	}
	ImGui::End();

	if(s_EditorMode == EditorMode::Play)
		return;

	Ref<SceneRenderer> renderer;
	if(s_CurrentScene) {
		if(s_EditorMode == EditorMode::Edit) {
			renderer = s_EditorSceneRenderer;
			s_CurrentScene->OnRender(*renderer);
		}
		else if(s_EditorMode == EditorMode::Preview) {
			renderer = s_App->GetSceneRenderer();
		}
	}

	Renderer::StartPass(s_OutputPass);
	{
		Renderer2D::DrawFullscreenQuad(renderer->GetOutput());
	}
	Renderer::EndPass();

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

void Editor::NewCanvas(const Str& name) {

}

void Editor::OpenCanvas(const Str& name) {

}

void Editor::SaveCanvas(const Str& name) {

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

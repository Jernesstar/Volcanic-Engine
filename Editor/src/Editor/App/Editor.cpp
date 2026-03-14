#include "Editor.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>
#include <iterator>

#define RAPIDJSON_ASSERT(x) ((void)0)
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Window/Events.h>

#include <Engine/App/App.h>
#include <Engine/Graphics/Renderer.h>
#include <Engine/Graphics/Renderer2D.h>
#include <Engine/Script/ScriptGlue.h>
#include <Engine/Scene/Scene.h>
#include <Engine/Scene/SceneRenderer.h>
#include <Engine/Canvas/Canvas.h>

#include "./SceneRenderer.h"
#include "../Asset/AssetManager.h"
#include "../Asset/AssetImporter.h"

#include "SceneLoader.h"
#include "ScriptManager.h"
#include "Embed.h"

using namespace VolcaniCore;
using namespace VolcanicEngine;
using namespace VolcanicEngine::Graphics;
using namespace VolcanicEngine::Script;

namespace VolcanicEditor {

static Project s_Project;
static Ref<App> s_App;
static Ref<Project> s_CurrentProject;
static Ref<Scene> s_CurrentScene;
static Ref<Canvas> s_CurrentCanvas;

static Ref<EditorAssetManager> s_AssetManager;
static Ref<EditorSceneRenderer> s_EditorSceneRenderer;
static Ref<RenderPass> s_OutputPass;

enum class TabType { None, Scene, Canvas };
enum class EditorMode { Edit, Preview, Play, Pause };

static TabType s_TabType = TabType::None;
static EditorMode s_EditorMode = EditorMode::Edit;

static Ref<std::thread> s_AppThread;
static std::mutex s_Mutex;
static std::condition_variable s_Condition;
static EditorMode s_State = EditorMode::Edit;
static bool s_Updated = false;
static bool s_Debugging = false;

static TimeStep s_TimeStep;

void Editor::Init(const CommandLineArgs& args) {
	// Log::Init(args.Has("--embedded"));
	Log::Init();

	if(args.Has("--embedded")) {
		Embed::OnEvent =
			[](const Str& str)
			{
				rapidjson::Document document;
				rapidjson::ParseResult ok = document.Parse(str.c_str());
				if (!ok) {
					Log::Error("Parsing error for input data: {}", str);
					return;
				}

				Str type = document["type"].Get<Str>();
				if(type == "mouse_move") {
					f64 x = document["x"].Get<f64>();
					f64 y = document["y"].Get<f64>();
					MouseMovedEvent event(x, y);
					Events::Dispatch(event);
				}
				else if(type == "mouse_click") {
					f64 x = document["x"].Get<f64>();
					f64 y = document["y"].Get<f64>();
					u32 button = document["button"].Get<u32>();
					MouseButtonPressedEvent event((MouseCode)button, x, y);
					Events::Dispatch(event);
				}
				else if(type == "mouse_up") {
					f64 x = document["x"].Get<f64>();
					f64 y = document["y"].Get<f64>();
					u32 button = document["button"].Get<u32>();
					MouseButtonPressedEvent event((MouseCode)button, x, y);
					Events::Dispatch(event);
				}
				else if(type == "mouse_down") {
					f64 x = document["x"].Get<f64>();
					f64 y = document["y"].Get<f64>();
					u32 button = document["button"].Get<u32>();
					MouseButtonReleasedEvent event((MouseCode)button, x, y);
					Events::Dispatch(event);
				}
				else if(type == "mouse_scroll") {
					f64 dx = document["dx"].Get<f64>();
					f64 dy = document["dy"].Get<f64>();
					MouseMovedEvent event(dx, dy);
					Events::Dispatch(event);
				}
			};

		Embed::Init();
	}

	Renderer::Init();
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
	s_App->ScreenLoad =
		[](Ref<ScriptModule>& script, const std::string& name)
		{
			script = AssetImporter::GetScript("App/Screen/" + name + ".as");
		};
	s_App->SceneLoad =
		[](Scene& scene)
		{
			Str scenePath = "App/Scene/" + scene.Name + ".scene";
			SceneLoader::EditorLoad(scene, scenePath);
		};
	s_App->CanvasLoad =
		[](Canvas& canvas)
		{
			Str canvasPath = "App/Scene/" + canvas.Name + ".canvas";
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
	s_App->RenderCanvas = false;
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
	if(Embed::IsActive())
		Embed::Close();

	OnStop();

	s_CurrentScene.reset();
	s_CurrentCanvas.reset();

	s_App.reset();
	s_AssetManager.reset();

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
		if(s_TabType == TabType::Scene) {
			s_EditorSceneRenderer->Update(ts);
			s_CurrentScene->OnUpdate(ts);
		}
	}
	else if(s_EditorMode == EditorMode::Preview) {
		if(s_TabType == TabType::Scene) {
			s_App->GetSceneRenderer()->Update(ts);
			s_CurrentScene->OnUpdate(ts);
		}
	}
}

void Editor::Render() {
	if(s_EditorMode == EditorMode::Play)
		return;

	Ref<SceneRenderer> renderer;
	if(s_TabType == TabType::Scene && s_CurrentScene) {
		if(s_EditorMode == EditorMode::Edit) {
			renderer = s_EditorSceneRenderer;
			s_CurrentScene->OnRender(*renderer);
		}
		else if(s_EditorMode == EditorMode::Preview) {
			renderer = s_App->GetSceneRenderer();
		}
	}

	// Ref<CanvasRenderer> renderer2;
	if(s_TabType == TabType::Canvas && s_CurrentCanvas) {
		if(s_EditorMode == EditorMode::Edit) {
			// renderer = s_EditorCanvasRenderer;
			// s_CurrentCavas->OnRender(*renderer);
		}
		else if(s_EditorMode == EditorMode::Preview) {
			// renderer2 = s_App->GetCanvasRenderer();
		}
	}

	Renderer::StartPass(s_OutputPass);
	{
		Renderer2D::DrawFullscreenQuad(renderer->GetOutput());
		// if(renderer2)
		// 	Renderer2D::DrawFullscreenQuad(renderer2->GetOutput());
	}
	Renderer::EndPass();

	if(!Embed::IsActive()) {
		Renderer2D::DrawFullscreenQuad(s_OutputPass->GetOutput());
	}
	Renderer::EndFrame();

	if(Embed::IsActive()) {
		Buffer<u8> data = s_OutputPass->GetOutput()->GetPixels();
		Embed::SendFrame(std::move(data));
	}
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
	s_TabType = TabType::Scene;
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
	// if(current->Type == TabType::Scene) {
	// 	auto tab = current->As<SceneTab>();
	// 	tab->Reset();
	// }
}

}

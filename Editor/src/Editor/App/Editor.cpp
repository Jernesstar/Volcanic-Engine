#include "Editor.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>
#include <iterator>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Window/Events.h>

#include <Engine/App/App.h>
#include <Engine/Graphics/Renderer.h>
#include <Engine/Script/ScriptGlue.h>
#include <Engine/Scene/Scene.h>
#include <Engine/Scene/SceneRenderer.h>
#include <Engine/Canvas/Canvas.h>

#include "./SceneRenderer.h"
#include "../Asset/AssetManager.h"

#include "SceneLoader.h"

using namespace VolcaniCore;
using namespace VolcanicEngine;
using namespace VolcanicEngine::Graphics;
using namespace VolcanicEngine::Script;

namespace VolcanicEditor {

static Ref<App> s_App;
static Ref<Project> s_CurrentProject;
static Ref<Scene> s_CurrentScene;
static Ref<Canvas> s_CurrentCanvas;

static Ref<EditorAssetManager> s_AssetManager;

static Ref<EditorSceneRenderer> s_EditorSceneRenderer;

enum class TabType { None, Scene, Canvas };
enum class EditorMode { Edit, Play, Pause };

static TabType s_TabType = TabType::None;
static EditorMode s_EditorMode = EditorMode::Edit;

void Editor::Init(const CommandLineArgs& args) {
	if(args["--embed_window"]) {
		Log::Init(true);
		Str handle = args["--embed_window"];
		EmbedWindow(handle.c_str());
	}
	else
		Log::Init(false);

	Renderer::Init();
	ScriptEngine::Init();
	ScriptGlue::RegisterInterface();

	s_AssetManager = CreateRef<EditorAssetManager>();
	s_EditorSceneRenderer = CreateRef<EditorSceneRenderer>();

	s_App = CreateRef<App>();

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

	// s_App->CreateSceneRenderer();
}

void Editor::Close() {
	s_CurrentScene.reset();
	s_CurrentCanvas.reset();

	s_App.reset();
	s_AssetManager.reset();

	Renderer::Close();
	ScriptEngine::Shutdown();
}

void Editor::Update(TimeStep ts) {
	if(s_TabType == TabType::Scene)
		if(s_EditorMode == EditorMode::Edit) {
			s_EditorSceneRenderer->Update(ts);
			s_CurrentScene->OnUpdate(ts);
		}
}

void Editor::Render() {
	Renderer::BeginFrame();

	if(s_TabType == TabType::Scene) {
		if(s_EditorMode == EditorMode::Edit)
			s_CurrentScene->OnRender(*s_EditorSceneRenderer);
		else if(s_EditorMode == EditorMode::Play)
			s_CurrentScene->OnRender(*s_App->GetSceneRenderer());
	}

	Renderer::EndFrame();
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

}
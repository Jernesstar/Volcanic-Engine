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
#include <Engine/Scene/Scene.h>
#include <Engine/Scene/SceneRenderer.h>
#include <Engine/Canvas/Canvas.h>

#include "./SceneRenderer.h"
#include "../Asset/AssetManager.h"

#include "SceneLoader.h"

using namespace VolcaniCore;
using namespace VolcanicEngine;
using namespace VolcanicEngine::Graphics;

namespace VolcanicEditor {

std::mutex s_IOMutex;
void StandardIO() {
	std::string line;
	while(std::getline(std::cin, line)) {
		if(line.empty())
			continue;

		std::stringstream ss(line);

		// 2. Tokenize by space into a vector (list)
		// istream_iterator automatically skips any whitespace
		std::vector<std::string> tokens{
			std::istream_iterator<std::string>{ss},
			std::istream_iterator<std::string>{}
		};

		char** args = new char*[tokens.size()];
		for(u32 i = 0; i < tokens.size(); i++) {
			args[i] = new char[tokens[i].size() + 1];
			strcpy(args[i], tokens[i].c_str());
		}

		CommandLineArgs arg(tokens.size(), args, false);
		if(arg["--embed_window"]) {
			std::string handle = arg["--embed_window"];
			EmbedWindow(handle.c_str());
		}

		{
			std::lock_guard<std::mutex> lock(s_IOMutex);
		}
	}
}

static Ref<App> s_App;
static Ref<Project> s_CurrentProject;
static Ref<Scene> s_CurrentScene;
static Ref<Canvas> s_CurrentCanvas;

static Ref<EditorAssetManager> s_AssetManager;

static Ref<EditorSceneRenderer> s_EditorSceneRenderer;
static Ref<RuntimeSceneRenderer> s_RuntimeSceneRenderer;

enum class TabType { None, Scene, Canvas };

static TabType s_TabType = TabType::None;

void Editor::Init(const CommandLineArgs& args) {
	Log::Init();
	Renderer::Init();
	// std::thread(StandardIO).detach();

	s_AssetManager = CreateRef<EditorAssetManager>();
	s_EditorSceneRenderer = CreateRef<EditorSceneRenderer>();
	// s_RuntimeSceneRenderer = CreateRef<RuntimeSceneRenderer>();

	if(args["--open_project"]) {
		std::string path = args["--open_project"];
		Editor::OpenProject(path);
	}
	if(args["--new_project"]) {
		std::string path = args["--new_project"];
		Editor::NewProject(path);
	}
	if(args["--open_scene"]) {
		std::string name = args["--open_scene"];
		Editor::OpenScene(name);
		Log::Info("Loaded");
	}
}

void Editor::Close() {

}

void Editor::Update(TimeStep ts) {
	if(s_TabType == TabType::Scene)
		s_CurrentScene->OnUpdate(ts);
}

void Editor::Render() {
	if(s_TabType == TabType::Scene)
		s_CurrentScene->OnRender(*s_EditorSceneRenderer);
}

void Editor::OpenProject(const std::string& path) {
	Application::PushDir(path);
}

void Editor::NewProject(const std::string& path) {

}

void Editor::SaveProject() {

}

void Editor::NewScene(const std::string& path) {

}

void Editor::OpenScene(const std::string& name) {
	s_CurrentScene = CreateRef<Scene>(name);
	SceneLoader::EditorLoad(*s_CurrentScene, "Object/Scene/" + name + ".scene");
	Log::Info("Loaded scene {}", name);
	// s_TabType = TabType::Scene;
}

void Editor::SaveScene(const std::string& name) {

}

void Editor::NewCanvas(const std::string& name) {

}

void Editor::OpenCanvas(const std::string& name) {

}

void Editor::SaveCanvas(const std::string& name) {

}

}
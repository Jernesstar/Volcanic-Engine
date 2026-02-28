#include "Editor.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>
#include <iterator>

#include <VolcaniCore/Window/Events.h>
#include <Engine/Graphics/Renderer.h>

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
		{
			std::lock_guard<std::mutex> lock(s_IOMutex);
			if(arg["--embed_window"]) {
				std::string handle = arg["--embed_window"];
				EmbedWindow(handle.c_str());
			}
			if(arg["--open_project"]) {
				std::string path = arg["--open_project"];
				Editor::OpenProject(path);
			}
			if(arg["--new_project"]) {
				std::string path = arg["--open_project"];
				Editor::NewProject(path);
			}
			if(arg["--open-scene"]) {
				std::string name = arg["--open-scene"];
				Editor::OpenScene(name);
			}
		}
	}
}

static Ref<Project> s_CurrentProject;
static Ref<Scene> s_CurrentScene;
static Ref<Canvas> s_CurrentCanvas;

void Editor::Init(const CommandLineArgs& args) {
	Log::Init();
	Renderer::Init();
	std::thread(StandardIO).detach();
}

void Editor::Close() {

}

void Editor::Update(TimeStep ts) {

}

void Editor::Render() {
	
}

void Editor::OpenProject(const std::string& path) {

}

void Editor::NewProject(const std::string& path) {

}

void Editor::SaveProject() {

}

void Editor::NewScene(const std::string& path) {

}

void Editor::OpenScene(const std::string& name) {

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
#include "App.h"

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

		{
			std::lock_guard<std::mutex> lock(s_IOMutex);
		}
	}
}

EditorApp::EditorApp(const CommandLineArgs& args)
	: Application({ "Editor" })
{
	if(args["--embed_window"]) {
		std::string handle = args["--embed_window"];
		EmbedWindow(handle.c_str());
	}

	Log::Init();
	Renderer::Init();
	std::thread(StandardIO).detach();
}

EditorApp::~EditorApp() {

}

void EditorApp::OnUpdate(TimeStep ts) {
	Renderer::Clear();
}

}
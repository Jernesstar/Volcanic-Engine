#include "App.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>
#include <iterator>

#include <Engine/Graphics/Renderer.h>

using namespace VolcanicEngine::Graphics;

namespace VolcanicEditor {

std::mutex s_IOMutex;

void StandardIO() {
	std::string line;
	while (std::getline(std::cin, line)) {
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
		}
	}
}

EditorApp::EditorApp(const CommandLineArgs& args)
	: Application({ "Editor" })
{
	Renderer::Init();

	std::thread(StandardIO).detach();
}

EditorApp::~EditorApp() {

}

void EditorApp::OnUpdate(TimeStep ts) {
	Renderer::Clear();
}

}
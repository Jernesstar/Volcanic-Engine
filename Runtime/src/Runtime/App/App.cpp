#include "App.h"

namespace fs = std::filesystem;

namespace VolcaniCore { extern std::string FindExecutablePath(); }

namespace VolcanicRuntime {

RuntimeApp::RuntimeApp(const CommandLineArgs& args)
	: Application({ "Runtime" })
{
	std::string rootPath;
	if(args["--project"]) {
		auto volcPath = args["--project"].Args[0];
		rootPath = fs::path(volcPath).parent_path().string();
	}
	else
		rootPath = fs::path(FindExecutablePath()).parent_path().string();

	Application::PushDir(rootPath);

}

RuntimeApp::~RuntimeApp() {

}

void RuntimeApp::OnUpdate(TimeStep ts) {

}

}
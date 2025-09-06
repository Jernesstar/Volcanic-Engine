#include <cstdlib>

#include "Application.h"
#include "Assert.h"

#include "Event/Events.h"

namespace fs = std::filesystem;

namespace VolcaniCore {

void Application::Init() {
	VOLCANICORE_ASSERT(glfwInit(), "Failed to initialize GLFW");
}

void Application::Close() {
	delete s_Instance;

	s_Window.reset();
	glfwTerminate();
	exit(0);
}

Application::Application(const WindowSpecification& spec) {
	s_Instance = this;
	s_Window = CreateRef<Window>(spec);
	Events::Init();
}

void Application::Run() {
	while(s_Window->IsOpen()) {
		TimePoint time = Time::GetTime();
		TimeStep ts = time - s_LastFrame;
		s_LastFrame = time;

		Events::PollEvents();

		s_Instance->OnUpdate(ts);

		s_Window->Update();
	}
}

static std::string s_OldPath;

std::string Application::GetHomeDir() {
#ifdef VOLCANICENGINE_WINDOWS
	return getenv("USERPROFILE");
#elif VOLCANICENGINE_LINUX
	return getenv("HOME");
#endif
}

std::string Application::GetCurrentDir() {
	return s_Path;
}

void Application::PushDir() {
	PushDir(s_LibraryPath);
}

void Application::PushDir(const std::string& path) {
	if(path == "")
		return;

	s_OldPath = s_Path;
	s_Path = path;
	fs::current_path(s_Path);
}

void Application::PopDir() {
	s_Path = s_OldPath;
	fs::current_path(s_Path);
}

void Application::SetCurrentDir() {
	s_Path = fs::current_path().string();
	s_OldPath = s_Path;

	char* env = getenv("VOLC_PATH");
	if(!env) {
		VOLCANICORE_LOG_WARNING("Cound not find VOLC_PATH env variable");
		return;
	}

	s_LibraryPath = env;
}

}

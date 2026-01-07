#include <cstdlib>
#include <chrono>
#include <thread>

#include "Application.h"
#include "Assert.h"

namespace fs = std::filesystem;

namespace VolcaniCore {

void Application::Close(u32 code) {
	delete s_Instance;
	exit(code);
}

Application* Application::Get() {
	return s_Instance;
}

Application::Application(const AppSpecification& spec) {
	s_Instance = this;
	s_Spec = spec;
}

void Application::Run() {
	while(true) {
		TimePoint time = Time::GetTime();
		TimeStep ts = time - m_LastFrame;
		m_LastFrame = time;

		s_Instance->OnUpdate(ts);

		if(!s_Spec.TickRate)
			continue;

		f32 targetDelta = (1.0f / float(s_Spec.TickRate)) * 1000.0f;
		if (ts < targetDelta) {
			f32 sleep = targetDelta - ts;
			auto timeMS = std::chrono::milliseconds(static_cast<u32>(sleep));
			std::this_thread::sleep_for(timeMS);
		}
	}
}

static std::string s_OldPath;

std::string Application::GetHomeDir() {
#ifdef VOLCANIC_WINDOWS
	return getenv("USERPROFILE");
#elif VOLCANIC_LINUX
	return getenv("HOME");
#endif
}

std::string Application::GetCurrentDir() {
	return s_Path;
}

std::string Application::GetLibraryDir() {
	return s_LibraryPath;
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
		Log::Warning("Cound not find VOLC_PATH env variable");
		return;
	}

	s_LibraryPath = env;
}

}

#include "Runtime.h"

#include <Lava/Core/Lava.h>

#include "VolcanicWindow/Events.h"

namespace fs = std::filesystem;

using namespace VolcanicWindow;

namespace Lava {

extern std::string FindExecutablePath();

Runtime::Runtime(const CommandLineArgs& args)
	: WindowApplication({ "Runtime", 1920, 1080, false, true })
{
	Events::RegisterListener<KeyPressedEvent>(
		[](const KeyPressedEvent& event)
		{
			if(event.Key == Key::Escape)
				Application::Close();
		});

	std::string rootPath;
	if(args["--project"]) {
		auto volcPath = args["--project"].Args[0];
		rootPath = fs::path(volcPath).parent_path().string();
	}
	else
		rootPath = fs::path(FindExecutablePath()).parent_path().string();

	Application::PushDir(rootPath);

	Lava::InitComponents();

	Project project;
	// project.Load("./.volc.proj");

	m_App->Running = true;
	m_App->OnLoad();
}

Runtime::~Runtime() {
	m_App->OnClose();
	m_App.reset();

	Lava::CloseComponents();
}

void Runtime::OnUpdate(TimeStep ts) {
	m_App->OnUpdate(ts);
}

}
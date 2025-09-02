#include "Runtime.h"

#include <VolcaniCore/Event/Events.h>

#include <Magma/Graphics/Renderer.h>
#include <Magma/Graphics/Shader.h>
#include <Magma/Graphics/ShaderLibrary.h>

#include <Magma/Core/BinaryReader.h>
#include <Magma/Script/ScriptEngine.h>
#include <Magma/Audio/AudioEngine.h>
// #include <Magma/Physics/Physics.h>
// #include <Magma/UI/UIRenderer.h>

#include <Lava/Core/ScriptGlue.h>

namespace fs = std::filesystem;

namespace Lava {

extern std::string FindExecutablePath();

Runtime::Runtime(const CommandLineArgs& args)
	: Application(1920, 1080)
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

	// Ash::Init();
	// Cinder::Init();
	// Igneous::Init();
	// Pyro::Init();
	// Silica::Init();

	Project project;
	// project.Load("./.volc.proj");

	m_App->Running = true;
	m_App->OnLoad();
}

Runtime::~Runtime() {
	m_App->OnClose();
	m_App.reset();

	// Silica::Close();
	// Pyro::Close();
	// Igneous::Close();
	// Cinder::Close();
	// Ash::Close();
}

void Runtime::OnUpdate(TimeStep ts) {
	m_App->OnUpdate(ts);
}

}
#include "EditorApp.h"

#include <VolcaniCore/Core/Log.h>
#include <VolcaniCore/Event/Events.h>

#include <Lava/Core/Lava.h>

using namespace VolcaniCore;

namespace Magma {

EditorApp::EditorApp(const CommandLineArgs& args)
	: Application({ "Magma Editor v0.1.0", 1400, 800, true })
{
	Events::RegisterListener<KeyPressedEvent>(
		[](const KeyPressedEvent& event)
		{
			if(event.Key == Key::Escape)
				Application::Close();
		});

	Lava::InitComponents();

	m_Editor.Open();
	m_Editor.Load(args);
}

EditorApp::~EditorApp() {
	m_Editor.Close();

	Lava::CloseComponents();
}

void EditorApp::OnUpdate(TimeStep ts) {
	Lava::BeginFrame();
	Lava::Update(ts);

	m_Editor.Update(ts);
	m_Editor.Render();

	Lava::EndFrame();
}

}
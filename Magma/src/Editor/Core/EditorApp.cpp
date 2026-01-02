#include "EditorApp.h"

#include <VolcaniCore/Core/Log.h>
#include <VolcanicWindow/Application.h>
#include <VolcanicWindow/Events.h>

using namespace VolcaniCore;
using namespace VolcanicWindow;

namespace Magma {

EditorApp::EditorApp(const CommandLineArgs& args)
	: WindowApplication({
		.Title = "Magma Editor v0.1.0",
		.Width = 1400,
		.Height = 800,
		.TickRate = 60,
		.VSync = true,
		.Undecorated = true
	})
{
	Events::RegisterListener<KeyPressedEvent>(
		[](const KeyPressedEvent& event)
		{
			if(event.Key == Key::Escape)
				Application::Close();
		});

	m_Editor.Open();
	m_Editor.Load(args);
}

EditorApp::~EditorApp() {
	m_Editor.Close();
}

void EditorApp::OnUpdate(TimeStep ts) {
	m_Editor.Update(ts);
	m_Editor.Render();
}

}
#include "EditorApp.h"

#include <VolcaniCore/Core/Log.h>
#include <VolcanicWindow/Application.h>
#include <VolcanicWindow/Events.h>

#include <Lava/Core/Lava.h>

#include "UI/Widget.h"
#include "Networking/Networking.h"
#include "Asset/AssetImporter.h"
#include "Utils/YAMLSerializer.h"

using namespace VolcaniCore;
using namespace VolcanicWindow;

namespace Magma {

EditorApp::EditorApp(const CommandLineArgs& args)
	: WindowApplication({ "Magma Editor v0.1.0", 1400, 800, true })
{
	Events::RegisterListener<KeyPressedEvent>(
		[](const KeyPressedEvent& event)
		{
			if(event.Key == Key::Escape)
				Application::Close();
		});

	Lava::InitComponents();

	UI::UIManager::Init();

	m_Editor.Open();
	m_Editor.Load(args);
}

EditorApp::~EditorApp() {
	m_Editor.Close();

	Lava::CloseComponents();
}

void EditorApp::OnUpdate(TimeStep ts) {
	UI::UIManager::Update(ts);
	
	Lava::BeginFrame();
	Lava::Update(ts);
	
	m_Editor.Update(ts);
	m_Editor.Render();

	Lava::EndFrame();

	UI::UIManager::Render();
}

}
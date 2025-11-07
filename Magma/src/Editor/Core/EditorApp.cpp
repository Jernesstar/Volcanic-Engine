#include "EditorApp.h"

#include <VolcaniCore/Core/Log.h>
#include <VolcanicWindow/Application.h>
#include <VolcanicWindow/Events.h>

#include <Lava/Core/Lava.h>

// #include "Asset/Asset.h"
#include "Graphics/Renderer.h"
#include "Networking/Networking.h"
#include "UI/Widget.h"

using namespace VolcaniCore;
using namespace VolcanicWindow;

// using namespace Magma::Asset;
using namespace Magma::UI;
using namespace Magma::Graphics;
using namespace Magma::Networking;

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

	// AssetManager::Init();
	Renderer::Init();
	UIManager::Init();

	m_Editor.Open();
	m_Editor.Load(args);
}

EditorApp::~EditorApp() {
	m_Editor.Close();

	UIManager::Close();
	Renderer::Close();
	// AssetManager::Close();

	Lava::CloseComponents();
}

void EditorApp::OnUpdate(TimeStep ts) {
	Lava::BeginFrame();
	Lava::Update(ts);

	Renderer::StartFrame();

	UIManager::Update(ts);
	m_Editor.Update(ts);
	m_Editor.Render();
	UIManager::Render();

	Renderer::EndFrame();

	Lava::EndFrame();
}

}
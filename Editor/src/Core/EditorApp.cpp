#include "EditorApp.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <ImGuizmo/ImGuizmo.h>

#include <VolcaniCore/Core/Log.h>
#include <VolcaniCore/Event/Events.h>

#include <Lava/Core/Lava.h>

#include "Core/AssetImporter.h"
#include "UI/Widget.h"

using namespace VolcaniCore;
using namespace Magma::UI;

namespace Magma {

EditorApp::EditorApp(const CommandLineArgs& args)
	: Application({ "Magma Editor", 1400, 800, true })
{
	Events::RegisterListener<KeyPressedEvent>(
		[](const KeyPressedEvent& event)
		{
			if(event.Key == Key::Escape)
				Application::Close();
		});

	Lava::InitComponents();

	WidgetManager::Init();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;

	m_Editor.Open();
	m_Editor.Load(args);
}

EditorApp::~EditorApp() {
	m_Editor.Close();

	WidgetManager::Close();

	Lava::CloseComponents();
}

void EditorApp::OnUpdate(TimeStep ts) {
	WidgetManager::BeginFrame();
	ImGuizmo::BeginFrame();

	Lava::BeginFrame();
	Lava::Update(ts);

	m_Editor.Update(ts);
	m_Editor.Render();

	Lava::EndFrame();

	WidgetManager::EndFrame();
}

}
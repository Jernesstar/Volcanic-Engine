#include "EditorApp.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <ImGuizmo/ImGuizmo.h>

#include <VolcaniCore/Core/Log.h>
#include <VolcaniCore/Event/Events.h>

#include <Lava/Core/Lava.h>

#include "Editor/AssetImporter.h"
#include "UI/UIRenderer.h"

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

	UIRenderer::Init();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;

	m_Editor.Open();
	m_Editor.Load(args);
}

EditorApp::~EditorApp() {
	m_Editor.Close();

	UIRenderer::Close();

	Lava::CloseComponents();
}

void EditorApp::OnUpdate(TimeStep ts) {
	UIRenderer::BeginFrame();
	ImGuizmo::BeginFrame();

	Lava::BeginFrame();
	Lava::Update(ts);

	m_Editor.Update(ts);
	m_Editor.Render();

	UIRenderer::EndFrame();

	Lava::EndFrame();
}

}
#include "Editor.h"

#include <fstream>
#include <iostream>
#include <regex>

#include <VolcaniCore/Core/Application.h>
// #include <VolcaniCore/Core/Algo.h>
// #include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Core/Log.h>

#include <VolcanicWindow/Application.h>
#include <VolcanicWindow/Events.h>
// #include <VolcanicWindow/Input.h>

// #include <Lava/Core/Lava.h>

// #include "Integration/AI/AI.h"
// #include "Integration/VersionControl/VersionControl.h"
// #include "Integration/Lang/ScriptManager.h"

#include "Utils/YAMLSerializer.h"
#include "Widget/Widget.h"
#include "Graphics/Renderer.h"
#include "Graphics/Platform/RendererAPI.h"
#include "Networking/Networking.h"
#include "Asset/Asset.h"

#undef LoadImage
#undef LoadImageW
#undef LoadImageA

using namespace VolcaniCore;
using namespace VolcanicWindow;

// using namespace Magma::Asset;
using namespace Magma::UI;
using namespace Magma::Graphics;
using namespace Magma::Networking;

namespace fs = std::filesystem;

namespace Magma {

void Editor::Open() {
	// Editor::RegisterInterface();

	Application::PushDir();
	auto logo =
		AssetImporter::LoadImage(
			"Magma/assets/images/VolcanicDisplay.png", false);

	// auto window = Application::As<WindowApplication>()->GetWindow();
	// window->SetIcon({ logo->Width, logo->Height, logo->Data.Copy() });

	AssetManager::Init();
	Renderer::Init();
	WidgetManager::Init();

	WidgetManager::Load("Magma/assets/UI/test.rml");

	Application::PopDir();

	Events::RegisterListener<KeyPressedEvent>(
		[](const KeyPressedEvent& event)
		{
			if(event.Key == Key::R)
				WidgetManager::Reload();
		});

	// AI::AIManager::Init();
	// AI::AIManager::RunAnalysis();
}

void Editor::Close() {
	WidgetManager::Close();
	Renderer::Close();
	AssetManager::Close();

	// AI::AIManager::Close();
}

void Editor::Load(const CommandLineArgs& args) {
	// if(args["--project"])
	// 	OpenProject(args["--project"]);
	// else if(args["--lavaflow"])
	// 	OpenLavaFlow(args["--lavaflow"]);
	// else if(args["--component"])
	// 	OpenComponent(args["--component"]);
}

void Editor::Update(TimeStep ts) {
	// if(Mode == EditorMode::Project)
	// 	for(auto& tab : m_Tabs)
	// 		tab.OnUpdate(ts);

	WidgetManager::Update(ts);
}

void Editor::Render() {
	Renderer::BeginFrame();

	if(Mode == EditorMode::None)
		RenderStartScreen();
	else if(Mode == EditorMode::Component)
		RenderComponentEditor();
	else if(Mode == EditorMode::Flow)
		RenderFlowEditor();
	else if(Mode == EditorMode::Project)
		RenderProjectEditor();

	WidgetManager::Render();

	Renderer::EndFrame();
}

void Editor::RenderStartScreen() {

}

void Editor::RenderComponentEditor() {

}

void Editor::RenderFlowEditor() {

}

void Editor::RenderProjectEditor() {

}

void Editor::RegisterInterface() {

}

}
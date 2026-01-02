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

#include "Integration/AI/AI.h"
// #include "Integration/VersionControl/VersionControl.h"
// #include "Integration/Lang/ScriptManager.h"

#include "Utils/YAMLSerializer.h"
#include "Widget/Widget.h"
#include "Widget/ASX.h"
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
	AssetManager::Init();
	Renderer::Init();
	WidgetManager::Init();

	Events::RegisterListener<KeyPressedEvent>(
		[](const KeyPressedEvent& event)
		{
			if(event.Key == Key::R)
				WidgetManager::Reload();
		});

	LoadHomeScreen();
}

void Editor::Close() {
	WidgetManager::Close();
	Renderer::Close();
	AssetManager::Close();
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

	WidgetManager::Render();

	Renderer::EndFrame();
}

void Editor::LoadHomeScreen() {
	WidgetManager::Load("Magma/assets/UI/Home.rml");

	auto doc = WidgetManager::GetDocument();
	Rml::Element* element;
	element = doc->GetElementById("new-project");
	element->AddEventListener("click",
		new ElementEventListener(element,
			[doc](Rml::Element* element, Rml::Event& event)
			{
				auto popup = doc->GetElementById("popup-container");
				bool visible = popup->IsPseudoClassSet("visible");
				popup->SetPseudoClass("visible", !visible);
				printf("Clicked");
			}
		),
		true
	);
}

void Editor::LoadFlowEditor() {

}

void Editor::LoadProjectEditor() {

}

void Editor::RegisterInterface() {

}

}
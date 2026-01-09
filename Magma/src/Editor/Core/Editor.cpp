#include "Editor.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Log.h>

#include <VolcanicWindow/Application.h>
#include <VolcanicWindow/Events.h>

#include "Integration/AI/AI.h"

#include "Utils/YAMLSerializer.h"
#include "Widget/Widget.h"
#include "Widget/Graph.h"
#include "Graphics/Renderer.h"
#include "Graphics/Platform/RendererAPI.h"
#include "Networking/Networking.h"
#include "Asset/Asset.h"
#include "Script/ScriptManager.h"

#include "Integration/Core/Graph.h"

#undef LoadImage
#undef LoadImageW
#undef LoadImageA

using namespace VolcaniCore;
using namespace VolcanicWindow;

// using namespace Magma::Asset;
using namespace Magma::UI;
using namespace Magma::Graphics;
using namespace Magma::Networking;

namespace Magma {

void Editor::Open() {
	AssetManager::Init();
	Renderer::Init();
	WidgetManager::Init();
	ScriptEngine::Init();

	GraphManager::Init();

	Events::RegisterListener<KeyPressedEvent>(
		[](const KeyPressedEvent& event)
		{
			if(event.Key == Key::R)
				WidgetManager::Reload();
		});

	LoadProjectEditor();
}

void Editor::Close() {
	ScriptEngine::Shutdown();
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

static GraphView* s_GraphView = nullptr;

void Editor::Update(TimeStep ts) {
	// if(Mode == EditorMode::Project)
	// 	for(auto& tab : m_Tabs)
	// 		tab.OnUpdate(ts);

	if(Mode == EditorMode::Project && s_GraphView)
		s_GraphView->Update(ts);

	WidgetManager::Update(ts);
}

void Editor::Render() {
	Renderer::BeginFrame();

	WidgetManager::Render();

	Renderer::EndFrame();
}

void Editor::LoadHomeScreen() {
	Mode = EditorMode::None;
	WidgetManager::Load("Magma/assets/UI/Home.rml");

	auto doc = WidgetManager::GetDocument();
	Rml::Element* element;

	element = doc->GetElementById("close-button");
	element->AddEventListener(Rml::EventId::Click,
		new ElementEventListener(element,
			[doc, this](Rml::Element* e, Rml::Event& event)
			{
				Application::Close();
			}
		)
	);

	element = doc->GetElementById("new-project");
	element->AddEventListener(Rml::EventId::Click,
		new ElementEventListener(element,
			[doc, this](Rml::Element* e, Rml::Event& event)
			{
				auto popup = doc->GetElementById("popup-container");
				popup->SetPseudoClass("visible", true);
			}
		)
	);

	element = doc->GetElementById("okButton");
	element->AddEventListener(Rml::EventId::Click,
		new ElementEventListener(element,
			[doc, this](Rml::Element* e, Rml::Event& event)
			{
				LoadProjectEditor();
			}
		)
	);
}

void Editor::LoadProjectEditor() {
	Mode = EditorMode::Project;
	WidgetManager::Load("Magma/assets/UI/Project.rml");

	auto doc = WidgetManager::GetDocument();
	Rml::Element* element;

	element = doc->GetElementById("scan-button");
	element->AddEventListener(Rml::EventId::Click,
		new ElementEventListener(element,
			[doc, this](Rml::Element* e, Rml::Event& event)
			{
				printf("Scanning project...\n");
				if(s_GraphView)
					s_GraphView->Build();
				else {
					auto graph =
						GraphManager::CreateGraph(Application::GetLibraryDir());
					s_GraphView = GraphView::Create(graph);
				}
			}
		)
	);
}

void Editor::RegisterInterface() {
	
}

}
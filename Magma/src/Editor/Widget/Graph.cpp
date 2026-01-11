#include "Graph.h"

#include <VolcaniCore/Core/Defines.h>

namespace Magma::UI {

class CanvasEventListener : public Rml::EventListener {
public:
	void ProcessEvent(Rml::Event& event) override {
		auto element = event.GetTargetElement();

		if(event.GetType() == "dragstart") {

		}

		else if(event.GetType() == "drag") {
			Log::Info("Drag");
			f32 dx = event.GetParameter<f32>("mouse_delta_x", 0);
			f32 dy = event.GetParameter<f32>("mouse_delta_y", 0);
		}

		else if(event.GetType() == "dragend") {
			
		}
	}

	void OnAttach(Rml::Element* element) override {
		
	}

	void OnDetach(Rml::Element* element) override {
		// delete this;
	}
};

class NodeEventListener : public Rml::EventListener {
public:
	void ProcessEvent(Rml::Event& event) override {
		auto element = event.GetTargetElement();
		// if(element != event.GetCurrentElement())
		// 	return;

		if(event.GetType() == "click") {
			Log::Info("Click Node");
		}

		if(event.GetType() == "dragstart") {
			Log::Info("Drag Node Start");
		}

		else if(event.GetType() == "drag") {
			Log::Info("Drag Node");
			f32 dx = event.GetParameter<f32>("mouse_delta_x", 0);
			f32 dy = event.GetParameter<f32>("mouse_delta_y", 0);
			f32 x = element->GetProperty<f32>("left");
			f32 y = element->GetProperty<f32>("top");
			element->SetProperty("left", std::format("{}px", x + dx));
			element->SetProperty("top", std::format("{}px", y + dy));
		}

		else if(event.GetType() == "dragend") {
			
		}
	}

	void OnAttach(Rml::Element* element) override {
		
	}

	void OnDetach(Rml::Element* element) override {
		// delete this;
	}
};

static std::string GetNodeClass(NodeType type) {
	switch(type) {
		case NodeType::Project:    return "project";
		case NodeType::Folder:     return "folder";
		case NodeType::File:       return "file";
		case NodeType::System:     return "system";
		case NodeType::Module:     return "module";
		case NodeType::Class:      return "class";
		case NodeType::Function:   return "function";
		case NodeType::Expression: return "expression";
		case NodeType::Variable:   return "variable";
		default:
			return "default";
	}
}

static NodeView CreateNode(Node* node, Rml::Element* parent, f32 x, f32 y) {
	auto doc = WidgetManager::GetDocument();

	auto element = doc->CreateElement("div");
	element->SetClass("graph-node", true);
	element->SetPseudoClass(GetNodeClass(node->Type), true);

	auto listener = new NodeEventListener();
	element->AddEventListener(Rml::EventId::Click, listener);
	element->AddEventListener(Rml::EventId::Dragstart, listener);
	element->AddEventListener(Rml::EventId::Drag, listener);
	element->AddEventListener(Rml::EventId::Dragend, listener);

	element->SetProperty("left", std::format("{}px", x));
	element->SetProperty("top", std::format("{}px", y));

	// Create node content
	auto nameElement = doc->CreateElement("div");
	nameElement->SetClass("node-name", true);

	// Truncate long names
	std::string displayName = node->Name;
	if(displayName.length() > 20)
		displayName = displayName.substr(0, 17) + "...";

	nameElement->SetInnerRML(displayName.c_str());
	element->AppendChild(std::move(nameElement));

	return { node, parent->AppendChild(std::move(element)) };
}

static EdgeView CreateEdge(Edge* edge) {
	auto doc = WidgetManager::GetDocument();
	return { };
}

static List<GraphView> s_Views;

GraphView* GraphView::Create(Graph* graph) {
	auto& view = s_Views.Emplace(graph);
	view.Build();
	return &view;
}

void GraphView::Build() {
	if(!RootGraph)
		return;

	auto doc = WidgetManager::GetDocument();

	// Build has been called before, so clear old nodes
	if(Canvas) {
		VisibleNodes.Clear();
		VisibleEdges.Clear();
		while(Canvas->GetNumChildren())
			Canvas->RemoveChild(Canvas->GetChild(0));
	}

	if(!Canvas) {
		Canvas = doc->GetElementById("graph-canvas");
		auto listener = new CanvasEventListener();
		Canvas->AddEventListener(Rml::EventId::Dragstart, listener);
		Canvas->AddEventListener(Rml::EventId::Drag, listener);
		Canvas->AddEventListener(Rml::EventId::Dragend, listener);

		if(!Canvas) {
			Log::Error("Could not find graph canvas");
			return;
		}
	}

	u32 level = 0;
	if(Zoom > 2.0f)
		level = 4;
	else if(Zoom > 1.5f)
		level = 3;
	else if(Zoom > 1.0f)
		level = 2;
	else if(Zoom > 0.5f)
		level = 1;

	const f32 gridSpacing = 250.0f;
	const f32 startX = 5.0f;
	const f32 startY = 5.0f;
	const u32 columns = 5; // Nodes per row

	u32 index = 0;
	GraphManager::ExtractLevel(RootGraph,
		[&](Node& node)
		{
			u32 col = index % columns;
			u32 row = index / columns;
			f32 x = startX + col * gridSpacing;
			f32 y = startY + row * gridSpacing;
			auto view = CreateNode(&node, Canvas, x, y);
			VisibleNodes.Push(view);

			index++;
		}, 1);
}

void GraphView::Update(TimeStep ts) {

}

}
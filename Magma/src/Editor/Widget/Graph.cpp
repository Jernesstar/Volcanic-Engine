#include "Graph.h"

#include <VolcaniCore/Core/Defines.h>

namespace Magma::UI {

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

static NodeView CreateNode(Node* node, Rml::Element* parent) {
	auto doc = WidgetManager::GetDocument();

	auto element = doc->CreateElement("div");
	element->SetClass("graph-node", true);
	element->SetPseudoClass(GetNodeClass(node->Type), true);

	// f32 x = Random::RandUInt();
	// f32 y = Random::RandUInt();
	f32 x = 0.0f, y = 0.0f;
	// element->SetProperty("left", std::format("{}px", x));
	// element->SetProperty("top", std::format("{}px", y));

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
		if(!Canvas) {
			Log::Error("Could not find graph canvas");
			return;
		}
	}

	u32 level = 0;
	if(Zoom > 0.5f)
		level = 1;
	else if(Zoom > 1.0)
		level = 2;
	else if(Zoom > 1.5)
		level = 3;
	else if(Zoom > 2.0)
		level = 4;

	GraphManager::TraverseBFS(RootGraph,
		[&](Node& node)
		{
			auto view = CreateNode(&node, Canvas);
			VisibleNodes.Push(view);
		}, level);
}

void GraphView::Update(TimeStep ts) {

}

}
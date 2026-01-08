#pragma once

#include "Integration/Core/Graph.h"

#include "Widget.h"

namespace Magma::UI {

struct NodeView {
	Node* Data;
	Rml::Element* UI;
};

struct EdgeView {
	Edge* Data;
	Rml::Element* UI;
};

struct GraphView {
	Graph* RootGraph = nullptr;
	Rml::Element* Canvas = nullptr;

	f32 Zoom = 1.0f, PanX = 0.0f, PanY = 0.0f;
	List<NodeView> VisibleNodes;
	List<EdgeView> VisibleEdges;

	void Build();
	void Update(TimeStep ts);

	static GraphView* Create(Graph* graph);
};

}
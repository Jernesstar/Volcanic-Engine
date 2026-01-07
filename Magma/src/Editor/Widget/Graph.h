#pragma once

#include "Integration/Core/Graph.h"

#include "Widget.h"

namespace Magma::UI {

struct GraphView {
	Graph* RootGraph = nullptr;
	Rml::Element* Canvas = nullptr;

	f32 Zoom = 1.0f, PanX = 0.0f, PanY = 0.0f;

	void Build();
	void Update(TimeStep ts);
	void Render();
};

}
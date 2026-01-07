#include "Graph.h"

namespace Magma::UI {

void GraphView::Build() {
	if(!RootGraph)
		return;

	if(!Canvas) {
		Canvas = WidgetManager::GetDocument()->GetElementById("graph-canvas");
		if(!Canvas) {
			Log::Error("Could not find graph canvas");
			return;
		}
	}

	
}

void GraphView::Update(TimeStep ts) {

}

void GraphView::Render() {

}

}
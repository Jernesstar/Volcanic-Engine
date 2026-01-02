#include "Graph.h"

#include <VolcaniCore/Core/UUID.h>
#include <VolcaniCore/Core/FileUtils.h>

using namespace VolcaniCore;

namespace Magma {

Map<UUID, Graph> s_Graphs;

void GraphManager::Init() {

}

void GraphManager::Shutdown() {

}

static Graph* NewGraph() {
	auto id = UUID();
	auto& graph = s_Graphs[id];
	graph.ID = id;
	return &graph;
}

static Node* NewNode(Graph* graph) {
	Node& node = graph->Nodes.Emplace();
	node.ID = UUID();
	return &node;
}

static void TraverseGraph(Graph* graph, Graph* parent) {
	auto& node = graph->Nodes[0];
	if(node.Kind == NodeKind::Project) {
		for(auto p : FileUtils::GetFiles(node.Path)) {
			auto path = fs::path(p);
			Node* node = NewNode(graph);
			if(fs::is_directory(path)) {
				node->Kind = NodeKind::Folder;
				node->Name = path.filename().string();
				node->Path = path.string();
			} else if(fs::is_regular_file(path)) {
				node->Kind = NodeKind::File;
				node->Name = path.filename().string();
				node->Path = path.string();
			}
		}
	}
}

Graph* GraphManager::CreateGraph(const std::string& path) {
	auto rootGraph = NewGraph();
	Node rootNode;
	rootNode.ID = UUID();
	rootNode.Kind = NodeKind::Project;
	rootNode.Name = "Project";
	rootNode.Path = path;

	rootGraph->Nodes.Add(rootNode);

	TraverseGraph(rootGraph, nullptr);

	return rootGraph;
}

Graph* GraphManager::GetGraph(UUID graphID) {
	return &s_Graphs[graphID];
}

void GraphManager::DeleteGraph(UUID graphID) {
	s_Graphs.erase(graphID);
}

void GraphManager::TraverseBFS(Graph* graph, const Func<void, Node&>& cb) {

}

void GraphManager::TraverseDFS(Graph* graph, const Func<void, Node&>& cb,
	u32 depth)
{

}

}
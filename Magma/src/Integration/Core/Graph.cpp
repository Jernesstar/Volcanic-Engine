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

static Graph* NewGraph(UUID id) {;
	auto& graph = s_Graphs[id];
	graph.ID = id;
	return &graph;
}

static Node* NewNode(Graph* graph) {
	Node& node = graph->Nodes.Emplace();
	node.ID = UUID();
	return &node;
}

static void BuildGraph(Graph* graph, Graph* parent) {
	for(auto& node : graph->Nodes) {
		if(node.Type == NodeType::None)
			continue;

		auto subgraph = NewGraph(node.ID);

		// Project or folder
		if((u32)node.Type <= (u32)NodeType::Folder) {
			for(auto p : FileUtils::GetFiles(node.Path)) {
				auto path = fs::path(p);
				Node* n = NewNode(graph);
				if(fs::is_directory(path)) {
					n->Type = NodeType::Folder;
					n->Name = path.filename().string();
					n->Path = path.string();
				}
				else if(fs::is_regular_file(path)) {
					n->Type = NodeType::File;
					n->Name = path.filename().string();
					n->Path = path.string();
				}
			}
		}

		if(node.Type == NodeType::File) {
			
		}

		BuildGraph(subgraph, graph);
	}
}

Graph* GraphManager::CreateGraph(const std::string& path) {
	auto rootGraph = NewGraph(1);
	Node* rootNode = NewNode(rootGraph);
	rootNode->Type = NodeType::Project;
	rootNode->Name = "Project";
	rootNode->Path = path;

	BuildGraph(rootGraph, nullptr);

	return rootGraph;
}

Graph* GraphManager::GetGraph(UUID graphID) {
	if(!s_Graphs.contains(graphID))
		return nullptr;
	return &s_Graphs[graphID];
}

void GraphManager::DeleteGraph(UUID graphID) {
	auto* graph = GetGraph(graphID);
	if(!graph)
		return;

	for(auto& node : graph->Nodes)
		DeleteGraph(node.ID);

	s_Graphs.erase(graphID);
}

void GraphManager::TraverseBFS(Graph* graph, const Func<void, Node&>& cb) {

}

void GraphManager::TraverseDFS(Graph* graph, const Func<void, Node&>& cb,
	u32 depth)
{
	for(auto& node : graph->Nodes) {
		cb(node);
		auto* graph = GetGraph(node.ID);
		if(graph)
			TraverseBFS(graph, cb);
	}
}

}
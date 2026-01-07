#include "Graph.h"

#include <VolcaniCore/Core/UUID.h>
#include <VolcaniCore/Core/FileUtils.h>

#include "Language/Language.h"

using namespace VolcaniCore;
using namespace Magma::Language;

namespace Magma {

Map<UUID, Graph> s_Graphs;

void GraphManager::Init() {
	LanguageManager::Init();
}

void GraphManager::Shutdown() {

}

static Graph* NewGraph(UUID id) {
	if(s_Graphs.contains(id)) {
		Log::Warning("Graph already exists");
		return &s_Graphs[id];
	}

	auto& graph = s_Graphs[id];
	graph.ID = id;
	return &graph;
}

static Node* NewNode(Graph* graph) {
	Node& node = graph->Nodes.Emplace();
	node.ID = UUID();
	return &node;
}

static Edge* NewEdge(Graph* graph, UUID to = 0, UUID from = 0) {
	Edge& edge = graph->Edges.Emplace();
	edge.ID = UUID();
	edge.To = to;
	edge.From = from;
	return &edge;
}

static void GraphFileSystem(Graph* graph, Node* parentNode) {
	if(!parentNode)
		return;

	for(auto p : FileUtils::GetFiles(parentNode->Path)) {
		auto path = fs::path(p);
		auto name = path.filename().string();
		if(name == ".vendor" || name == ".git" || name == "build")
			continue;

		Node* n = NewNode(graph);
		n->Type =
			fs::is_directory(path) ? NodeType::Folder : NodeType::File;
		n->Name = path.filename().string();
		n->Path = path.string();

		if(n->Type == NodeType::Folder)
			GraphFileSystem(NewGraph(n->ID), n);
	}
}

static void GraphLanguage(Graph* graph) {
	for(auto& node : graph->Nodes) {
		if(node.Type == NodeType::Folder) {
			GraphLanguage(GraphManager::GetGraph(node.ID));
			continue;
		}

		if(node.Type != NodeType::File)
			continue;

		auto parser = LanguageManager::GetParser(node.Path);
		if(!parser)
			continue;

		
	}
}


Graph* GraphManager::CreateGraph(const std::string& path) {
	auto rootGraph = NewGraph(UUID());
	Node* rootNode = NewNode(rootGraph);
	rootNode->Type = NodeType::Project;
	rootNode->Name = "Project";
	rootNode->Path = path;

	auto* graph = NewGraph(rootNode->ID);
	GraphFileSystem(graph, rootNode);
	GraphLanguage(graph);

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
	if(depth == 0)
		return;

	for(auto& node : graph->Nodes) {
		cb(node);
		auto* graph = GetGraph(node.ID);
		if(graph)
			TraverseDFS(graph, cb, depth - 1);
	}
}

}
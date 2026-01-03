#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/UUID.h>

using namespace VolcaniCore;

namespace Magma {

enum class NodeType {
	None,
	Project,
	Folder,
	File,
	System,   // Functionality spanning many files
	Module,   // Functionality within one or a few files
	Class,
	Function,
	Expression,
	Variable
};

enum class EdgeType {
	None,
	Implementation,
	Ownership,
	Data,
};

// Range: 0.0 -> 1.0
struct NodeMetrics {
	f32 Complexity = 0.0f;  // The mental load required to understand the code
	f32 Volatility = 0.0f;  // Likelihood of change
	f32 Performance = 0.0f; // Time complexity, memory complexity, allocations, cache hits
	f32 Readability = 0.0f; // Nesting, line length, conditional branches (cyclomatic complexity)
	f32 TotalScore = 0.0f;  // Average of all metrics: (C + V + P + R) / 4
};

struct Node {
	UUID ID = 0;   // Both the Node's ID and the Subgraph ID
	NodeType Type = NodeType::None;
	NodeMetrics Metrics;
	std::string Name, Path;
	u64 Line = 0, Column = 0, EndLine = 0, EndColumn = 0;
};

struct Edge {
	UUID ID = 0;
	UUID To = 0;
	UUID From = 0;
	EdgeType Type = EdgeType::None;
};

struct Graph {
	UUID ID = 0;
	UUID ParentNodeID = 0;
	List<Node> Nodes;
	List<Edge> Edges;
};

class GraphManager {
public:
	static void Init();
	static void Shutdown();

	static Graph* CreateGraph(const std::string& path);
	static Graph* GetGraph(UUID graphID);
	static void DeleteGraph(UUID graphID);

	static void TraverseBFS(Graph* graph, const Func<void, Node&>& cb);
	// depth = 0 => infinite
	static void TraverseDFS(Graph* graph, const Func<void, Node&>& cb, u32 depth = 0);
};

}
#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace Magma {

using NodeID = u64;
using EdgeID = u64;
using GraphID = u64;

enum class NodeKind {
	System,   // Functionality spanning many files
	Module,   // Functionality within one or a few files
	Class,
	Function,
	Expression,
	Variable
};

enum class EdgeKind {
	Implementation, // Subclass
	Ownership,      // Resource ownership
	Data            // Data flow
};

// Range: 0.0 -> 1.0
struct NodeMetrics {
	f32 Complexity = 0.0f;      // The mental load required to understand the code
	f32 Volatility = 0.0f;      // Likelihood of change
	f32 Performance = 0.0f;     // Time complexity, memory complexity, allocations, cache hits
	f32 Readability = 0.0f;     // Nesting, line length, conditional branches (cyclomatic complexity)
	f32 TotalScore = 0.0f;      // Average of all metrics: (C + V + P + R) / 4
};

struct Node {
	NodeID ID;
	NodeKind Kind;
	NodeMetrics Metrics;
	std::string Name;
	std::string Path;
	u64 Line;
	GraphID SubgraphID;
};

struct Edge {
	EdgeID ID;
	NodeID To;
	NodeID From;
	EdgeKind Kind;
};

struct Graph {
	u64 ID;
	u64 ParentID;
	List<Node> Nodes;
	List<Edge> Edges;
};

class GraphManager {
public:
	static void Init();
	static void Shutdown();

	static const Graph* CreateGraph();
	static const Graph* GetGraph(GraphID graphID);
	static void DeleteGraph(GraphID graphID);
};

}
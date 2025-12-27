#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace Magma {

using NodeID = u64;
using EdgeID = u64;

enum class NodeKind {
	System,   // Functionality spanning multiple files
	Module,   // Functionality within a single file
	Function,
	Class,
	Expression,
	Variable
};

enum class EdgeKind {
	Dependency, // Dependency injection
	Ownership,  // Resource ownership
	DataFlow    // Data flow
};

struct NodeMetrics {
	f32 Complexity = 0.0f;
	f32 Velocity = 0.0f;
	f32 Readability = 0.0f;
	f32 Performance = 0.0f;
	f32 Maintainability = 0.0f;
};

struct Node {
	NodeID ID;
	NodeKind Kind;
	NodeMetrics Metrics;
	std::string Name;
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

}
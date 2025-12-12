#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

namespace Magma {

struct Node {
	std::string Title;
	std::string Content;
};

struct Edge {
	u32 Node1;
	u32 Node2;
	u8 Direction; // 0 = Node1 -> Node2, 1 = Node2 -> Node1, 2 = Node1 <-> Node2
	f32 Weight;
};

struct Graph {
	List<Node> Nodes;
	List<Edge> Edges;
};

}
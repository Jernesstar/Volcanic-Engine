#pragma once

#include <VolcaniCore/Core/Defines.h>

namespace Magma::AI {

enum class TaskType {
	Chat, // Entry point
	Plan, // Schemas, Components, Flows
	CodeGen, // Script register, add ECS system
	Refactor, // Rewrite
	Research, // Google search, find similar solutions, LavaFlows
};

struct Response {

};

class Assistant {
public:
	Assistant();
	~Assistant();

	Response HandleRequest(const std::string& request);
};

}
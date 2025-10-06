#include <VolcaniCore/Core/Defines.h>

namespace Volcanic::AI {

enum class TaskType {
	Chat, // Entry point
	Plan, // Schemas, Components, Flows
	CodeGen, // Script register, add ECS system
	Refactor, // Rewrite into new API
	Research, // Google search, find similar solutions, LavaFlows
};

struct TaskDescriptor {
	TaskType Type;
	std::string Prompt;
	std::string ProjectContext;
	Map<std::string, std::string> Metadata;
	bool PreferLocal = true;
};

}
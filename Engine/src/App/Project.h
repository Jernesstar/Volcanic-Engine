#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace VolcanicEngine {

class Project {
public:
	std::string Path;
	std::string VolcanicEngineFlow;
	std::string Name;

public:
	Project() = default;
	~Project() = default;
};

}
#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace Magma {

class Project {
public:
	std::string Path;
	std::string LavaFlow;
	std::string Name;

public:
	Project() = default;
	~Project() = default;
};

}
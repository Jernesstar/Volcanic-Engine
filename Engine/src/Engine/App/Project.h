#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace VolcanicEngine {

class Project {
public:
	std::string Name;
	std::string StartScreen;

public:
	Project() = default;
	~Project() = default;
};

}

#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace VolcanicEngine {

struct Screen {
	const std::string& Name;
	const std::string& Scene;
	const std::string& Canvas;
};

class Project {
public:
	std::string Name;
	std::string App;
	List<Screen> Screens;

public:
	Project() = default;
	~Project() = default;
};

}
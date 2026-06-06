#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

#include <filesystem>
#include <string>

using namespace VolcaniCore;

namespace VolcanicEngine {

class Project {
public:
	std::string Name;
	std::string Path;
	std::string StartScreen;

public:
	Project() = default;
	~Project() = default;

	std::string ResolvePath(const std::string& relative) const {
		return (std::filesystem::path(Path) / relative).generic_string();
	}
};

}

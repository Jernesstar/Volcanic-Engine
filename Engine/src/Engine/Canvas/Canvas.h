#pragma once

#include <VolcaniCore/Core/TimeUtils.h>

#include "ECS/World.h"

#include "CanvasRenderer.h"

using namespace VolcaniCore;

namespace VolcanicEngine {

class Canvas {
public:
	std::string Name;
	ECS::World EntityWorld;

public:
	Canvas(const std::string& name = "Untitled Canvas");
	~Canvas() = default;
};

}
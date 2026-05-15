#pragma once

#include <VolcaniCore/Core/TimeUtils.h>

#include "ECS/World.h"

using namespace VolcaniCore;

namespace VolcanicEngine {

class Scene {
public:
	std::string Name;
	ECS::World World3D;
	ECS::World World2D;
	ECS::World Canvas;

public:
	Scene(const std::string& name = "Untitled Scene");
	~Scene() = default;

	void OnUpdate(TimeStep ts);
	void RegisterSystems();
	void UnregisterSystems();
};

}
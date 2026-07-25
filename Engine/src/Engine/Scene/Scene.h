#pragma once

#include <VolcaniCore/Core/TimeUtils.h>

#include "ECS/World.h"
#include "GameSystemRegistry.h"

using namespace VolcaniCore;

namespace VolcanicEngine {

class Scene {
public:
	std::string Name;
	ECS::World World3D;
	ECS::World World2D;
	ECS::World Canvas;

	// Fixed-tick gameplay systems (script-registered via Scene.AddSystem). Driven
	// each frame before World3D, scoped to this scene's lifetime.
	GameSystemRegistry GameSystems;

public:
	Scene(const std::string& name = "Untitled Scene");
	~Scene();

	void OnUpdate(TimeStep ts);
	void RegisterSystems();
	void UnregisterSystems();
};

}
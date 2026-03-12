#pragma once

#include <VolcaniCore/Core/TimeUtils.h>

#include "ECS/World.h"

#include "SceneRenderer.h"

using namespace VolcaniCore;

namespace VolcanicEngine {

class Scene {
public:
	std::string Name;
	std::string Screen;
	ECS::World EntityWorld;

public:
	Scene(const std::string& name = "Untitled Scene");
	~Scene() = default;

	void OnUpdate(TimeStep ts);
	void OnRender(SceneRenderer& renderer);
	void RegisterSystems();
	void UnregisterSystems();
};

}
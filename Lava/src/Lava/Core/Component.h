#pragma once

#include <VolcaniCore/Core/Time.h>

using namespace VolcaniCore;

namespace Lava {

class Component {
public:
	const std::string Name;

public:
	Component(const std::string& name)
		: Name(name) { }
	virtual ~Component() = default;

	virtual void Init() = 0;
	virtual void Shutdown() = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;

	virtual void OnUpdate(TimeStep ts) = 0;
};

extern Component* CreateComponent();

}
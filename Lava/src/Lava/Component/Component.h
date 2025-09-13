#pragma once

#include <VolcaniCore/Core/Time.h>

using namespace VolcaniCore;

namespace Lava {

class Component {
public:
	Component() = default;
	~Component() = default;

	virtual void Init() = 0;
	virtual void Shutdown() = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;

	virtual void OnUpdate(TimeStep ts) = 0;
	virtual void OnEvent(uint32_t event) = 0;
};

}
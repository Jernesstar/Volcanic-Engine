#pragma once

#include "Component.h"

namespace Lava {

class Silica : public Component {
public:
	void Init() override;
	void Shutdown() override;
	void BeginFrame() override;
	void EndFrame() override;

	void OnUpdate(TimeStep ts) override;
	void OnEvent(uint32_t event) override;
}
;
}
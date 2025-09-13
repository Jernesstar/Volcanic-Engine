#pragma once

#include "Component.h"

using namespace VolcaniCore;

namespace Lava {

enum AshEvent : uint32_t {
	AssetAdded,
	AssetRemoved,
	AssetLoaded,
	AssetUnloaded,
	AssetUpdated,
	RegistryLoaded,
	RegistryUnloaded
};

class Ash : public Component {
public:
	void Init() override;
	void Shutdown() override;
	void BeginFrame() override;
	void EndFrame() override;

	void OnUpdate(TimeStep ts) override;
	void OnEvent(uint32_t event) override;
};

}
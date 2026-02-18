// #pragma once

// #include "Component.h"

// using namespace VolcaniCore;

// namespace VolcanicEngine {

// enum AshEvent : uint32_t {
// 	AssetAdded,
// 	AssetDeleted,
// 	AssetLoaded,
// 	AssetUnloaded,
// 	AssetSourceUpdated,
// 	AssetSourceDeleted,
// 	AssetMetadataUpdated,
// 	AssetMetadataDeleted
// };

// class Ash : public Component {
// public:
// 	void Init() override;
// 	void Shutdown() override;
// 	void BeginFrame() override;
// 	void EndFrame() override;

// 	void OnUpdate(TimeStep ts) override;
// 	void OnEvent(uint32_t event) override;
// };

// }
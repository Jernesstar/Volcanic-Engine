#pragma once

#include <VolcaniCore/Utils/BinaryReader.h>
#include <Engine/Asset/AssetManager.h>

using namespace VolcaniCore;
using namespace VolcanicEngine;

namespace VolcanicRuntime {

class RuntimeAssetManager : public AssetManager {
public:
	RuntimeAssetManager();
	~RuntimeAssetManager();

	void Load(Asset asset) override;
	void Unload(Asset asset) override;

	void Load();

private:
	Map<UUID, uint64_t> m_AssetOffsets;
};

}
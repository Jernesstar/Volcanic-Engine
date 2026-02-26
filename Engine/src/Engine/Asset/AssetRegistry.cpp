#include "AssetRegistry.h"

namespace VolcanicEngine {

AssetRegistry::AssetRegistry() {
	m_Registry = ("./volc/assets/registry.db", 2);
	m_AssetMetadata = m_Registry.NewDatabase("AssetMetadata");
	m_AssetData = m_Registry.NewDatabase("AssetData");
}

AssetRegistry::~AssetRegistry() { }

void AssetRegistry::Add(Asset asset) { }

void AssetRegistry::Remove(Asset asset) { }

bool AssetRegistry::IsValid(Asset asset) const {
	return asset.ID != 0 && asset.Type != AssetType::None;
}

bool AssetRegistry::IsLoaded(Asset asset) const {
	auto res = m_AssetMetadata->Query({ (u64)asset.ID });
	if(res) {
		auto& asset = res.Get<Asset>();
	}
}

}
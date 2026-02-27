#include "AssetRegistry.h"

#include <VolcaniCore/Core/Application.h>

namespace VolcanicEngine {

AssetRegistry::AssetRegistry() {
	Application::PushDir();
	m_Registry = new Registry("./.volc/assets/registry.db", 2);
	Application::PopDir();
	m_AssetMetadata = m_Registry->NewDatabase("AssetMetadata");
	m_AssetData = m_Registry->NewDatabase("AssetData");
}

AssetRegistry::~AssetRegistry() {
	delete m_Registry;
}

void AssetRegistry::Add(Asset asset) {
	m_AssetMetadata->Insert(
		(u64)asset.ID, { (u8*)&asset, sizeof(Asset), 0, false });
}

void AssetRegistry::Remove(Asset asset) {
	m_AssetMetadata->Remove((u64)asset.ID);
}

bool AssetRegistry::IsValid(Asset asset) const {
	return asset.ID != 0 && asset.Type != AssetType::None;
}

bool AssetRegistry::HasRefs(Asset asset) const {

}

void AssetRegistry::AddRef(Asset base, Asset ref) {

}

const List<Asset>& AssetRegistry::GetRefs(Asset asset) const {

}

}
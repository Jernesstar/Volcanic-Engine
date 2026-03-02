#include "AssetRegistry.h"

#include <VolcaniCore/Core/Application.h>

namespace VolcanicEngine {

AssetRegistry::AssetRegistry() {
	m_Registry = new Registry("./.volc/assets/registry.db", 2);
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

void AssetRegistry::SetData(Asset asset, Bytes&& data) {
	m_AssetData->Insert((u64)asset.ID, std::move(data));
}

Bytes AssetRegistry::GetData(Asset asset) {
	return m_AssetData->Query((u64)asset.ID).Data;
}

bool AssetRegistry::HasRefs(Asset asset) const {
	return false;
}

void AssetRegistry::AddRef(Asset base, Asset ref) {

}

const List<Asset>& AssetRegistry::GetRefs(Asset asset) const {

}

void AssetRegistry::NameAsset(Asset asset, const std::string& name) {

}

void AssetRegistry::RemoveName(Asset asset) {

}

std::string AssetRegistry::GetAssetName(Asset asset) const {

}

Asset AssetRegistry::FindAsset(const std::string& lookup) const {

}

void AssetRegistry::For(const Func<void, Asset>& cb) {

}

void AssetRegistry::Clear() {

}

}
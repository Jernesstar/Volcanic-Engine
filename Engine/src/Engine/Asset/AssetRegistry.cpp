#include "AssetRegistry.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Utils/BinaryReader.h>
#include <VolcaniCore/Utils/BinaryWriter.h>

namespace VolcanicEngine {

AssetRegistry::AssetRegistry() {
	// m_Registry = new Registry(".volc/asset_registry", 4);
	// m_AssetMetadata = m_Registry->NewDatabase("AssetMetadata");
	// m_AssetRefs = m_Registry->NewDatabase("AssetRefs", true);
	// m_AssetNames = m_Registry->NewDatabase("AssetNames");
	// m_AssetNamesReverse = m_Registry->NewDatabase("AssetNamesReverse");
}

AssetRegistry::~AssetRegistry() {
	// delete m_Registry;
}

void AssetRegistry::Add(Asset asset) {
	// Bytes bytes = { (u8*)&asset, sizeof(Asset), 0, false };
	// m_AssetMetadata->Insert((u64)asset.ID, std::move(bytes));
	m_Registry[asset.ID] = asset;
}

void AssetRegistry::Remove(Asset asset) {
	// RemoveName(asset);
	// m_AssetMetadata->Remove((u64)asset.ID);
	// m_AssetRefs->Remove((u64)asset.ID);

	auto path = "Asset/.bin/" + std::to_string((u64)asset.ID) + ".asset";
	FileUtils::DeleteFile(path);
}

void AssetRegistry::SetData(Asset asset, Bytes&& data) {
	auto path = "Asset/.bin/" + std::to_string((u64)asset.ID) + ".asset";
	FileUtils::CreateFile(path);
	BinaryWriter writer(path);
	writer.Write(data);
}

Bytes AssetRegistry::GetData(Asset asset) {
	auto path = "Asset/.bin/" + std::to_string((u64)asset.ID) + ".asset";
	BinaryReader reader(path);

	Bytes res;
	reader.Read(res);
	return res;
}

bool AssetRegistry::HasRefs(Asset asset) const {
	// auto res = m_AssetRefs->Count((u64)asset.ID);
	// if(!res.Success)
	// 	return false;

	// return res.Count > 0;
	return m_Refs.count(asset.ID) && m_Refs.at(asset.ID).Count();
}

void AssetRegistry::AddRef(Asset base, Asset ref) {
	// Bytes bytes = { (u8*)&ref, sizeof(Asset), 0, false };
	// Log::Info("Adding ref {0} to {1}", (u64)ref.ID, (u64)base.ID);
	// m_AssetRefs->Insert((u64)base.ID, std::move(bytes));
	m_Refs[base.ID].Add(ref);
}

List<Asset> AssetRegistry::GetRefs(Asset asset) const {
	// auto res = m_AssetRefs->Query((u64)asset.ID);
	// if(!res.Success)
	// 	return { };

	// List<Asset> refs;
	// for(u64 i = 0; i < res.Data.GetCount(); i++)
	// 	refs.Add(*(Asset*)res.Data.Get(i));

	// return refs;
	if(!m_Refs.count(asset.ID))
		return { };

	return m_Refs.at(asset.ID);
}

void AssetRegistry::NameAsset(Asset asset, const std::string& name) {
	// Bytes bytes = { (u8*)name.c_str(), name.size(), 0, false };
	// m_AssetNames->Insert((u64)asset.ID, std::move(bytes));
	// Bytes bytes2 = { (u8*)&asset, sizeof(Asset), 0, false };
	// m_AssetNamesReverse->Insert(name, std::move(bytes2));
	m_Names[name] = asset;
}

void AssetRegistry::RemoveName(Asset asset) {
	// std::string name = GetAssetName(asset);
	// m_AssetNames->Remove((u64)asset.ID);
	// m_AssetNamesReverse->Remove(name);
}

std::string AssetRegistry::GetAssetName(Asset asset) const {
	// auto res = m_AssetNames->Query((u64)asset.ID);
	// if(res.Success)
	// 	return std::string((char*)res.Data.Get(), res.Data.GetCount());

	// return "";
	for(auto& [name, a] : m_Names) {
		if(a.ID == asset.ID)
			return name;
	}

	return "";
}

Asset AssetRegistry::FindAsset(const std::string& lookup) const {
	// auto res = m_AssetNamesReverse->Query(lookup);
	// if(!res.Success) {
	// 	Log::Info("Could not find asset by name '{}'", lookup);
	// 	return { };
	// }

	// return *(Asset*)res.Data.Get();
	return m_Names.at(lookup);
}

void AssetRegistry::For(const Func<void, Asset>& cb) {
	// auto it = m_AssetMetadata->Iterate();
	// while(it.Next()) {
	// 	auto asset = it.Get<Asset>();
	// 	cb(asset);
	// }

	for(auto& [id, asset] : m_Registry)
		cb(asset);
}

void AssetRegistry::Clear() {

}

}

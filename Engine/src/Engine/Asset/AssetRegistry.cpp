#include "AssetRegistry.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Utils/BinaryReader.h>
#include <VolcaniCore/Utils/BinaryWriter.h>

namespace VolcanicEngine {

AssetRegistry::AssetRegistry() {
	m_Registry = new Registry(".volc/asset_registry", 5);
	m_AssetMetadata = m_Registry->NewDatabase("AssetMetadata");
	m_AssetRefs = m_Registry->NewDatabase("AssetRefs", true);
	m_AssetNames = m_Registry->NewDatabase("AssetNames");
}

AssetRegistry::~AssetRegistry() {
	delete m_Registry;
}

void AssetRegistry::Add(Asset asset) {
	Bytes bytes = { (u8*)&asset, sizeof(Asset), 0, false };
	m_AssetMetadata->Insert((u64)asset.ID, std::move(bytes));
}

void AssetRegistry::Remove(Asset asset) {
	m_AssetMetadata->Remove((u64)asset.ID);
	m_AssetRefs->Remove((u64)asset.ID);
	m_AssetNames->Remove((u64)asset.ID);

	FileUtils::DeleteFile(
		"Asset/.bin/" + std::to_string((u64)asset.ID) + ".asset");
}

void AssetRegistry::SetData(Asset asset, Bytes&& data) {
	auto path = "Asset/.bin/" + std::to_string((u64)asset.ID) + ".asset";
	FileUtils::CreateFile(path);
	BinaryWriter writer(path);
	writer.Write(data);

}

Bytes AssetRegistry::GetData(Asset asset) {
	BinaryReader reader(
		"Asset/.bin/" + std::to_string((u64)asset.ID) + ".asset");

	Bytes res;
	reader.Read(res);
	return res;
}

bool AssetRegistry::HasRefs(Asset asset) const {
	auto res = m_AssetRefs->Count((u64)asset.ID);
	if(!res.Success)
		return false;

	return res.Count > 0;
}

void AssetRegistry::AddRef(Asset base, Asset ref) {
	u64 refID = (u64)ref.ID;
	Bytes bytes = { (u8*)&refID, sizeof(Asset), 0, false };
	Log::Info("Adding ref {0} to {1}", (u64)refID, (u64)base.ID);
	m_AssetRefs->Insert((u64)base.ID, std::move(bytes));
}

List<Asset> AssetRegistry::GetRefs(Asset asset) const {
	auto res = m_AssetRefs->Query((u64)asset.ID);
	if(!res.Success)
		return { };

	List<Asset> refs;
	for(u64 i = 0; i < res.Data.GetCount(); i++)
		refs.Add(*(Asset*)res.Data.Get(i));

	return refs;
}

void AssetRegistry::NameAsset(Asset asset, const std::string& name) {
	Bytes bytes = { (u8*)name.c_str(), name.size(), 0, false };
	m_AssetNames->Insert((u64)asset.ID, std::move(bytes));
}

void AssetRegistry::RemoveName(Asset asset) {
	m_AssetNames->Remove((u64)asset.ID);
}

std::string AssetRegistry::GetAssetName(Asset asset) const {
	auto res = m_AssetNames->Query((u64)asset.ID);
	if(res.Success)
		return std::string((char*)res.Data.Get(), res.Data.GetCount());

	return "";
}

Asset AssetRegistry::FindAsset(const std::string& lookup) const {
	return { };
}

void AssetRegistry::For(const Func<void, Asset>& cb) {

}

void AssetRegistry::Clear() {

}

}
#include "AssetManager.h"

#include <iostream>
#include <bitset>

#include <glm/gtc/matrix_access.hpp>

#include <efsw/efsw.hpp>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Utils/BytesWriter.h>

#include <Engine/App/App.h>

#include "YAMLSerializer.h"
#include "AssetImporter.h"
#include "ScriptManager.h"

namespace fs = std::filesystem;

namespace VolcanicEditor {

std::string AssetTypeToString(AssetType type) {
	switch(type) {
		case AssetType::Mesh:
			return "Mesh";
		case AssetType::Texture:
			return "Texture";
		case AssetType::Cubemap:
			return "Cubemap";
		case AssetType::Shader:
			return "Shader";
		case AssetType::Font:
			return "Font";
		case AssetType::Audio:
			return "Audio";
		case AssetType::Script:
			return "Script";
		case AssetType::Material:
			return "Material";
	}

	return "None";
}

AssetType AssetTypeFromString(const std::string& str) {
	if(str == "Mesh")
		return AssetType::Mesh;
	else if(str == "Texture")
		return AssetType::Texture;
	else if(str == "Cubemap")
		return AssetType::Cubemap;
	else if(str == "Shader")
		return AssetType::Shader;
	else if(str == "Font")
		return AssetType::Font;
	else if(str == "Audio")
		return AssetType::Audio;
	else if(str == "Script")
		return AssetType::Script;
	else if(str == "Material")
		return AssetType::Material;

	return AssetType::None;
}

class FileWatcher : public efsw::FileWatchListener {
public:
	FileWatcher(EditorAssetManager* assetManager);
	~FileWatcher() = default;

	void handleFileAction(efsw::WatchID id, const std::string& dir,
		const std::string& file, efsw::Action action, std::string old) override;

	u32 AddCallback(const Func<void, Asset, bool>& callback);
	void RemoveCallback(u32 id);

	void Add(AssetType type, const std::string& path);
	void Remove(AssetType type, const std::string& path);
	void Reload(AssetType type, const std::string& path);

private:
	EditorAssetManager* m_AssetManager;
	List<Func<void, Asset, bool>> m_Callbacks;
};

FileWatcher::FileWatcher(EditorAssetManager* assetManager)
	: m_AssetManager(assetManager) { }

void FileWatcher::handleFileAction(
	efsw::WatchID id, const std::string& dir,
	const std::string& file, efsw::Action action, std::string oldFilename)
{
	auto fullPath = (fs::path(dir) / file).string();
	auto assetDir = fs::path(dir).parent_path().filename().string();
	AssetType type = AssetTypeFromString(assetDir);
	Asset asset = { m_AssetManager->GetFromPath(fullPath), type };

	if(action == efsw::Actions::Add)
		Add(type, fullPath);
	else if(action == efsw::Actions::Delete)
		Remove(type, fullPath);
	else if(action == efsw::Actions::Modified) {
		for(auto callback : m_Callbacks)
			callback(asset, 0);

		Reload(type, fullPath);

		for(auto callback : m_Callbacks)
			callback(asset, 1);
	}
	else if(action == efsw::Actions::Moved) {
		Log::Info("{} has moved from {}", file, oldFilename);
		// m_AssetManager->Change(fullPath, asset);
	}
	else {
		Log::Error("Unexpected filewatch error!");
	}
}

u32 FileWatcher::AddCallback(const Func<void, Asset, bool>& callback) {
	m_Callbacks.Add(callback);
	return m_Callbacks.Count() - 1;
}

void FileWatcher::RemoveCallback(u32 id) {
	m_Callbacks.Pop(id);
}

void FileWatcher::Add(AssetType type, const std::string& path) {
	Log::Info("Adding AssetType::{} at path '{}'",
				AssetTypeToString(type).c_str(), path);
	m_AssetManager->Add(type, 0, true, path);
}

void FileWatcher::Remove(AssetType type, const std::string& path) {
	Log::Info("Removing AssetType::{} at path '{}'",
				AssetTypeToString(type), path);
	UUID id = m_AssetManager->GetFromPath(path);
	m_AssetManager->Remove({ id, type });
}

void FileWatcher::Reload(AssetType type, const std::string& path) {
	Log::Info("Reloading AssetType::{} at path '{}'",
				AssetTypeToString(type), path);
	UUID id = m_AssetManager->GetFromPath(path);
	Asset asset = { id, type };

	if(type == AssetType::Script) {
		bool error;
		auto mod = AssetImporter::GetScriptData(path, &error, "TestBuild");
		if(error)
			return;
	}

	m_AssetManager->Unload(asset);
	m_AssetManager->Build(asset);
	m_AssetManager->Load(asset);
}

static efsw::FileWatcher* s_FileWatcher;
static FileWatcher* s_Listener;
static List<efsw::WatchID> s_WatcherIDs;

EditorAssetManager::EditorAssetManager()
	: AssetManager()
{
	s_FileWatcher = new efsw::FileWatcher();
	s_Listener = new FileWatcher(this);
}

EditorAssetManager::~EditorAssetManager() {
	Clear();
	delete s_Listener;
	delete s_FileWatcher;
}

void EditorAssetManager::Build(Asset asset) {
	if(!asset || IsLoaded(asset))
		return;

	auto path = GetPath(asset.ID);
	if(asset.Type == AssetType::Mesh) {
		List<SubMesh> meshes;
		List<MaterialPaths> materials;
		AssetImporter::GetMeshData(path, meshes, materials);

		BytesWriter wr(
			meshes.GetBuffer().GetSize() +
			materials.GetBuffer().GetSize());

		wr.Write(meshes.Count());
		for(auto& mesh : meshes) {
			wr.Write(mesh.Vertices);
			wr.Write(mesh.Indices);
			wr.Write(mesh.MaterialIndex);
		}

		// for(auto matPath : materials) {
		// }
	}
	else if(asset.Type == AssetType::Texture) {
		ImageData image = AssetImporter::GetImageData(path, false);


	}
	else if(asset.Type == AssetType::Cubemap) {
		const auto& refs = m_AssetRegistry->GetRefs(asset);
		for(auto& ref : refs)
			Build(ref);
	}
	else if(asset.Type == AssetType::Shader) {
		Buffer<u32> data = AssetImporter::GetShaderData(path);
	}
	else if(asset.Type == AssetType::Audio) {
		Buffer<f32> soundData = AssetImporter::GetAudioData(path);
	}
	else if(asset.Type == AssetType::Script) {
		auto* mod = ScriptManager::LoadScript(path, false);
	}
	else if(asset.Type == AssetType::Material) {

	}

	Application::PopDir();
}

u32 EditorAssetManager::AddReloadCallback(const Func<void, Asset, bool>& cb) {
	return s_Listener->AddCallback(cb);
}

void EditorAssetManager::RemoveReloadCallback(u32 id) {
	s_Listener->RemoveCallback(id);
}

Asset EditorAssetManager::Add(AssetType type, UUID id, bool primary,
							  const std::string& path)
{
	Asset newAsset{ id ? id : UUID(), type, primary };
	if(path != "")
		m_Paths[newAsset.ID] = path;

	m_AssetRegistry->Add(newAsset);
	return newAsset;
}

void EditorAssetManager::Remove(Asset asset) {
	Unload(asset);

	m_AssetRegistry->Remove(asset);
	m_Paths.erase(asset.ID);
}

UUID EditorAssetManager::GetFromPath(const std::string& path) const {
	for(auto& [id, assetPath] : m_Paths)
		if(fs::path(path) == fs::path(assetPath))
			return id;

	return 0;
}

std::string EditorAssetManager::GetPath(UUID id) const {
	if(!m_Paths.count(id))
		return "";
	return m_Paths.at(id);
}

void EditorAssetManager::Clear() {
	ClearCache();
	m_Path = "";
	m_Paths.clear();

	s_WatcherIDs.ForEach([](auto& id) { s_FileWatcher->removeWatch(id); });
	s_WatcherIDs.Clear();
}

void EditorAssetManager::LoadRegistry() {
	m_AssetRegistry = CreateRef<AssetRegistry>();
	
	auto rootPath = fs::path("Asset");
	m_Path = (rootPath / ".magma.assetpk").string();

	List<fs::path> paths
	{
		(rootPath / "Mesh"),
		(rootPath / "Texture"),
		(rootPath / "Cubemap"),
		(rootPath / "Shader"),
		(rootPath / "Material"),
		(rootPath / "Font"),
		(rootPath / "Audio"),
		(rootPath / "Script"),
		// (rootPath / "Custom")
	};

	for(auto& assetDir : paths)
		if(fs::exists(assetDir))
			s_WatcherIDs.Add(
				s_FileWatcher->addWatch(assetDir.string(), s_Listener));

	s_FileWatcher->watch();

	YAML::Node file;
	try {
		file = YAML::LoadFile(m_Path);
	}
	catch(YAML::ParserException e) {
		VOLCANICORE_ASSERT_ARGS(false, "Could not load file {}: {}",
								m_Path, e.what());
	}

	Log::Info("Loading Asset Registry at path '{}'", m_Path);

	auto assetPackNode = file["AssetPack"];
	if(!assetPackNode)
		return;

	for(auto assetNode : assetPackNode["Assets"]) {
		auto node = assetNode["Asset"];
		AssetType type = AssetTypeFromString(node["Type"].as<std::string>());
		UUID id = node["ID"].as<u64>();
		std::string path;
		if(node["Path"]) {
			path = (paths[(u32)type - 1] / node["Path"].as<std::string>()).generic_string();
			if(!fs::exists(path)) {
				Log::Warning("Asset path '{}' does not exist", path);
				continue;
			}
		}

		Asset asset = Add(type, id, true, path);
		if(node["Name"])
			m_AssetRegistry->NameAsset(asset, node["Name"].as<std::string>());
	}

	u32 i = 1;
	for(auto& folder : paths) {
		for(auto path : FileUtils::GetFiles(folder.string())) {
			if(i == 1)
				path = FileUtils::GetFiles(path, { ".obj" })[0];
			if(!GetFromPath(path)) {
				Log::Info("Loading Asset at path '{}'", path);
				Add((AssetType)i, 0, true, path);
			}
		}
		i++;
	}
}

void EditorAssetManager::Save() {
	namespace fs = std::filesystem;

	if(m_Path == "")
		return;
	auto rootPath = fs::path(m_Path).parent_path();

	YAMLSerializer serializer;
	serializer.BeginMapping();
	serializer.WriteKey("AssetPack").BeginMapping();

	serializer.WriteKey("Assets").BeginSequence();
	m_AssetRegistry->For(
		[&](Asset asset)
		{
			if(!asset.Primary)
				return;

			serializer.BeginMapping();
			serializer.WriteKey("Asset").BeginMapping();
			serializer.WriteKey("ID").Write((u64)asset.ID);
			serializer.WriteKey("Type").Write(AssetTypeToString(asset.Type));
			auto name = m_AssetRegistry->GetAssetName(asset);
			if(name != "")
				serializer.WriteKey("Name").Write(name);

			std::string path = GetPath(asset.ID);
			if(path != "")
				serializer.WriteKey("Path")
					.Write(fs::relative(path, rootPath).generic_string());

			serializer.EndMapping(); // Asset
			serializer.EndMapping();
		}
	);
	serializer.EndSequence();

	serializer.EndMapping(); // AssetPack
	serializer.EndMapping();

	serializer.Finalize(m_Path);
}

void EditorAssetManager::Export(const std::string& exportPath) {

}

}

namespace VolcaniCore {

template<>
BytesWriter& BytesWriter::WriteObject(const Asset& asset) {
	Write((u64)asset.ID);
	Write((u8)asset.Type);
	Write((bool)asset.Primary);

	auto reg = AssetManager::Get()->GetRegistry();
	if(reg->HasRefs(asset)) {
		const auto& refs = reg->GetRefs(asset);
		Write(refs.Count());
		for(const auto& ref : refs) {
			Write((u64)ref.ID);
			Write((u8)ref.Type);
		}
	}
	else
		Write((u64)0);

	Write(reg->GetAssetName(asset));

	return *this;
}

template<>
BytesWriter& BytesWriter::WriteObject(const Vec4& vec) {
	WriteData(&vec.x, sizeof(Vec4));
	return *this;
}

template<>
BytesWriter& BytesWriter::WriteObject(const Mat2& mat) {
	WriteData(glm::value_ptr(mat), sizeof(Mat2));
	return *this;
}

template<>
BytesWriter& BytesWriter::WriteObject(const Mat3& mat) {
	WriteData(glm::value_ptr(mat), sizeof(Mat3));
	return *this;
}

template<>
BytesWriter& BytesWriter::WriteObject(const Mat4& mat) {
	WriteData(glm::value_ptr(mat), sizeof(Vec4));
	return *this;
}

template<>
BytesWriter& BytesWriter::WriteObject(const UUID& uuid) {
	Write((u64)uuid);
	return *this;
}

}

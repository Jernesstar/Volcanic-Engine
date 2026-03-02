#include "AssetManager.h"

#include <iostream>
#include <bitset>

#include <glm/gtc/matrix_access.hpp>

#include <efsw/efsw.hpp>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Utils/BinaryWriter.h>
#include <VolcaniCore/Utils/BinaryReader.h>

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

	void ReloadMesh(const std::string& path);
	void ReloadTexture(const std::string& path);
	void ReloadShader(const std::string& path);
	void ReloadAudio(const std::string& path);
	void ReloadScript(const std::string& path);

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

		switch(type) {
			case AssetType::Mesh:
				ReloadMesh(fullPath);
				break;
			case AssetType::Texture:
				ReloadTexture(fullPath);
				break;
			// case AssetType::Cubemap:
			// 	ReloadCubemap(fullPath);
			// 	break;
			case AssetType::Shader:
				ReloadShader(fullPath);
				break;
			// case AssetType::Font:
			// 	ReloadFont(fullPath);
			// 	break;
			case AssetType::Audio:
				ReloadAudio(fullPath);
				break;
			case AssetType::Script:
				ReloadScript(fullPath);
				break;
		}

		for(auto callback : m_Callbacks)
			callback(asset, 1);
	}
	else if(action == efsw::Actions::Moved) {
		Log::Info("%s has moved from %s",
			file.c_str(), oldFilename.c_str());
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
	Log::Info("Adding AssetType::%s at path '%s'",
		AssetTypeToString(type).c_str(), path.c_str());
	m_AssetManager->Add(type, 0, true, path);
}

void FileWatcher::Remove(AssetType type, const std::string& path) {
	Log::Info("Removing AssetType::%s at path '%s'",
		AssetTypeToString(type).c_str(), path.c_str());
	UUID id = m_AssetManager->GetFromPath(path);
	m_AssetManager->Remove({ id, type });
}

void FileWatcher::ReloadMesh(const std::string& path) {
	Log::Info("Reloading Mesh at path '%s'", path.c_str());
	UUID id = m_AssetManager->GetFromPath(path);
	Asset asset = { id, AssetType::Mesh };
	m_AssetManager->Unload(asset);
	m_AssetManager->Build(asset);
	m_AssetManager->Load(asset);
}

void FileWatcher::ReloadTexture(const std::string& path) {
	Log::Info("Reloading Texture at path '%s'", path.c_str());
	UUID id = m_AssetManager->GetFromPath(path);
	Asset asset = { id, AssetType::Texture };
	m_AssetManager->Unload(asset);
	m_AssetManager->Build(asset);
	m_AssetManager->Load(asset);
}

void FileWatcher::ReloadShader(const std::string& path) {
	Log::Info("Reloading Shader at path '%s'", path.c_str());
	UUID id = m_AssetManager->GetFromPath(path);
	Asset asset = { id, AssetType::Shader };
	m_AssetManager->Unload(asset);
	m_AssetManager->Build(asset);
	m_AssetManager->Load(asset);
}

void FileWatcher::ReloadAudio(const std::string& path) {
	Log::Info("Reloading Audio at path '%s'", path.c_str());
	UUID id = m_AssetManager->GetFromPath(path);
	Asset asset = { id, AssetType::Audio };
	m_AssetManager->Unload(asset);
	m_AssetManager->Build(asset);
	m_AssetManager->Load(asset);
}

void FileWatcher::ReloadScript(const std::string& path) {
	Log::Info("Reloading Script at path '%s'", path.c_str());
	UUID id = m_AssetManager->GetFromPath(path);
	bool error;
	auto mod = AssetImporter::GetScriptData(path, &error, "TestBuild");

	if(!error) {
		Asset asset = { id, AssetType::Script };
		m_AssetManager->Unload(asset);
		m_AssetManager->Build(asset);
		m_AssetManager->Load(asset);
	}
	else
		Log::Info("Error occured when reloading script");
}

static efsw::FileWatcher* s_FileWatcher;
static FileWatcher* s_Listener;
static List<efsw::WatchID> s_WatcherIDs;

EditorAssetManager::EditorAssetManager() {
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
	}
	else if(asset.Type == AssetType::Texture) {
		ImageData image = AssetImporter::GetImageData(path, false);
		m_AssetRegistry->SetData(asset, std::move(image.Data));
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

	return newAsset;
}

void EditorAssetManager::Remove(Asset asset) {
	Unload(asset);

	// Ensures accidently removing an asset won't break references
	// m_AssetRegistry->Remove(asset);
	// m_Paths.erase(asset.ID);
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

void EditorAssetManager::LoadRegistry(const std::string& path) {
	auto rootPath = fs::path(path) / "Asset";
	m_Path = (rootPath / ".magma.assetpk").string();

	YAML::Node file;
	try {
		file = YAML::LoadFile(m_Path);
	}
	catch(YAML::ParserException e) {
		VOLCANICORE_ASSERT_ARGS(false, "Could not load file %s: %s",
								m_Path.c_str(), e.what());
	}

	fs::path assetDir;
	assetDir = rootPath / "Mesh";
	if(fs::exists(assetDir))
		s_WatcherIDs.Add(
			s_FileWatcher->addWatch(assetDir.string(), s_Listener));
	assetDir = rootPath / "Image";
	if(fs::exists(assetDir))
		s_WatcherIDs.Add(
			s_FileWatcher->addWatch(assetDir.string(), s_Listener));
	assetDir = rootPath / "Cubemap";
	if(fs::exists(assetDir))
		s_WatcherIDs.Add(
			s_FileWatcher->addWatch(assetDir.string(), s_Listener));
	assetDir = rootPath / "Shader";
	if(fs::exists(assetDir))
		s_WatcherIDs.Add(
			s_FileWatcher->addWatch(assetDir.string(), s_Listener));
	assetDir = rootPath / "Font";
	if(fs::exists(assetDir))
		s_WatcherIDs.Add(
			s_FileWatcher->addWatch(assetDir.string(), s_Listener));
	assetDir = rootPath / "Audio";
	if(fs::exists(assetDir))
		s_WatcherIDs.Add(
			s_FileWatcher->addWatch(assetDir.string(), s_Listener));
	assetDir = rootPath / "Script";
	if(fs::exists(assetDir))
		s_WatcherIDs.Add(
			s_FileWatcher->addWatch(assetDir.string(), s_Listener));

	s_FileWatcher->watch();

	auto assetPackNode = file["AssetPack"];
	if(!assetPackNode)
		return;

	for(auto assetNode : assetPackNode["Assets"]) {
		auto node = assetNode["Asset"];
		AssetType type = AssetTypeFromString(node["Type"].as<std::string>());
		UUID id = node["ID"].as<uint64_t>();
		std::string path;
		if(node["Path"]) {
			path = (rootPath / node["Path"].as<std::string>()).generic_string();
			if(!fs::exists(path))
				continue;
		}

		Asset asset = Add(type, id, true, path);
		if(node["Name"])
			m_AssetRegistry->NameAsset(asset, node["Name"].as<std::string>());
	}

	// New assets
	List<fs::path> paths
	{
		(rootPath / "Mesh"),
		(rootPath / "Image"),
		(rootPath / "Cubemap"),
		(rootPath / "Shader"),
		(rootPath / "Font"),
		(rootPath / "Audio"),
		(rootPath / "Script"),
	};
	u32 i = 1;
	for(auto& folder : paths) {
		for(auto path : FileUtils::GetFiles(folder.string())) {
			if(i == 1)
				path = FileUtils::GetFiles(path, { ".obj" })[0];
			if(!GetFromPath(path))
				Add((AssetType)i, 0, true, path);
		}
		i++;
	}

	// m_AssetRegistry[Asset{ 10012345, AssetType::Mesh }] = true;
	// m_MeshAssets[10012345] = Mesh::Create(MeshType::Cube);
	// m_AssetRegistry[Asset{ 10112345, AssetType::Mesh }] = true;
	// m_MeshAssets[10112345] = Mesh::Create(MeshType::Quad);
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
			// if(!asset.Primary || IsNativeAsset(asset))
			// 	continue;

			serializer.BeginMapping();
			serializer.WriteKey("Asset").BeginMapping();
			serializer.WriteKey("ID").Write((uint64_t)asset.ID);
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

}

namespace VolcaniCore {

template<>
BinaryWriter& BinaryWriter::WriteObject(const Asset& asset) {
	Write((uint64_t)asset.ID);
	Write((uint8_t)asset.Type);
	Write((bool)asset.Primary);

	auto reg = AssetManager::Get()->GetRegistry();
	if(reg->HasRefs(asset)) {
		const auto& refs = reg->GetRefs(asset);
		Write(refs.Count());
		for(const auto& ref : refs) {
			Write((uint64_t)ref.ID);
			Write((uint8_t)ref.Type);
		}
	}
	else
		Write((uint64_t)0);

	Write(reg->GetAssetName(asset));

	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const Vec4& vec) {
	WriteData(&vec.x, sizeof(Vec4));
	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const Mat2& mat) {
	WriteData(glm::value_ptr(mat), sizeof(Mat2));
	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const Mat3& mat) {
	WriteData(glm::value_ptr(mat), sizeof(Mat3));
	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const Mat4& mat) {
	WriteData(glm::value_ptr(mat), sizeof(Vec4));
	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const UUID& uuid) {
	Write((uint64_t)uuid);
	return *this;
}

}

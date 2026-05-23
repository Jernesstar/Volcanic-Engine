#include "AssetManager.h"

#include <iostream>
#include <bitset>

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
class ByteCodeWriter : public asIBinaryStream {
public:
	ByteCodeWriter(BytesWriter* writer)
		: m_Writer(writer) { }
	~ByteCodeWriter() = default;

	int Read(void* data, u32 size) override {
		return 1;
	}

	int Write(const void* data, u32 size) override {
		m_Writer->WriteData(data, (u64)size);
		return 0;
	}

private:
	BytesWriter* m_Writer = nullptr;
};

Str AssetTypeToString(AssetType type) {
	switch(type) {
		case AssetType::Geometry: return "Geometry";
		case AssetType::Material: return "Material";
		case AssetType::Model:    return "Model";
		case AssetType::Texture:  return "Texture";
		case AssetType::Cubemap:  return "Cubemap";
		case AssetType::Shader:   return "Shader";
		case AssetType::Font:     return "Font";
		case AssetType::Audio:    return "Audio";
		case AssetType::Script:   return "Script";
	}

	return "None";
}

AssetType AssetTypeFromString(const Str& str) {
	if(str == "Geometry")      return AssetType::Geometry;
	else if(str == "Material") return AssetType::Material;
	else if(str == "Model")    return AssetType::Model;
	else if(str == "Texture")  return AssetType::Texture;
	else if(str == "Cubemap")  return AssetType::Cubemap;
	else if(str == "Shader")   return AssetType::Shader;
	else if(str == "Font")     return AssetType::Font;
	else if(str == "Audio")    return AssetType::Audio;
	else if(str == "Script")   return AssetType::Script;

	return AssetType::None;
}

class FileWatcher : public efsw::FileWatchListener {
public:
	FileWatcher(EditorAssetManager* assetManager);
	~FileWatcher() = default;

	void handleFileAction(efsw::WatchID id, const Str& dir,
		const Str& file, efsw::Action action, Str old) override;

	u32 AddCallback(const Func<void, Asset, bool>& callback);
	void RemoveCallback(u32 id);

	void Add(AssetType type, const Str& path);
	void Remove(AssetType type, const Str& path);
	void Reload(AssetType type, const Str& path);

private:
	EditorAssetManager* m_AssetManager;
	List<Func<void, Asset, bool>> m_Callbacks;
};

FileWatcher::FileWatcher(EditorAssetManager* assetManager)
	: m_AssetManager(assetManager) { }

void FileWatcher::handleFileAction(
	efsw::WatchID id, const Str& dir,
	const Str& file, efsw::Action action, Str oldFilename)
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

void FileWatcher::Add(AssetType type, const Str& path) {
	Log::Info("Adding AssetType::{} at path '{}'",
				AssetTypeToString(type).c_str(), path);
	m_AssetManager->Add(type, 0, true, path);
}

void FileWatcher::Remove(AssetType type, const Str& path) {
	Log::Info("Removing AssetType::{} at path '{}'",
				AssetTypeToString(type), path);
	UUID id = m_AssetManager->GetFromPath(path);
	m_AssetManager->Remove({ id, type });
}

void FileWatcher::Reload(AssetType type, const Str& path) {
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

	std::string path = GetPath(asset.ID);
	if(asset.ID > 100) // Not a native asset
		path = fs::canonical(path).string();

	if(asset.Type == AssetType::Geometry) {
		Log::Info("Loading geometry {}", (u64)asset.ID);
		List<SubGeometry> surfaces;
		List<MaterialPaths> materialPaths;
		AssetImporter::GetGeometryData(path, surfaces, materialPaths);

		u64 size = sizeof(u64);
		for(auto& s : surfaces)
			size += s.Vertices.GetSize() + s.Indices.GetSize() + sizeof(u32);

		BytesWriter wr(size);
		wr.Write(surfaces.Count());
		for(auto& s : surfaces) {
			wr.Write(s.Vertices);
			wr.Write(s.Indices);
			wr.Write(s.SurfaceSlot);
		}
		m_AssetRegistry->SetData(asset, std::move(wr.Bytes));
	}
	else if(asset.Type == AssetType::Model) {
		// Lightweight: register child Geometry + Material assets,
		// serialize the ModelNode hierarchy as asset ID references only.
		// GPU data lives in the child Geometry assets, not here.
	}
	else if(asset.Type == AssetType::Texture) {
		ImageData image = AssetImporter::GetImageData(path, false);

		BytesWriter wr((sizeof(u32) * 4) + image.Data.GetSize());
		wr.Write(image.Width);
		wr.Write(image.Height);
		wr.Write(image.BPP);
		wr.Write(image.Data);

		m_AssetRegistry->SetData(asset, std::move(wr.Bytes));
	}
	else if(asset.Type == AssetType::Cubemap) {
		for(auto& ref : m_AssetRegistry->GetRefs(asset))
			Build(ref);
	}
	else if(asset.Type == AssetType::Shader) {
		if(!asset.Primary)
			return;

		auto refs = m_AssetRegistry->GetRefs(asset);

		List<ShaderFile> files;
		ShaderLayout layout;
		u64 size = sizeof(u64);
		for(auto& ref : refs) {
			auto shaderPath = GetPath(ref.ID);
			Log::Info("Shader path {} for {}", shaderPath, (u64)ref.ID);
			ShaderFile file = AssetImporter::GetShaderFileData(shaderPath);
			size += sizeof(u32) + file.Data.GetSize();
			Buffer<u32> data((u32*)file.Data.Get(), file.Data.GetSize(), 0, false);

			Log::Info("Reflecting shader");
			AssetImporter::ReflectShader(data, layout);

			files.AddMove(std::move(file));
		}

		BytesWriter wr(size + 1024); // Extra space for layout
		wr.Write((u64)files.Count());

		for(auto& file : files) {
			wr.Write((u32)file.FileType);
			wr.Write(file.Data);
		}

		wr.Write(layout);

		m_AssetRegistry->SetData(asset, std::move(wr.Bytes));
	}
	else if(asset.Type == AssetType::Audio) {
		Buffer<f32> soundData = AssetImporter::GetAudioData(path);
		BytesWriter wr(soundData.GetSize());
		wr.Write(soundData);
		m_AssetRegistry->SetData(asset, std::move(wr.Bytes));
	}
	else if(asset.Type == AssetType::Script) {
		auto* mod = ScriptManager::LoadScript(path, false);
		Str name = mod->GetName();

		BytesWriter wr(name.size() + std::numeric_limits<u16>::max());
		wr.Write(name);

		ByteCodeWriter stream(&wr);
		mod->SaveByteCode(&stream, true);

		m_AssetRegistry->SetData(asset, std::move(wr.Bytes));
	}
	else if(asset.Type == AssetType::Material) {
		
	}
}

u32 EditorAssetManager::AddReloadCallback(const Func<void, Asset, bool>& cb) {
	return s_Listener->AddCallback(cb);
}

void EditorAssetManager::RemoveReloadCallback(u32 id) {
	s_Listener->RemoveCallback(id);
}

Asset EditorAssetManager::Add(AssetType type, UUID id, bool primary,
							  const Str& path)
{
	Asset newAsset{ id ? id : UUID(), type, primary };
	if(path != "")
		m_Paths[newAsset.ID] = path;

	m_AssetRegistry->Add(newAsset);
	Build(newAsset);
	return newAsset;
}

void EditorAssetManager::Remove(Asset asset) {
	Unload(asset);

	m_AssetRegistry->Remove(asset);
	m_Paths.erase(asset.ID);
}

UUID EditorAssetManager::GetFromPath(const Str& path) const {
	for(auto& [id, assetPath] : m_Paths)
		if(fs::path(path) == fs::path(assetPath))
			return id;

	return 0;
}

Str EditorAssetManager::GetPath(UUID id) const {
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

	struct AssetFolder {
		fs::path Path;
		AssetType Type;
	};

	List<AssetFolder> folders {
		{ rootPath / "Geometry", AssetType::Geometry },
		{ rootPath / "Texture",  AssetType::Texture  },
		{ rootPath / "Cubemap",  AssetType::Cubemap  },
		{ rootPath / "Shader",   AssetType::Shader   },
		{ rootPath / "Material", AssetType::Material },
		{ rootPath / "Font",     AssetType::Font     },
		{ rootPath / "Audio",    AssetType::Audio    },
		{ rootPath / "Script",   AssetType::Script   },
		{ rootPath / "Model",    AssetType::Model    },
		{ rootPath / "Custom",   AssetType::Custom   },
	};

	for(auto& folder : folders) {
		if(fs::exists(folder.Path))
			s_WatcherIDs.Add(
				s_FileWatcher->addWatch(folder.Path.string(), s_Listener)
			);
	}

	s_FileWatcher->watch();
	Log::Info("Watching...");

	YAML::Node file;
	try {
		file = YAML::LoadFile(m_Path);
	}
	catch(YAML::ParserException e) {
		VOLCANICORE_ASSERT_ARGS(false, "Could not load file {}: {}",
								m_Path, e.what());
	}

	auto assetPackNode = file["AssetPack"];
	if(!assetPackNode)
		return;

	for(auto assetNode : assetPackNode["Assets"]) {
		auto node = assetNode["Asset"];
		AssetType type = AssetTypeFromString(node["Type"].as<Str>());
		UUID id = node["ID"].as<u64>();
		Str path;
		if(node["Path"]) {
			path = (
				folders[(u32)type - 1].Path / node["Path"].as<Str>()
			).generic_string();
			if(!fs::exists(path)) {
				Log::Warning("Asset path '{}' does not exist", path);
				continue;
			}
		}
		Log::Info("Adding asset {} {} {}", node["Type"].as<Str>(), (u64)id, path);

		Asset asset = Add(type, id, true, path);
		if(node["Name"])
			m_AssetRegistry->NameAsset(asset, node["Name"].as<Str>());
	}

	Log::Info("Loaded registered assets");

	for(auto& folder : folders) {
		for(auto& path : FileUtils::GetFiles(folder.Path.string()))
			if(!GetFromPath(path))
				Add(folder.Type, 0, true, path);
	}

	Log::Info("New assets");

	Application::PushDir();

	Asset asset;
	{
		asset = { 99, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Framebuffer.glsl.vert",
				"Editor/assets/Shaders/Framebuffer.glsl.frag"
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "FullscreenQuad");
	}

	{
		asset = { 100, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				"Editor/assets/Shaders/GBuffer.glsl.vert",
				"Editor/assets/Shaders/GBuffer.glsl.frag"
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "GBuffer");
	}

	{
		asset = { 101, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Shadow.glsl.vert",
				"Editor/assets/Shaders/Shadow.glsl.frag"
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "ShadowDepth");
	}

	{
		asset = { 102, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Framebuffer.glsl.vert",
				"Editor/assets/Shaders/DeferredLighting.glsl.frag"
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "DeferredLighting");
	}

	{
		asset = { 103, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				"Editor/assets/Shaders/BloomDownsample.glsl.comp",
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "BloomDownsample");
	}

	{
		asset = { 104, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				"Editor/assets/Shaders/BloomUpsample.glsl.comp"
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "BloomUpsample");
	}

	{
		asset = { 105, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Tonemap.glsl.vert",
				"Editor/assets/Shaders/Tonemap.glsl.frag",
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "Tonemap");
	}

	{
		asset = { 106, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Particle.glsl.vert",
				"Editor/assets/Shaders/Particle.glsl.frag"
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "Particle");
	}

	{
		asset = { 107, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				"Editor/assets/Shaders/ParticleEmitter.glsl.comp",
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "ParticleEmitter");
	}

	{
		asset = { 108, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				"Editor/assets/Shaders/ParticleUpdate.glsl.comp",
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "ParticleUpdate");
	}

	{
		asset = { 109, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Framebuffer.glsl.vert",
				"Editor/assets/Shaders/PixelUpscale.glsl.frag",
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "PixelUpscale");
	}

	Application::PopDir();

	{
		asset = { 109, AssetType::Geometry };
		m_GeometryAssets[asset.ID] =
			Geometry::Create(GeometryType::Cube);
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "Cube");
	}

	Log::Info("Default assets");
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

			Str path = GetPath(asset.ID);
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

void EditorAssetManager::Export(const Str& exportPath) {

}

}

namespace VolcaniCore {

template<>
BytesWriter& BytesWriter::WriteObject(const ShaderPropDeclaration& decl) {
	Write(decl.Name);
	Write((u8)decl.Type);
	Write(decl.Binding);
	Write(decl.Set);
	return *this;
}

template<>
BytesWriter& BytesWriter::WriteObject(const ShaderLayout& layout) {
	Write((u32)layout.Uniforms.Count());
	for(auto& decl : layout.Uniforms) Write(decl);
	Write((u32)layout.Samplers.Count());
	for(auto& decl : layout.Samplers) Write(decl);
	return *this;
}

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

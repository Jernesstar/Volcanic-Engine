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

// Mirrors ReadModelNode in AssetManager.h exactly:
// [transform: 9 floats] [geoID: u64] [geoType: u8] [matID: u64] [matType: u8]
// [childCount: u32] [children...]
static void WriteModelNode(BytesWriter& wr,
	const AssetImporter::ModelNodeData& node,
	const List<Asset>& geoAssets,
	const List<Asset>& matAssets)
{
	wr.WriteData(&node.LocalTransform, sizeof(Vec3) * 3);

	Asset geo = (node.GeometryIndex >= 0) ? geoAssets[node.GeometryIndex] : Asset{};
	Asset mat = (node.MaterialIndex >= 0) ? matAssets[node.MaterialIndex] : Asset{};
	wr.Write((u64)geo.ID); wr.Write((u8)geo.Type);
	wr.Write((u64)mat.ID); wr.Write((u8)mat.Type);
	wr.Write((u32)node.Children.Count());
	for(auto& child : node.Children)
		WriteModelNode(wr, child, geoAssets, matAssets);
}

// Shared MatProp encoder — the single writer counterpart to ReadMatPropValue in
// AssetManager.h. Format: [name] [type: u8] [value]. Texture value is written as
// an unresolved [asset id: u64] [asset type: u8], matching the reader. Used by
// both the model-material writer and the MaterialInstance writer so the on-disk
// prop encoding lives in exactly one place.
static void WriteMatProp(BytesWriter& wr, const Str& name, const MatProp& prop) {
	wr.Write(name);
	wr.Write((u8)prop.Type);
	std::visit([&](auto&& v) {
		using T = std::decay_t<decltype(v)>;
		if constexpr(std::is_same_v<T, i32>)  wr.Write(v);
		else if constexpr(std::is_same_v<T, f32>)  wr.Write(v);
		else if constexpr(std::is_same_v<T, Vec2>) wr.WriteData(&v.x, sizeof(Vec2));
		else if constexpr(std::is_same_v<T, Vec3>) wr.WriteData(&v.x, sizeof(Vec3));
		else if constexpr(std::is_same_v<T, Vec4>) wr.WriteData(&v.x, sizeof(Vec4));
		else if constexpr(std::is_same_v<T, Mat4>)
			wr.WriteData(glm::value_ptr(v), sizeof(Mat4));
		else if constexpr(std::is_same_v<T, Asset>) {
			wr.Write((u64)v.ID); wr.Write((u8)v.Type);
		}
		else if constexpr(std::is_same_v<T, Ref<Texture>>) {
			// A live texture has no asset id to persist; overrides authored for
			// serialization always reference assets. Write a null texture ref.
			wr.Write((u64)0); wr.Write((u8)AssetType::Texture);
		}
	}, prop.Value);
}

// MaterialInstance writer — counterpart to LoadFromBytes<MaterialInstance>.
// [parent id: u64] [parent type: u8] [override count: u32] (MatProp)*
static void WriteMaterialInstance(BytesWriter& wr, const MaterialInstance& inst) {
	wr.Write((u64)inst.ParentAsset.ID);
	wr.Write((u8)inst.ParentAsset.Type);
	wr.Write((u32)inst.Overrides.size());
	for(auto& [name, prop] : inst.Overrides)
		WriteMatProp(wr, name, prop);
}

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
	// Not a native asset, and has a source path. Shader-group primaries are
	// path-less (they build from their stage-file refs), so guard the empty case.
	if(asset.ID > 100 && path != "")
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
		List<SubGeometry>   surfaces;
		List<MaterialPaths> mats;
		AssetImporter::ModelNodeData root =
			AssetImporter::GetModelData(path, surfaces, mats);

		// Deterministic child UUIDs — stable across editor restarts.
		// Different primes per slot type to avoid ID collisions.
		auto geoChildID = [&](u32 i) -> UUID {
			return UUID(((u64)asset.ID * 6364136223846793005ULL) ^ (u64)(i + 1));
		};
		auto matChildID = [&](u32 i) -> UUID {
			return UUID(((u64)asset.ID * 2862933555777941757ULL) ^ (u64)(i + 1));
		};
		auto texChildID = [&](u32 i) -> UUID {
			return UUID(((u64)asset.ID * 1442695040888963407ULL) ^ (u64)(i + 1));
		};

		// ── Geometry child assets ────────────────────────────────────────────
		List<Asset> geoAssets;
		geoAssets.Allocate(surfaces.Count());
		for(u32 i = 0; i < (u32)surfaces.Count(); i++) {
			auto& surf = surfaces[i];
			Asset geoAsset{ geoChildID(i), AssetType::Geometry, false };
			m_AssetRegistry->Add(geoAsset);

			// One surface per decomposed geometry; normalize slot to 0 so
			// MeshComponent::GetMaterialForSlot(0) resolves correctly.
			BytesWriter wr(sizeof(u64)
				+ surf.Vertices.GetSize()
				+ surf.Indices.GetSize()
				+ sizeof(u32));
			wr.Write((u64)1);
			wr.Write(surf.Vertices);
			wr.Write(surf.Indices);
			wr.Write((u32)0);
			m_AssetRegistry->SetData(geoAsset, std::move(wr.Bytes));
			geoAssets.Add(geoAsset);
		}

		// ── Material child assets ────────────────────────────────────────────
		// All model-derived materials reference the native GBuffer shader. This is
		// the same sentinel a null-shader Material resolves to at bind time — see
		// Graphics::k_DefaultGBufferShaderID and the MaterialBinder contract.
		Asset gbufShader{ Graphics::k_DefaultGBufferShaderID, AssetType::Shader };

		List<Asset> matAssets;
		matAssets.Allocate(mats.Count());
		for(u32 i = 0; i < (u32)mats.Count(); i++) {
			auto& mp = mats[i];
			Asset matAsset{ matChildID(i), AssetType::Material, false };
			m_AssetRegistry->Add(matAsset);

			// Optional diffuse texture
			Asset diffTex{};
			if(!mp.Diffuse.empty() && fs::exists(mp.Diffuse)) {
				diffTex = { texChildID(i), AssetType::Texture, false };
				m_AssetRegistry->Add(diffTex);

				ImageData img = AssetImporter::GetImageData(mp.Diffuse, false);
				BytesWriter texWr(sizeof(u32) * 3 + img.Data.GetSize());
				texWr.Write(img.Width);
				texWr.Write(img.Height);
				texWr.Write(img.BPP);
				texWr.Write(img.Data);
				m_AssetRegistry->SetData(diffTex, std::move(texWr.Bytes));
			}

			u32 propCount = diffTex ? 2u : 1u;
			BytesWriter matWr(256);
			matWr.Write((u64)gbufShader.ID);
			matWr.Write((u8)gbufShader.Type);
			matWr.Write(propCount);

			// DiffuseColor — always present. Shares the prop encoder with the
			// MaterialInstance writer so material and override props round-trip
			// identically.
			WriteMatProp(matWr, "DiffuseColor",
				{ ShaderPropType::Vec4, mp.DiffuseColor });

			// DiffuseMap — only when a texture was found
			if(diffTex)
				WriteMatProp(matWr, "DiffuseMap",
					{ ShaderPropType::Texture, diffTex });

			m_AssetRegistry->SetData(matAsset, std::move(matWr.Bytes));
			matAssets.Add(matAsset);
		}

		// ── ModelNode hierarchy ──────────────────────────────────────────────
		BytesWriter modelWr(2048);
		WriteModelNode(modelWr, root, geoAssets, matAssets);
		m_AssetRegistry->SetData(asset, std::move(modelWr.Bytes));
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

		// A game-supplied shader that fails to compile must not take down the
		// Editor: glslang/SPIRV-Cross paths can assert or throw on bad source.
		// Log and leave the asset unbuilt so the rest of the project still loads.
		try {
			List<ShaderFile> files;
			ShaderLayout layout;
			u64 size = sizeof(u64);
			for(auto& ref : refs) {
				auto shaderPath = GetPath(ref.ID);
				Log::Info("Shader path {} for {}", shaderPath, (u64)ref.ID);
				ShaderFile file = AssetImporter::GetShaderFileData(shaderPath);
				size += sizeof(u32) + file.Data.GetSize();

				// A failed glslang compile returns an empty SPIR-V buffer (the
				// failure is already logged by GetShaderData). Reflecting an empty
				// buffer throws "0 words" — surface a clear error instead.
				if(!file.Data.GetCount()) {
					Log::Error("Shader stage '{}' produced no SPIR-V; skipping "
						"shader group {}.", shaderPath, (u64)asset.ID);
					return;
				}

				// Buffer view over the SPIR-V words: second arg is the word COUNT.
				Buffer<u32> data((u32*)file.Data.Get(), file.Data.GetCount(),
								 0, false);

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
		catch(const std::exception& e) {
			Log::Error("Failed to build shader {}: {}", (u64)asset.ID, e.what());
		}
		catch(...) {
			Log::Error("Failed to build shader {}: unknown error", (u64)asset.ID);
		}
	}
	else if(asset.Type == AssetType::Audio) {
		// Header [sampleRate f32][channels u32] precedes the planar sample data
		// so playback uses the source's true rate/channel count (the previous
		// hardcoded 44.1kHz-mono replay distorted speed and dropped channels).
		f32 sampleRate;
		u32 channels;
		Buffer<f32> soundData =
			AssetImporter::GetAudioData(path, sampleRate, channels);
		BytesWriter wr(sizeof(f32) + sizeof(u32) + soundData.GetSize());
		wr.Write(sampleRate);
		wr.Write(channels);
		wr.Write(soundData);
		m_AssetRegistry->SetData(asset, std::move(wr.Bytes));
	}
	else if(asset.Type == AssetType::Script) {
		auto* mod = ScriptManager::LoadScript(path, false);
		if(!mod) {
			Log::Error("Failed to compile script '{}'", path);
			return;
		}
		Str name = mod->GetName();

		BytesWriter wr(name.size() + std::numeric_limits<u16>::max());
		wr.Write(name);

		ByteCodeWriter stream(&wr);
		mod->SaveByteCode(&stream, true);

		m_AssetRegistry->SetData(asset, std::move(wr.Bytes));
	}
	else if(asset.Type == AssetType::Material) {
		// Standalone .mat builder. Parses a YAML material into the same binary
		// Material blob format the model-import path writes (and LoadFromBytes<
		// Material> reads): [shader id:u64] [shader type:u8] [prop count:u32]
		// then (name, type, value)* via the shared WriteMatProp encoder.
		//
		// .mat schema (all blocks optional):
		//   Shader: <asset name>            resolved by name; unknown → GBuffer
		//   TextureUniforms: [{ Uniform: { Name, Ref: <texture asset name> } }]
		//   Props:           [{ Prop: { Name, Type: Float|Vec3|Vec4|Int, Data } }]
		//   Vec4Uniforms/Vec3Uniforms/FloatUniforms/IntUniforms (legacy typed
		//     blocks with { Uniform: { Name, Data } }) are also accepted so older
		//     materials round-trip.
		YAML::Node file;
		try {
			file = YAML::LoadFile(path);
		}
		catch(const std::exception& e) {
			Log::Error("Failed to parse material '{}': {}", path, e.what());
			return;
		}

		YAML::Node matNode = file["Material"];
		if(!matNode) {
			Log::Error("Material '{}' has no 'Material' node", path);
			return;
		}

		// ── Shader: resolve by name (guarded), else the default G-Buffer shader ──
		Asset shaderAsset{ Graphics::k_DefaultGBufferShaderID, AssetType::Shader };
		if(auto shaderName = matNode["Shader"]) {
			Str name = shaderName.as<Str>();
			Asset found = FindAssetByName(name);
			if(found && found.Type == AssetType::Shader)
				shaderAsset = found;
			else
				Log::Warning("Material '{}' references unknown shader '{}'; "
					"falling back to the default G-Buffer shader.", path, name);
		}

		// Collect props first so we can write an accurate count.
		List<Pair<Str, MatProp>> props;

		// ── TextureUniforms: { Name, Ref: <texture asset name> } ────────────────
		if(auto texNode = matNode["TextureUniforms"]) {
			for(auto item : texNode) {
				auto u = item["Uniform"];
				if(!u || !u["Name"] || !u["Ref"])
					continue;
				Str name = u["Name"].as<Str>();
				Str ref = u["Ref"].as<Str>();
				Asset tex = FindAssetByName(ref);
				if(!tex) {
					Log::Warning("Material '{}' texture uniform '{}' references "
						"unknown asset '{}'; skipping.", path, name, ref);
					continue;
				}
				props.Emplace(name, MatProp{ ShaderPropType::Texture, tex });
			}
		}

		// ── Generic Props block: { Name, Type: Float|Vec3|Vec4|Int, Data } ──────
		if(auto propsNode = matNode["Props"]) {
			for(auto item : propsNode) {
				auto p = item["Prop"];
				if(!p || !p["Name"] || !p["Type"] || !p["Data"])
					continue;
				Str name = p["Name"].as<Str>();
				Str type = p["Type"].as<Str>();
				if(type == "Float")
					props.Emplace(name,
						MatProp{ ShaderPropType::Float, p["Data"].as<f32>() });
				else if(type == "Int")
					props.Emplace(name,
						MatProp{ ShaderPropType::Int, p["Data"].as<i32>() });
				else if(type == "Vec3")
					props.Emplace(name,
						MatProp{ ShaderPropType::Vec3, p["Data"].as<Vec3>() });
				else if(type == "Vec4")
					props.Emplace(name,
						MatProp{ ShaderPropType::Vec4, p["Data"].as<Vec4>() });
				else
					Log::Warning("Material '{}' prop '{}' has unsupported type "
						"'{}'; skipping.", path, name, type);
			}
		}

		// ── Legacy typed uniform blocks: { Name, Data } ─────────────────────────
		auto readTyped = [&](const char* block, ShaderPropType type) {
			auto node = matNode[block];
			if(!node) return;
			for(auto item : node) {
				auto u = item["Uniform"];
				if(!u || !u["Name"] || !u["Data"])
					continue;
				Str name = u["Name"].as<Str>();
				MatProp prop; prop.Type = type;
				switch(type) {
					case ShaderPropType::Float: prop.Value = u["Data"].as<f32>(); break;
					case ShaderPropType::Int:   prop.Value = u["Data"].as<i32>(); break;
					case ShaderPropType::Vec3:  prop.Value = u["Data"].as<Vec3>(); break;
					case ShaderPropType::Vec4:  prop.Value = u["Data"].as<Vec4>(); break;
					default: continue;
				}
				props.Emplace(name, prop);
			}
		};
		readTyped("FloatUniforms", ShaderPropType::Float);
		readTyped("IntUniforms",   ShaderPropType::Int);
		readTyped("Vec3Uniforms",  ShaderPropType::Vec3);
		readTyped("Vec4Uniforms",  ShaderPropType::Vec4);

		BytesWriter matWr(256);
		matWr.Write((u64)shaderAsset.ID);
		matWr.Write((u8)shaderAsset.Type);
		matWr.Write((u32)props.Count());
		for(auto& [name, prop] : props)
			WriteMatProp(matWr, name, prop);

		m_AssetRegistry->SetData(asset, std::move(matWr.Bytes));
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

Asset EditorAssetManager::FindAssetByName(const std::string& name) const {
	// AssetRegistry::FindAsset uses map::at and throws on unknown names; guard it.
	auto reg = m_AssetRegistry;
	Asset found;
	reg->For([&](Asset asset) {
		if(found) return;
		if(reg->GetAssetName(asset) == name)
			found = asset;
	});
	return found;
}

void EditorAssetManager::RegisterShaderGroups(const fs::path& shaderFolder) {
	if(!fs::exists(shaderFolder))
		return;

	// Group stage files by double-stem: Foo.glsl.vert / Foo.glsl.frag → "Foo".
	Map<Str, List<Str>> groups;
	for(auto& path : FileUtils::GetFiles(shaderFolder.string(),
			{ ".vert", ".frag", ".geom", ".comp" }))
	{
		Str group = fs::path(path).stem().stem().string();
		groups[group].Add(path);
	}

	for(auto& [group, stageFiles] : groups) {
		// Deterministic, path-independent UUID from the group name so the primary
		// keeps a stable ID across restarts. High bit set keeps it clear of the
		// native asset range (<= 200) and the hashed-child ID space is disjoint.
		UUID primaryID(
			(std::hash<Str>{}(group) | 0x4000000000000000ULL));
		Asset primary{ primaryID, AssetType::Shader, true };

		// A re-derived group may already be registered from a previous scan in the
		// same session (e.g. after a folder-watch reload) — rebuild it in place.
		m_AssetRegistry->Add(primary);
		m_AssetRegistry->NameAsset(primary, group);

		// Stage files as non-primary refs; Build's Shader branch reads their SPIR-V.
		u32 i = 0;
		for(auto& stagePath : stageFiles) {
			UUID stageID(
				((u64)primaryID * 6364136223846793005ULL) ^ (u64)(++i));
			Asset stage{ stageID, AssetType::Shader, false };
			m_Paths[stageID] = stagePath;
			m_AssetRegistry->Add(stage);
			m_AssetRegistry->AddRef(primary, stage);
		}

		Log::Info("Registering shader group '{}' ({} stage file(s))",
			group, stageFiles.Count());
		Build(primary);
	}
}

void EditorAssetManager::LoadRegistry(const std::string& projectRoot) {
	m_AssetRegistry = CreateRef<AssetRegistry>(projectRoot);

	auto rootPath = fs::path(projectRoot) / "Asset";
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

	// A Material Build resolves its `Shader: <name>` reference against the
	// registry, so every shader group must be registered first. Shader primaries
	// are re-derived from the Shader folder each startup (below), which runs after
	// this registry pass — so defer material builds until then.
	List<Asset> deferredMaterialBuilds;

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

		// Shader primaries are path-less group descriptors re-derived from the
		// Shader folder on every startup — never persisted meaningfully. Skip any
		// that leaked into the assetpk so the folder-group pass owns them.
		if(type == AssetType::Shader && path == "")
			continue;

		Log::Info("Adding asset {} {} {}", node["Type"].as<Str>(), (u64)id, path);

		Asset asset = Add(type, id, true, path);
		if(node["Name"])
			m_AssetRegistry->NameAsset(asset, node["Name"].as<Str>());

		// Script bytecode is tied to the exact registered AngelScript interface;
		// any engine binding change invalidates cached bytecode (LoadByteCode
		// then fails). Recompile scripts from source at load so they always
		// match the current interface.
		if(type == AssetType::Script && path != "")
			Build(asset);
		// Materials resolve a shader by name; defer until shader groups exist.
		else if(type == AssetType::Material && path != ""
				&& !fs::exists(m_AssetRegistry->GetBinPath(id)))
			deferredMaterialBuilds.Add(asset);
		// For every other asset the .bin blob is a persistent cache: rebuild it
		// from source when it is missing, so a stale/corrupt blob can be fixed
		// by deleting the file. (Models also regenerate their child geometry/
		// material/texture blobs here.)
		else if(path != "" && !fs::exists(m_AssetRegistry->GetBinPath(id)))
			Build(asset);
	}

	Log::Info("Loaded registered assets");

	for(auto& folder : folders) {
		// Shader folders are grouped by double-stem (e.g. Foo.glsl.vert +
		// Foo.glsl.frag → one "Foo" shader), so each stage file cannot be a
		// standalone primary. RegisterShaderGroups owns the whole folder.
		if(folder.Type == AssetType::Shader) {
			RegisterShaderGroups(folder.Path);
			continue;
		}

		for(auto& path : FileUtils::GetFiles(folder.Path.string())) {
			if(folder.Type == AssetType::Model)
				path = FileUtils::GetFiles(path, { ".obj" })[0];
			if(!GetFromPath(path)) {
				auto newAsset = Add(folder.Type, 0, true, path);
				Build(newAsset);
			}
		}
	}

	// Shader groups now exist — build the deferred materials so their
	// `Shader: <name>` references resolve.
	for(auto& mat : deferredMaterialBuilds)
		Build(mat);

	Log::Info("New assets");

	auto editorShader = [](const char* relativePath) {
		return (fs::path(Application::GetLibraryDir()) / relativePath).string();
	};

	Asset asset;
	{
		asset = { 99, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				editorShader("Editor/assets/Shaders/Framebuffer.glsl.vert"),
				editorShader("Editor/assets/Shaders/Framebuffer.glsl.frag")
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "FullscreenQuad");
	}

	{
		asset = { 100, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				editorShader("Editor/assets/Shaders/GBuffer.glsl.vert"),
				editorShader("Editor/assets/Shaders/GBuffer.glsl.frag")
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "GBuffer");
	}

	{
		asset = { 101, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				editorShader("Editor/assets/Shaders/Shadow.glsl.vert"),
				editorShader("Editor/assets/Shaders/Shadow.glsl.frag")
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "ShadowDepth");
	}

	{
		asset = { 102, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				editorShader("Editor/assets/Shaders/Framebuffer.glsl.vert"),
				editorShader("Editor/assets/Shaders/DeferredLighting.glsl.frag")
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "DeferredLighting");
	}

	{
		asset = { 103, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				editorShader("Editor/assets/Shaders/BloomDownsample.glsl.comp"),
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "BloomDownsample");
	}

	{
		asset = { 104, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				editorShader("Editor/assets/Shaders/BloomUpsample.glsl.comp")
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "BloomUpsample");
	}

	{
		asset = { 105, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				editorShader("Editor/assets/Shaders/Tonemap.glsl.vert"),
				editorShader("Editor/assets/Shaders/Tonemap.glsl.frag"),
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "Tonemap");
	}

	{
		asset = { 106, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				editorShader("Editor/assets/Shaders/Particle.glsl.vert"),
				editorShader("Editor/assets/Shaders/Particle.glsl.frag")
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "Particle");
	}

	{
		asset = { 107, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				editorShader("Editor/assets/Shaders/ParticleEmitter.glsl.comp"),
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "ParticleEmitter");
	}

	{
		asset = { 108, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				editorShader("Editor/assets/Shaders/ParticleUpdate.glsl.comp"),
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "ParticleUpdate");
	}

	{
		asset = { 109, AssetType::Shader };
		m_ShaderAssets[asset.ID] =
			AssetImporter::GetShader({
				editorShader("Editor/assets/Shaders/Framebuffer.glsl.vert"),
				editorShader("Editor/assets/Shaders/PixelUpscale.glsl.frag"),
			});
		m_LoadedAssets[asset.ID] = true;
		m_AssetRegistry->NameAsset(asset, "PixelUpscale");
	}

	{
		asset = { 110, AssetType::Geometry };
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

#include "AssetManager.h"

#include <bitset>

#include <VolcaniCore/Core/Math.h>

#include <Engine/Core/BinaryReader.h>

namespace VolcanicRuntime {

RuntimeAssetManager::RuntimeAssetManager() {

}

RuntimeAssetManager::~RuntimeAssetManager() {

}

}

namespace VolcanicRuntime {

template<>
BinaryReader& BinaryReader::ReadObject(Asset& asset) {
	uint64_t id;
	uint8_t type;
	bool primary;
	Read(id);
	Read(type);
	Read(primary);
	asset = { id, (AssetType)type, primary };

	uint64_t refCount;
	Read(refCount);
	for(uint64_t i = 0; i < refCount; i++) {
		Read(id);
		Read(type);
		Asset ref = { id, (AssetType)type, false };
		AssetManager::Get()->AddRef(asset, ref);
	}

	std::string name;
	Read(name);
	if(name != "")
		AssetManager::Get()->NameAsset(asset, name);

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(SubMesh& mesh) {
	Read(mesh.Vertices);
	Read(mesh.Indices);
	Read(mesh.MaterialIndex);

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(Mat2& mat) {
	ReadData(glm::value_ptr(mat), sizeof(Mat2));
	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(Mat3& mat) {
	ReadData(glm::value_ptr(mat), sizeof(Mat3));
	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(Mat4& mat) {
	ReadData(glm::value_ptr(mat), sizeof(Vec4));
	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(UUID& uuid) {
	uint64_t id;
	Read(id);
	uuid = id;
	return *this;
}

}

namespace VolcanicRuntime {

void RuntimeAssetManager::Load(Asset asset) {
	if(!IsValid(asset) || IsLoaded(asset) || IsNativeAsset(asset))
		return;

	m_AssetRegistry[asset] = true;
	BinaryReader pack("./.volc.assetpk");
	pack.SetPosition(m_AssetOffsets[asset.ID]);

	if(asset.Type == AssetType::Mesh) {
		Ref<Mesh> mesh = CreateRef<Mesh>(MeshType::Model);
		pack.Read(mesh->SubMeshes);

		for(auto& materialRef : GetRefs(asset)) {
			Load(materialRef);
			auto material = Get<VolcanicEngine::Material>(materialRef);
			VolcaniCore::Material& mat = mesh->Materials.Emplace();

			if(material->TextureUniforms.count("u_Diffuse")) {
				UUID id = material->TextureUniforms["u_Diffuse"];
				Asset textureAsset = { id, AssetType::Texture };
				Load(textureAsset);
				mat.Diffuse = Get<Texture>(textureAsset);
			}

			if(material->TextureUniforms.count("u_Specular")) {
				UUID id = material->TextureUniforms["u_Specular"];
				Asset textureAsset = { id, AssetType::Texture };
				Load(textureAsset);
				mat.Specular = Get<Texture>(textureAsset);
			}

			if(material->TextureUniforms.count("u_Emissive")) {
				UUID id = material->TextureUniforms.count("u_Emissive");
				Asset textureAsset = { id, AssetType::Texture };
				Load(textureAsset);
				mat.Emissive = Get<Texture>(textureAsset);
			}

			if(material->Vec4Uniforms.count("u_DiffuseColor"))
				mat.DiffuseColor = material->Vec4Uniforms["u_DiffuseColor"];
			if(material->Vec4Uniforms.count("u_SpecularColor"))
				mat.SpecularColor = material->Vec4Uniforms["u_SpecularColor"];
			if(material->Vec4Uniforms.count("u_EmissiveColor"))
				mat.EmissiveColor = material->Vec4Uniforms["u_EmissiveColor"];
		}

		m_MeshAssets[asset.ID] = mesh;
	}
	else if(asset.Type == AssetType::Texture) {
		uint32_t width, height;
		pack.Read(width);
		pack.Read(height);
		Buffer<uint8_t> data(width * height * 4);
		pack.ReadData(data.Get(), data.GetMaxCount());

		Ref<Texture> texture = Texture::Create(width, height);
		texture->SetData(data);
		m_TextureAssets[asset.ID] = texture;
	}
	else if(asset.Type == AssetType::Cubemap) {
		uint32_t test;
		pack.Read(test);
		VOLCANICORE_ASSERT(test == 4);

	}
	else if(asset.Type == AssetType::Shader) {
		// Mirrors the Editor Shader Build output and LoadFromBytes<Shader>:
		// [file count:u64] [(file type:u32, data:Buffer<u32>)*] [ShaderLayout].
		// (Runtime module is stale/best-effort; kept symmetric with the engine.)
		u64 fileCount;
		pack.Read(fileCount);

		List<Graphics::ShaderFile> files;
		files.Allocate(fileCount);
		for(u64 i = 0; i < fileCount; i++) {
			u32 fileType;
			pack.Read(fileType);
			Buffer<u32> data;
			pack.Read(data);
			files.AddMove(
				{ (Graphics::ShaderFileType)fileType, std::move(data) });
		}

		Graphics::ShaderLayout layout;
		u32 uniformCount;
		pack.Read(uniformCount);
		layout.Uniforms.Allocate(uniformCount);
		for(u32 i = 0; i < uniformCount; i++) {
			Graphics::ShaderPropDeclaration decl;
			u8 type;
			pack.Read(decl.Name);
			pack.Read(type); decl.Type = (Graphics::ShaderPropType)type;
			pack.Read(decl.Binding);
			pack.Read(decl.Set);
			layout.Uniforms.Add(decl);
		}
		u32 samplerCount;
		pack.Read(samplerCount);
		layout.Samplers.Allocate(samplerCount);
		for(u32 i = 0; i < samplerCount; i++) {
			Graphics::ShaderPropDeclaration decl;
			u8 type;
			pack.Read(decl.Name);
			pack.Read(type); decl.Type = (Graphics::ShaderPropType)type;
			pack.Read(decl.Binding);
			pack.Read(decl.Set);
			layout.Samplers.Add(decl);
		}

		auto shader = RendererAPI::Get()->CreateShader({});
		shader->SetShaderData(std::move(files), layout);
		m_ShaderAssets[asset.ID] = shader;
	}
	else if(asset.Type == AssetType::Audio) {
		Buffer<float> data;
		pack.Read(data);

		Ref<Sound> sound = CreateRef<Sound>();
		bool success =
			sound->GetInternal()
			.loadRawWave(data.Get(), data.GetCount(), 44100.0f, 1, true, false);
		VOLCANICORE_ASSERT(success == 0);

		m_AudioAssets[asset.ID] = sound;
	}
	else if(asset.Type == AssetType::Script) {
		std::string name;
		pack.Read(name);

		auto handle =
			ScriptEngine::Get()->GetModule(name.c_str(), asGM_ALWAYS_CREATE);
		ByteCodeReader stream(&pack);
		handle->LoadByteCode(&stream);
		m_ScriptAssets[asset.ID] = CreateRef<ScriptModule>(handle);
	}
	else if(asset.Type == AssetType::Material) {
		auto mat = CreateRef<VolcanicEngine::Material>();
		pack.Read(mat->IntUniforms);
		pack.Read(mat->FloatUniforms);
		pack.Read(mat->Vec2Uniforms);
		pack.Read(mat->Vec3Uniforms);
		pack.Read(mat->Vec4Uniforms);
		pack.Read(mat->Mat2Uniforms);
		pack.Read(mat->Mat3Uniforms);
		pack.Read(mat->Mat4Uniforms);
		pack.Read(mat->TextureUniforms);

		m_MaterialAssets[asset.ID] = mat;
	}
}

void RuntimeAssetManager::Unload(Asset asset) {
	if(!IsValid(asset) || !IsLoaded(asset) || IsNativeAsset(asset))
		return;

	m_AssetRegistry[asset] = false;

	if(asset.Type == AssetType::Mesh)
		m_MeshAssets.erase(asset.ID);
	else if(asset.Type == AssetType::Texture)
		m_TextureAssets.erase(asset.ID);
	else if(asset.Type == AssetType::Cubemap)
		m_CubemapAssets.erase(asset.ID);
	else if(asset.Type == AssetType::Shader)
		m_ShaderAssets.erase(asset.ID);
	else if(asset.Type == AssetType::Audio)
		m_AudioAssets.erase(asset.ID);
	else if(asset.Type == AssetType::Script)
		m_ScriptAssets.erase(asset.ID);
}

void RuntimeAssetManager::Load() {
	BinaryReader pack("./.volc.assetpk");

	std::string header;
	pack.Read(header);
	VOLCANICORE_ASSERT(header == "VOLC_PACK");

	uint64_t count;
	pack.Read(count);
	for(uint64_t i = 0; i < count; i++) {
		Asset asset;
		pack.Read(asset);

		uint64_t offset;
		pack.Read(offset);
		m_AssetRegistry[asset] = false;
		m_AssetOffsets[asset.ID] = offset;
	}
}

}
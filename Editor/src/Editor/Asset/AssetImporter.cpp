#include "AssetImporter.h"

#include <glad/glad.h>

#include <stb_image/stb_image.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/Logger.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <soloud.h>
#include <soloud_wav.h>

#include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Core/Log.h>
#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/FileUtils.h>

#include <Engine/Graphics/Mesh.h>
#include <Engine/Graphics/Platform/RendererAPI.h>

#include "ScriptManager.h"

using namespace VolcaniCore;
using namespace VolcanicEngine::Audio;
using namespace VolcanicEngine::Graphics;
using namespace VolcanicEngine::Script;

namespace fs = std::filesystem;

namespace VolcanicEditor {

ImageData AssetImporter::GetImageData(const std::string& path, bool flip) {
	stbi_set_flip_vertically_on_load((i32)flip);
	ImageData image;
	i32 width, height, bpp;
	u8* pixels = stbi_load(path.c_str(), &width, &height, &bpp, 4);
	if(!pixels) {
		Log::Warning("Could not load image '{}'", path.c_str());
		return { };
	}

	image.Width = (u32)width;
	image.Height = (u32)height;
	image.BPP = (u32)bpp;
	image.Data = Buffer(pixels, image.Width * image.Height * bpp);
	return image;
}

Ref<Texture> AssetImporter::GetTexture(const std::string& path, bool flip) {
	ImageData image = GetImageData(path, flip);
	auto texture = 
		RendererAPI::Get()->CreateTexture({ image.Width, image.Height });
	texture->SetData(image.Data);
	return texture;
}

// Ref<Cubemap> AssetImporter::GetCubemap(const std::string& folder) {
// 	List<fs::path> paths;
// 	for(auto path : FileUtils::GetFiles(folder, { ".png", ".jpg", ".jpeg" }))
// 		paths.Add(fs::path(path));

// 	if(paths.Count() < 6)
// 		Log::Warning(
// 			"Folder %s does not have at least 6 images", folder.c_str());

// 	Map<std::string, i32> map =
// 	{
// 		{ "right", 0 }, { "left",	1 },
// 		{ "top",   2 }, { "bottom", 3 },
// 		{ "front", 4 }, { "back",	5 }
// 	};
// 	List<ImageData> output(6);
// 	for(u32 i = 0; i < 6; i++)
// 		output.Emplace();

// 	for(auto& path : paths)
// 		output[map[path.filename().string()]] = GetImageData(path.string());

// 	return Cubemap::Create(output);
// }

static SubMesh LoadMesh(const aiMesh* mesh) {
	Buffer<Vertex> vertices(mesh->mNumVertices);
	Buffer<u32> indices(mesh->mNumFaces * 3);

	for(u32 i = 0; i < mesh->mNumVertices; i++) {
		const aiVector3D& pos	   = mesh->mVertices[i];
		const aiVector3D& normal   = mesh->mNormals[i];
		const aiVector3D& texCoord =
			mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i]
									  : aiVector3D(0.0f, 0.0f, 0.0f);
		Vertex v
		{
			.Position = glm::vec3(pos.x, pos.y, pos.z),
			.Normal   = glm::vec3(normal.x, normal.y, normal.z),
			.TexCoord = glm::vec2(texCoord.x, texCoord.y)
		};
		vertices.Add(v);
	}

	for(u32 i = 0; i < mesh->mNumFaces; i++) {
		const aiFace& face = mesh->mFaces[i];
		indices.Add(face.mIndices[0]);
		indices.Add(face.mIndices[1]);
		indices.Add(face.mIndices[2]);
	}

	return { std::move(vertices), std::move(indices), mesh->mMaterialIndex };
}

static std::string GetMaterialPath(const std::string& dir,
	const aiMaterial* material, aiTextureType type)
{
	if(material->GetTextureCount(type) == 0)
		return "";
	aiString filename;
	if(material->GetTexture(type, 0, &filename) == AI_FAILURE)
		return "";

	auto fullPath = fs::path(dir) / std::string(filename.data);
	return fullPath.string();
}

static Ref<Texture> LoadTexture(const std::string& dir,
	const aiMaterial* material, aiTextureType type)
{
	auto path = GetMaterialPath(dir, material, type);
	if(path == "")
		return nullptr;
	return AssetImporter::GetTexture(path, false);
}

// static MeshMaterial LoadMaterial(const std::string& dir, const aiMaterial* mat) {
// 	glm::vec4 diffuse = glm::vec4(0.0f);
// 	glm::vec4 specular = glm::vec4(0.0f);
// 	glm::vec4 emissive = glm::vec4(0.0f);

// 	aiColor4D color;
// 	if(mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
// 		diffuse = glm::vec4(color.r, color.g, color.b, color.a);
// 	if(mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
// 		specular = glm::vec4(color.r, color.g, color.b, color.a);
// 	if(mat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS)
// 		emissive = glm::vec4(color.r, color.g, color.b, color.a);

// 	return Material{
// 		.Diffuse  = LoadTexture(dir, mat, aiTextureType_DIFFUSE),
// 		.Specular = LoadTexture(dir, mat, aiTextureType_SPECULAR),
// 		.Emissive = LoadTexture(dir, mat, aiTextureType_EMISSIVE),
// 		// .Roughness = LoadTexture(dir, mat, aiTextureType_DIFFUSE_ROUGHNESS)

// 		.DiffuseColor  = diffuse,
// 		.SpecularColor = specular,
// 		.EmissiveColor = emissive
// 	};
// }

Ref<Mesh> AssetImporter::GetMesh(const std::string& path) {
	Assimp::Importer importer;
	u32 loadFlags = aiProcess_Triangulate
						| aiProcess_GenSmoothNormals
						| aiProcess_FlipUVs
						| aiProcess_JoinIdenticalVertices;
	const aiScene* scene = importer.ReadFile(path.c_str(), loadFlags);

	VOLCANICORE_ASSERT_ARGS(scene, "Error importing mesh from %s: %s",
							path.c_str(), importer.GetErrorString());

	Ref<Mesh> mesh = CreateRef<Mesh>(MeshType::Model);
	mesh->SubMeshes.Allocate(scene->mNumMeshes);
	// mesh->Materials.Allocate(scene->mNumMaterials);

	for(u32 i = 0; i < scene->mNumMeshes; i++)
		mesh->SubMeshes.AddMove(LoadMesh(scene->mMeshes[i]));

	auto dir = (fs::path(path).parent_path() / "textures").string();
	// for(u32 i = 0; i < scene->mNumMaterials; i++)
	// 	mesh->Materials.Add(LoadMaterial(dir, scene->mMaterials[i]));

	return mesh;
}

void AssetImporter::GetMeshData(const std::string& path,
	List<SubMesh>& meshes, List<MaterialPaths>& materialPaths)
{
	Assimp::Importer importer;
	u32 loadFlags = aiProcess_Triangulate
						| aiProcess_GenSmoothNormals
						| aiProcess_FlipUVs
						| aiProcess_JoinIdenticalVertices;
	const aiScene* scene = importer.ReadFile(path.c_str(), loadFlags);

	VOLCANICORE_ASSERT_ARGS(scene, "Error importing mesh from %s: %s",
							path.c_str(), importer.GetErrorString());

	meshes.Allocate(scene->mNumMeshes);
	materialPaths.Allocate(scene->mNumMaterials);

	for(u32 i = 0; i < scene->mNumMeshes; i++)
		meshes.AddMove(LoadMesh(scene->mMeshes[i]));

	auto dir = (fs::path(path).parent_path() / "textures").string();
	for(u32 i = 0; i < scene->mNumMaterials; i++) {
		auto mat = scene->mMaterials[i];
		auto diffusePath = GetMaterialPath(dir, mat, aiTextureType_DIFFUSE);
		auto specularPath = GetMaterialPath(dir, mat, aiTextureType_SPECULAR);
		auto emissivePath = GetMaterialPath(dir, mat, aiTextureType_EMISSIVE);

		glm::vec4 diffuse, specular, emissive;
		aiColor4D color;
		if(mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
			diffuse = glm::vec4(color.r, color.g, color.b, color.a);
		if(mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
			specular = glm::vec4(color.r, color.g, color.b, color.a);
		if(mat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS)
			emissive = glm::vec4(color.r, color.g, color.b, color.a);

		materialPaths.Emplace(diffusePath, specularPath, emissivePath,
			diffuse, specular, emissive);
	}
}

bool StringContains(const std::string& str, const std::string& subStr) {
	return str.find(subStr) != std::string::npos;
}

struct ShaderFile {
	const std::string Path;
	const Graphics::ShaderFileType Type;
};

VolcanicEditor::ShaderFile TryGetShader(const std::string& path) {
	std::size_t dot = path.find_first_of('.');
	if(dot == std::string::npos)
		VOLCANICORE_ASSERT_ARGS(false,
			"%s is an incorrectly formatted file name. Accepted formats: \
			example.glsl.vert, example.vert.glsl, example.vert", path.c_str());

	std::string str = path.substr(dot);
	if(StringContains(str, "vert") || StringContains(str, "vs"))
		return ShaderFile{ path, ShaderFileType::Vertex };
	if(StringContains(str, "frag") || StringContains(str, "fs"))
		return ShaderFile{ path, ShaderFileType::Fragment };
	if(StringContains(str, "geom") || StringContains(str, "gs"))
		return ShaderFile{ path, ShaderFileType::Geometry };
	if(StringContains(str, "comp") || StringContains(str, "compute"))
		return ShaderFile{ path, ShaderFileType::Compute };

	VOLCANICORE_ASSERT_ARGS(false, "File %s is of unknown shader type",
							path.c_str());
	return ShaderFile{ "", ShaderFileType::Unknown };
}

List<ShaderFile> GetShaders(const List<std::string>& paths) {
	List<ShaderFile> shaders(paths.Count());
	for(const auto& path : paths)
		shaders.Add(TryGetShader(path));

	return shaders;
}

List<ShaderFile> GetShaders(const std::string& shaderFolder,
							const std::string& name)
{
	namespace fs = std::filesystem;

	List<std::string> paths;
	for(auto path : FileUtils::GetFiles(shaderFolder,
					{ ".vert", ".frag", ".geom", ".comp" }))
	{
		if(fs::path(path).stem().stem().string() == name)
			paths.Add(path);
	}
	
	return GetShaders(paths);
}

Ref<Shader> AssetImporter::GetShader(const List<std::string>& paths) {
	List<Graphics::ShaderFile> list(paths.Count());
	for(auto path : paths) {
		auto file = TryGetShader(path);
		auto str = FileUtils::ReadFile(file.Path);
		// Buffer<u32> code = AssetImporter::GetShaderData(path);

		list.Emplace(file.Type, str);
	}

	auto shader = RendererAPI::Get()->CreateShader({});
	shader->SetShaderData(std::move(list));
	return shader;
}

Buffer<u32> AssetImporter::GetShaderData(const std::string& path) {
	glslang::InitializeProcess();

	ShaderFile file = TryGetShader(path);
	EShLanguage stage;
	if(file.Type == ShaderFileType::Vertex)
		stage = EShLangVertex;
	else if(file.Type == ShaderFileType::Fragment)
		stage = EShLangFragment;
	else if(file.Type == ShaderFileType::Compute)
		stage = EShLangCompute;
	else if(file.Type == ShaderFileType::Geometry)
		stage = EShLangGeometry;

	glslang::TShader shader(stage);
	std::string str = FileUtils::ReadFile(path);
	const char* sources[1] = { str.c_str() };
	shader.setStrings(sources, 1);

	shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientOpenGL, 100);
	shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
	shader.setEntryPoint("main");

	const TBuiltInResource* resources = GetDefaultResources();
	bool forceDefaults = false;
	bool forwardCompatible = true; // Warn deprecated features
	i32 defaultVersion = 100;
	EProfile defaultProfile = ENoProfile;
	EShMessages messageFlags = (EShMessages)(EShMsgDefault | EShMsgSpvRules);
	bool parsed =
		shader.parse(resources, defaultVersion, defaultProfile,
					 forceDefaults, forwardCompatible, messageFlags);
	if(!parsed) {
		Log::Error("Failed to parse '%s': %s",
			path.c_str(), shader.getInfoLog());
		glslang::FinalizeProcess();
		return { };
	}

	glslang::TProgram program;
	program.addShader(&shader);
	bool linked = program.link(messageFlags);
	if(!linked) {
		Log::Error("Failed to link '%s': %s",
			path.c_str(), program.getInfoLog());
		glslang::FinalizeProcess();
		return { };
	}

	glslang::TIntermediate& intermediateRef = *program.getIntermediate(stage);
	std::vector<u32> spirv;
	glslang::SpvOptions options{};
	options.validate = true;
	options.stripDebugInfo = true;
	options.generateDebugInfo = false;
	options.optimizeSize = true;
	options.disableOptimizer = false;
	// options.emitNonSemanticShaderDebugInfo = true;
	// options.emitNonSemanticShaderDebugSource = true;
	// options.compileOnly = true;

	spv::SpvBuildLogger logger;
	glslang::GlslangToSpv(intermediateRef, spirv, &logger, &options);
	if(!spirv.size()) {
		Log::Info("SPIRV Log: %s", logger.getAllMessages().c_str());
		glslang::FinalizeProcess();
		return { };
	}

	glslang::FinalizeProcess();

	Buffer<u32> data(spirv.size());
	data.Set(spirv.data(), spirv.size());

	return data;
}

Buffer<f32> AssetImporter::GetAudioData(const std::string& path) {
	SoLoud::Wav sound;
	VOLCANICORE_ASSERT(sound.load(path.c_str()) == 0);
	Buffer<f32> data(sound.mSampleCount);
	data.Set(sound.mData, sound.mSampleCount);
	return data;
}

Ref<Sound> AssetImporter::GetAudio(const std::string& path) {
	auto output = CreateRef<Sound>();
	VOLCANICORE_ASSERT(output->GetInternal().load(path.c_str()) == 0);
	return output;
}

asIScriptModule* AssetImporter::GetScriptData(const std::string& path,
	bool* error, std::string name)
{
	return ScriptManager::LoadScript(path, false, error, name);
}

Ref<ScriptModule> AssetImporter::GetScript(const std::string& path) {
	bool error;
	asIScriptModule* handle = ScriptManager::LoadScript(path, true, &error);
	if(error)
		return nullptr;

	return CreateRef<ScriptModule>(handle);
}

}
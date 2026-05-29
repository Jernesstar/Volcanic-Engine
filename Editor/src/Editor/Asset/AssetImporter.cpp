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

#define assert(x) (void)0
#include <SPIRV-Cross/spirv_glsl.hpp>

#include <soloud.h>
#include <soloud_wav.h>

#include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Core/Log.h>
#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/FileUtils.h>

#include <Engine/Graphics/Geometry.h>
#include <Engine/Graphics/Platform/RendererAPI.h>

#include "ScriptManager.h"

using namespace VolcaniCore;
using namespace VolcanicEngine::Audio;
using namespace VolcanicEngine::Graphics;
using namespace VolcanicEngine::Script;

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

static SubGeometry LoadSubGeometry(const aiMesh* mesh) {
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

static AssetImporter::ModelNodeData LoadModelNode(
	const aiNode* node, const aiScene* scene,
	Map<u32, u32>& meshIndexMap) // global aiMesh idx -> outSurfaces idx
{
	AssetImporter::ModelNodeData data;

	aiVector3D pos, scale;
	aiQuaternion rot;
	node->mTransformation.Decompose(scale, rot, pos);
	data.LocalTransform.Translation = { pos.x, pos.y, pos.z };
	data.LocalTransform.Scale       = { scale.x, scale.y, scale.z };
	glm::quat q(rot.w, rot.x, rot.y, rot.z);
	data.LocalTransform.Rotation = glm::degrees(glm::eulerAngles(q));

	if(node->mNumMeshes == 1) {
		u32 mi = node->mMeshes[0];
		data.GeometryIndex = (i32)meshIndexMap[mi];
		data.MaterialIndex = (i32)scene->mMeshes[mi]->mMaterialIndex;
	}
	else {
		// Multi-mesh node: collapse into synthetic children with identity transform
		for(u32 i = 0; i < node->mNumMeshes; i++) {
			u32 mi = node->mMeshes[i];
			AssetImporter::ModelNodeData& child = data.Children.Emplace();
			child.LocalTransform  = Transform{};
			child.GeometryIndex   = (i32)meshIndexMap[mi];
			child.MaterialIndex   = (i32)scene->mMeshes[mi]->mMaterialIndex;
		}
	}

	for(u32 i = 0; i < node->mNumChildren; i++)
		data.Children.Add(LoadModelNode(node->mChildren[i], scene, meshIndexMap));

	return data;
}

AssetImporter::ModelNodeData AssetImporter::GetModelData(
	const std::string&   path,
	List<SubGeometry>&   outSurfaces,
	List<MaterialPaths>& outMaterials)
{
	Assimp::Importer importer;
	u32 flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals
			  | aiProcess_FlipUVs    | aiProcess_JoinIdenticalVertices;
	const aiScene* scene = importer.ReadFile(path.c_str(), flags);
	VOLCANICORE_ASSERT_ARGS(scene, "Error importing model '{}': {}",
		path, importer.GetErrorString());

	outSurfaces.Allocate(scene->mNumMeshes);
	Map<u32, u32> meshIndexMap;
	for(u32 i = 0; i < scene->mNumMeshes; i++) {
		meshIndexMap[i] = outSurfaces.Count();
		outSurfaces.AddMove(LoadSubGeometry(scene->mMeshes[i]));
	}

	auto dir = (fs::path(path).parent_path() / "textures").string();
	outMaterials.Allocate(scene->mNumMaterials);
	for(u32 i = 0; i < scene->mNumMaterials; i++) {
		auto* mat = scene->mMaterials[i];
		auto diffuse = GetMaterialPath(dir, mat, aiTextureType_DIFFUSE);
		auto specular = GetMaterialPath(dir, mat, aiTextureType_SPECULAR);
		auto emissive = GetMaterialPath(dir, mat, aiTextureType_EMISSIVE);

		glm::vec4 diffColor{}, specColor{}, emisColor{};
		aiColor4D c;
		if(mat->Get(AI_MATKEY_COLOR_DIFFUSE,  c) == AI_SUCCESS) diffColor = { c.r, c.g, c.b, c.a };
		if(mat->Get(AI_MATKEY_COLOR_SPECULAR, c) == AI_SUCCESS) specColor = { c.r, c.g, c.b, c.a };
		if(mat->Get(AI_MATKEY_COLOR_EMISSIVE, c) == AI_SUCCESS) emisColor = { c.r, c.g, c.b, c.a };

		outMaterials.Emplace(diffuse, specular, emissive,
							  diffColor, specColor, emisColor);
	}

	return LoadModelNode(scene->mRootNode, scene, meshIndexMap);
}

Ref<Geometry> AssetImporter::GetGeometry(const std::string& path) {
	Assimp::Importer importer;
	u32 loadFlags = aiProcess_Triangulate
						| aiProcess_GenSmoothNormals
						| aiProcess_FlipUVs
						| aiProcess_JoinIdenticalVertices;
	const aiScene* scene = importer.ReadFile(path.c_str(), loadFlags);

	VOLCANICORE_ASSERT_ARGS(scene, "Error importing mesh from {}: {}",
							path.c_str(), importer.GetErrorString());

	Ref<Geometry> geometry = CreateRef<Geometry>(GeometryType::Model);
	geometry->Surfaces.Allocate(scene->mNumMeshes);

	for(u32 i = 0; i < scene->mNumMeshes; i++)
		geometry->Surfaces.AddMove(LoadSubGeometry(scene->mMeshes[i]));

	return geometry;
}

void AssetImporter::GetGeometryData(const std::string& path,
	List<SubGeometry>& surfaces, List<MaterialPaths>& materialPaths)
{
	Assimp::Importer importer;
	u32 loadFlags = aiProcess_Triangulate
						| aiProcess_GenSmoothNormals
						| aiProcess_FlipUVs
						| aiProcess_JoinIdenticalVertices;
	const aiScene* scene = importer.ReadFile(path.c_str(), loadFlags);

	VOLCANICORE_ASSERT_ARGS(scene, "Error importing mesh from {}: {}",
							path.c_str(), importer.GetErrorString());

	surfaces.Allocate(scene->mNumMeshes);
	materialPaths.Allocate(scene->mNumMaterials);

	for(u32 i = 0; i < scene->mNumMeshes; i++)
		surfaces.AddMove(LoadSubGeometry(scene->mMeshes[i]));

	auto dir = (fs::path(path).parent_path() / "textures").string();
	for(u32 i = 0; i < scene->mNumMaterials; i++) {
		auto mat = scene->mMaterials[i];
		auto diffusePath = GetMaterialPath(dir, mat, aiTextureType_DIFFUSE);
		auto specularPath = GetMaterialPath(dir, mat, aiTextureType_SPECULAR);
		auto emissivePath = GetMaterialPath(dir, mat, aiTextureType_EMISSIVE);

		glm::vec4 diffuse = glm::vec4(0.0f);
		glm::vec4 specular = glm::vec4(0.0f);
		glm::vec4 emissive = glm::vec4(0.0f);
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
			"{} is an incorrectly formatted file name. Accepted formats: \
			example.glsl.vert, example.vert.glsl, example.vert", path);

	std::string str = path.substr(dot);
	if(StringContains(str, "vert") || StringContains(str, "vs"))
		return ShaderFile{ path, ShaderFileType::Vertex };
	if(StringContains(str, "frag") || StringContains(str, "fs"))
		return ShaderFile{ path, ShaderFileType::Fragment };
	if(StringContains(str, "geom") || StringContains(str, "gs"))
		return ShaderFile{ path, ShaderFileType::Geometry };
	if(StringContains(str, "comp") || StringContains(str, "compute"))
		return ShaderFile{ path, ShaderFileType::Compute };

	VOLCANICORE_ASSERT_ARGS(false, "File {} is of unknown shader type", path);
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
	Graphics::ShaderLayout layout;
	for(auto path : paths) {
		auto file = TryGetShader(path);
		auto data = GetShaderData(path);
		ReflectShader(data, layout);
		list.Emplace(file.Type, std::move(data));
	}

	auto shader = RendererAPI::Get()->CreateShader({});
	shader->SetShaderData(std::move(list), layout);
	return shader;
}

Graphics::ShaderFile AssetImporter::GetShaderFileData(const std::string& path) {
	return { TryGetShader(path).Type, GetShaderData(path) };
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
		Log::Error("Failed to parse '{}': {}", path, shader.getInfoLog());
		glslang::FinalizeProcess();
		return { };
	}

	glslang::TProgram program;
	program.addShader(&shader);
	bool linked = program.link(messageFlags);
	if(!linked) {
		Log::Error("Failed to link '{}': {}", path, program.getInfoLog());
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
		Log::Info("SPIRV Log: {}", logger.getAllMessages());
		glslang::FinalizeProcess();
		return { };
	}

	glslang::FinalizeProcess();

	Buffer<u32> data(spirv.size());
	data.Set(spirv.data(), spirv.size());

	return data;
}

void AssetImporter::ReflectShader(const VolcaniCore::Buffer<u32>& spirv,
								  Graphics::ShaderLayout& layout)
{
	spirv_cross::CompilerGLSL compiler(spirv.Get(), spirv.GetCount());
	spirv_cross::ShaderResources resources = compiler.get_shader_resources();

	auto getPropType = [&](const spirv_cross::SPIRType& type) {
		if(type.basetype == spirv_cross::SPIRType::Float) {
			if(type.columns == 1) {
				if(type.vecsize == 1) return Graphics::ShaderPropType::Float;
				if(type.vecsize == 2) return Graphics::ShaderPropType::Vec2;
				if(type.vecsize == 3) return Graphics::ShaderPropType::Vec3;
				if(type.vecsize == 4) return Graphics::ShaderPropType::Vec4;
			}
			if(type.columns == 4 && type.vecsize == 4) return Graphics::ShaderPropType::Mat4;
		}
		if(type.basetype == spirv_cross::SPIRType::Int) return Graphics::ShaderPropType::Int;
		return Graphics::ShaderPropType::Int; // Default
	};

	for(auto& resource : resources.sampled_images) {
		layout.Samplers.Emplace(resource.name, Graphics::ShaderPropType::Texture,
			compiler.get_decoration(resource.id, spv::DecorationBinding),
			compiler.get_decoration(resource.id, spv::DecorationDescriptorSet));
	}

	for(auto& resource : resources.uniform_buffers) {
		const auto& type = compiler.get_type(resource.base_type_id);
		for(u32 i = 0; i < type.member_types.size(); i++) {
			std::string name = compiler.get_member_name(resource.base_type_id, i);
			const auto& memberType = compiler.get_type(type.member_types[i]);
			layout.Uniforms.Emplace(name, getPropType(memberType),
				compiler.get_decoration(resource.id, spv::DecorationBinding),
				compiler.get_decoration(resource.id, spv::DecorationDescriptorSet));
		}
	}
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
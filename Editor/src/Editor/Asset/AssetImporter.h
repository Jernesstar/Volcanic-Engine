#pragma once

#include <Engine/Graphics/Geometry.h>
#include <Engine/Graphics/Platform/Texture.h>
#include <Engine/Graphics/Platform/Cubemap.h>
#include <Engine/Graphics/Platform/Shader.h>

#include <Engine/Audio/Sound.h>
#include <Engine/Script/ScriptModule.h>

using namespace VolcaniCore;
using namespace VolcanicEngine;

namespace VolcanicEditor {

struct MaterialPaths {
	std::string Diffuse;
	std::string Specular;
	std::string Emissive;
	glm::vec4 DiffuseColor;
	glm::vec4 SpecularColor;
	glm::vec4 EmissiveColor;

	std::string operator[](u32 i) { return *(&Diffuse + i); }
};

class AssetImporter {
public:
	struct ModelNodeData {
		Transform         LocalTransform;
		i32               GeometryIndex = -1;  // index into outSurfaces
		i32               MaterialIndex = -1;  // index into outMaterials
		List<ModelNodeData> Children;
	};

	static Graphics::ImageData GetImageData(const std::string& path,
		bool flip = true);

	static Ref<Graphics::Geometry> GetGeometry(const std::string& path);
	static void GetGeometryData(const std::string& path,
		List<Graphics::SubGeometry>& geometry,
		List<MaterialPaths>& materialPaths);

	static ModelNodeData GetModelData(
		const std::string& path,
		List<Graphics::SubGeometry>&  outSurfaces,
		List<MaterialPaths>& outMaterials);

	static Ref<Graphics::Texture> GetTexture(const std::string& path,
		bool flip = true);
	// static Ref<Graphics::Cubemap> GetCubemap(const std::string& path);

	static VolcaniCore::Buffer<u32> GetShaderData(const std::string& path);
	static void ReflectShader(const VolcaniCore::Buffer<u32>& spirv,
							  Graphics::ShaderLayout& layout);
	static Graphics::ShaderFile GetShaderFileData(const std::string& path);
	static Ref<Graphics::Shader> GetShader(const List<std::string>& path);

	static VolcaniCore::Buffer<f32> GetAudioData(const std::string& path);
	static Ref<Audio::Sound> GetAudio(const std::string& path);

	static asIScriptModule* GetScriptData(const std::string& path,
		bool* error = nullptr, std::string name = "");
	static Ref<Script::ScriptModule> GetScript(const std::string& path);
};

}
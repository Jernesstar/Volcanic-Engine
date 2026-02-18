#pragma once

#include <Engine/Graphics/Mesh.h>
#include <Engine/Graphics/Texture.h>
#include <Engine/Graphics/Cubemap.h>
#include <Engine/Graphics/Shader.h>

#include <Engine/Audio/Sound.h>
#include <Engine/Script/ScriptModule.h>

using namespace VolcaniCore;

using namespace VolcanicEngine::Audio;

namespace VolcanicEditor {
struct MaterialPaths {
	std::string Diffuse;
	std::string Specular;
	std::string Emissive;
	glm::vec4 DiffuseColor;
	glm::vec4 SpecularColor;
	glm::vec4 EmissiveColor;

	std::string operator[](uint32_t i) { return *(&Diffuse + i); }
};

struct ShaderFile {
	const std::string Path;
	const Graphics::ShaderType Type;
};

class AssetImporter {
public:
	static Graphics::ImageData GetImageData(const std::string& path,
		bool flip = true);

	static Ref<Graphics::Mesh> GetMesh(const std::string& path);
	static void GetMeshData(const std::string& path,
		List<Graphics::SubMesh>& mesh,
		List<MaterialPaths>& materialPaths);

	static Ref<Graphics::Texture> GetTexture(const std::string& path,
		bool flip = true);
	static Ref<Graphics::Cubemap> GetCubemap(const std::string& path);

	static VolcaniCore::Buffer<uint32_t> GetShaderData(const std::string& path);
	static Ref<Graphics::ShaderPipeline> GetShader(
		const List<std::string>& path);

	static VolcaniCore::Buffer<float> GetAudioData(const std::string& path);
	static Ref<Sound> GetAudio(const std::string& path);

	static asIScriptModule* GetScriptData(const std::string& path,
		bool* error = nullptr, std::string name = "");
	static Ref<Script::ScriptModule> GetScript(const std::string& path);
};

}
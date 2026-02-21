#pragma once

#include <Engine/Asset/AssetManager.h>
#include <Engine/Script/ScriptObject.h>
#include <Engine/Scene/Scene.h>

#include "YAMLSerializer.h"

namespace VolcanicEditor {

extern void SaveScript(YAMLSerializer& serializer,
	Ref<Script::ScriptObject> obj);
extern Ref<Script::ScriptObject> LoadScript(Entity entity, Asset asset,
	YAML::Node& scriptComponentNode);

class SceneLoader {
public:
	static void EditorLoad(Scene& scene, const std::string& path);
	static void EditorSave(const Scene& scene, const std::string& path);
	static void RuntimeSave(const Scene& scene,
							const std::string& projectPath,
							const std::string& exportPath);
};

}
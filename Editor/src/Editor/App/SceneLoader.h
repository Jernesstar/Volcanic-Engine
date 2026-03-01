#pragma once

#include <Engine/Asset/AssetManager.h>
#include <Engine/Script/ScriptObject.h>
#include <Engine/Scene/Scene.h>

#include "Utils/YAMLSerializer.h"

using namespace VolcanicEngine;
using namespace VolcanicEngine::ECS;
// using namespace VolcanicEngine::Physics;
using namespace VolcanicEngine::Graphics;
using namespace VolcanicEngine::Script;

namespace VolcanicEditor {

extern void SaveScript(YAMLSerializer& serializer, Ref<ScriptObject> obj);
extern Ref<ScriptObject> LoadScript(Entity entity, Asset asset, YAML::Node& node);

class SceneLoader {
public:
	static void EditorLoad(Scene& scene, const std::string& path);
	static void EditorSave(const Scene& scene, const std::string& path);
	static void RuntimeSave(const Scene& scene,
							const std::string& projectPath,
							const std::string& exportPath);
};

}